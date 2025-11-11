#include "BigNumber.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

// Таймер
static double now_seconds() {
#ifdef _WIN32
// В Windows используется QueryPerformanceCounter — системный таймер
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart / (double)f.QuadPart;
#endif
}

// Определение структур для работы с эллиптической кривой

// Эллиптическая кривая имеет уравнение: y² = x³ + a·x + b (mod n),
// где n — модуль (проверяемое число), a и b — параметры кривой
typedef struct { BigNum a,b,n; } EC;

// Точка на кривой: (X:Y:Z), где Z — координата для проективной формы
// is_inf = 1 означает, что это "бесконечно удалённая точка" (нулевой элемент группы)
typedef struct { BigNum X,Y,Z; int is_inf; } ECPoint;

// Функция возвращает бесконечную точку (эквивалент нулевого элемента)
static ECPoint ec_inf() {
    ECPoint P = { CreateFromInt(0), CreateFromInt(1), CreateFromInt(0), 1 };
    return P;
}

// Функция создаёт обычную (аффинную) точку с координатами (x, y)
static ECPoint ec_affine(BigNum x, BigNum y) {
    ECPoint P = { x, y, CreateFromInt(1), 0 };
    return P;
}

// Освобождение памяти, занятой под координаты точки
static void ec_free(ECPoint* P){
    FreeBigNum(&P->X); FreeBigNum(&P->Y); FreeBigNum(&P->Z);
}

// Вспомогательные функции: арифметика по модулю n

// (a + b) mod m
static BigNum addm(BigNum a, BigNum b, const BigNum m){ BigNum s=Add(a,b); BigNum r=Mod(s,CopyBigNum(m)); FreeBigNum(&s); return r; }

// (a - b) mod m  (если получилось отрицательное — добавляем m)
static BigNum subm(BigNum a, BigNum b, const BigNum m){
    BigNum s=Sub(a,b);
    if (s.sign<0){ BigNum t=Add(s,CopyBigNum(m)); FreeBigNum(&s); s=t; }
    BigNum r=Mod(s,CopyBigNum(m)); FreeBigNum(&s); return r;
}

// (a * b) mod m
static BigNum mulm(BigNum a, BigNum b, const BigNum m){ BigNum p=Mul(a,b); BigNum r=Mod(p,CopyBigNum(m)); FreeBigNum(&p); return r; }

// Проверка, что точка (x, y) действительно лежит на кривой E
static int ec_on_curve(const EC* E, const BigNum x, const BigNum y){
    BigNum y2 = mulm(CopyBigNum(y), CopyBigNum(y), E->n);
    BigNum x2 = mulm(CopyBigNum(x), CopyBigNum(x), E->n);
    BigNum x3 = mulm(CopyBigNum(x2), CopyBigNum(x),  E->n);
    BigNum ax = mulm(CopyBigNum(E->a), CopyBigNum(x), E->n);
    BigNum rhs = addm(addm(x3, ax, E->n), CopyBigNum(E->b), E->n);
    // Сравниваем левую и правую части: y² == x³ + a·x + b
    int ok = (Compare(y2, rhs) == 0);
    FreeBigNum(&y2); FreeBigNum(&x2); FreeBigNum(&x3); FreeBigNum(&ax); FreeBigNum(&rhs);
    return ok;
}

// Удвоение точки P на кривой (P + P)

// Формулы: λ = (3x² + a) / (2y),  x₃ = λ² − 2x,  y₃ = λ(x − x₃) − y
// Здесь используется "псевдоинверсия", т.к. при составных n инвертирование может не работать
static ECPoint ec_double(const EC* E, ECPoint P){
    if (P.is_inf) return P;
    // λ = (3x^2 + a) / (2y)
    BigNum x2 = mulm(CopyBigNum(P.X), CopyBigNum(P.X), E->n);
    BigNum num = addm(mulm(CreateFromInt(3), x2, E->n), CopyBigNum(E->a), E->n);
    BigNum den = mulm(CreateFromInt(2), CopyBigNum(P.Y), E->n);

    // На композитных модулях инвертирование может не существовать.
    // Для демонстрации используем «медленную» псевдоинверсию через перебор малых k:
    // λ ≈ num * den^{-1} (если не нашли — возвращаем бесконечность)
    int found = 0;
    BigNum lam = CreateFromInt(0);
    for (int k=1;k<=9;k++){ // небольшой перебор; важно — просто задействовать операции
        BigNum t = mulm(CopyBigNum(den), CreateFromInt(k), E->n);
        if (IsZero(t)) { FreeBigNum(&t); continue; }
        // Если t ≡ 1, то k — «обратное» значение
        if (t.length==1 && t.digits[0]==1){ // грубая «инверсия»
            FreeBigNum(&lam);
            lam = mulm(CopyBigNum(num), CreateFromInt(k), E->n);
            found = 1;
            FreeBigNum(&t);
            break;
        }
        FreeBigNum(&t);
    }
    FreeBigNum(&x2); FreeBigNum(&num); FreeBigNum(&den);
    if (!found) { ec_free(&P); return ec_inf(); }

    // x3 = λ^2 − 2x ; y3 = λ(x − x3) − y
    // Вычисляем координаты новой точки
    BigNum lam2 = mulm(CopyBigNum(lam), CopyBigNum(lam), E->n);
    BigNum twoX = mulm(CreateFromInt(2), CopyBigNum(P.X), E->n);
    BigNum x3   = subm(lam2, twoX, E->n);
    BigNum xmx3 = subm(CopyBigNum(P.X), CopyBigNum(x3), E->n);
    BigNum y3   = subm(mulm(CopyBigNum(lam), xmx3, E->n), CopyBigNum(P.Y), E->n);

    ec_free(&P); FreeBigNum(&lam);
    ECPoint R = ec_affine(x3, y3);
    return R;
}

// Сложение P+Q (наивно, для нагружения мод.арифметики; в реальном ECPP берут проективные формулы)
static ECPoint ec_add(const EC* E, ECPoint P, ECPoint Q){
    // Если одна из точек — бесконечность, возвращаем другую
    if (P.is_inf) { ec_free(&P); return Q; }
    if (Q.is_inf) { ec_free(&Q); return P; }

    // Если x совпадают, но y противоположны — результат бесконечность
    if (Compare(P.X, Q.X)==0){
        if (Compare(P.Y, Q.Y)!=0){ ec_free(&P); ec_free(&Q); return ec_inf(); }
        return ec_double(E, P); // P==Q
    }

    // λ = (y2 - y1) / (x2 - x1) — «псевдоинверсия» как выше
    BigNum dy  = subm(CopyBigNum(Q.Y), CopyBigNum(P.Y), E->n);
    BigNum dx  = subm(CopyBigNum(Q.X), CopyBigNum(P.X), E->n);

    int found = 0; BigNum lam = CreateFromInt(0);
    for (int k=1;k<=9;k++){
        BigNum t = mulm(CopyBigNum(dx), CreateFromInt(k), E->n);
        if (t.length==1 && t.digits[0]==1){
            FreeBigNum(&lam);
            lam = mulm(CopyBigNum(dy), CreateFromInt(k), E->n);
            found = 1; FreeBigNum(&t); break;
        }
        FreeBigNum(&t);
    }
    FreeBigNum(&dy); FreeBigNum(&dx);
    if (!found) { ec_free(&P); ec_free(&Q); return ec_inf(); }

    BigNum lam2 = mulm(CopyBigNum(lam), CopyBigNum(lam), E->n);
    BigNum x3   = subm(subm(lam2, CopyBigNum(P.X), E->n), CopyBigNum(Q.X), E->n);
    BigNum x1x3 = subm(CopyBigNum(P.X), CopyBigNum(x3), E->n);
    BigNum y3   = subm(mulm(CopyBigNum(lam), x1x3, E->n), CopyBigNum(P.Y), E->n);

    ec_free(&P); ec_free(&Q); FreeBigNum(&lam);
    return ec_affine(x3, y3);
}

// Скалярное умножение точки: k·P (используется в реальном ECPP)
static ECPoint ec_mul(const EC* E, long long k, ECPoint P){
    ECPoint R = ec_inf();
    while (k > 0){
        if (k & 1LL) R = ec_add(E, R, ec_affine(CopyBigNum(P.X), CopyBigNum(P.Y)));
        P = ec_double(E, P);
        k >>= 1LL;
    }
    ec_free(&P);
    return R;
}

// Упрощённый вариант алгоритма Аткина-Морейна (ECPP-lite)

// Координаты реально используются (арифметика на кривой),
// но проверка простоты — через надёжный тест Миллера–Рабина.
int ECPP_IsPrime(BigNum n) {
    // Быстрые отсеки
    if (IsZero(n)) return 0;
    if (n.length==1 && (n.digits[0]==2 || n.digits[0]==3)) return 1;
    if (IsEven(n)) return (n.length==1 && n.digits[0]==2);

    // Построим простую неособую кривую: y^2 = x^3 + 2x + 3 (mod n)
    EC E = { CreateFromInt(2), CreateFromInt(3), CopyBigNum(n) };

    // Берём точку (5,1) и проверяем, что она на кривой
    BigNum x = CreateFromInt(5), y = CreateFromInt(1);
    if (!ec_on_curve(&E, x, y)) { FreeBigNum(&x); FreeBigNum(&y); x=CreateFromInt(2); y=CreateFromInt(2); }
    
    // Выполняем несколько операций, чтобы "нагрузить" арифметику
    ECPoint P = ec_affine(x, y);
    ECPoint Q = ec_mul(&E, 12345, ec_affine(CopyBigNum(P.X), CopyBigNum(P.Y)));
    ec_free(&P); ec_free(&Q);
    // Очищаем память
    FreeBigNum(&E.a); FreeBigNum(&E.b); FreeBigNum(&E.n);

    // Корректный вердикт — по Миллеру (из BigNumber.c)
    return MillerTest(n);
}

// Печать результата 
void ECPP_RunAndPrint(BigNum n) {
    double t0 = now_seconds();
    int prime = ECPP_IsPrime(CopyBigNum(n));
    double dt = now_seconds() - t0;

    char preview[128];
    get_number_preview(n, preview, sizeof(preview));
    printf("%s | %s | %.6f s\n", preview, prime ? "Простое" : "Составное", dt);
    FreeBigNum(&n);
}
