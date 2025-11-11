#include "BigNumber.h"
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

// ------------------------------------------------------
// Базовые функции
// ------------------------------------------------------
BigNum CreateFromInt(long long value) {
    BigNum n;
    n.sign = (value < 0) ? -1 : 1;
    if (value < 0) value = -value;
    n.length = 0;
    n.digits = malloc(32 * sizeof(int));
    if (!n.digits) exit(1);
    if (value == 0) {
        n.length = 1;
        n.digits[0] = 0;
        return n;
    }
    while (value > 0) {
        n.digits[n.length++] = (int)(value % BASE);
        value /= BASE;
    }
    return n;
}

BigNum MakeBigNum(const char *str) {
    BigNum n;
    n.sign = 1;
    int L = (int)strlen(str);
    n.length = L;
    n.digits = malloc(L * sizeof(int));
    if (!n.digits) exit(1);
    for (int i = 0; i < L; i++) {
        char c = str[L - 1 - i];
        n.digits[i] = (c >= '0' && c <= '9') ? (c - '0') : 0;
    }
    while (n.length > 1 && n.digits[n.length - 1] == 0)
        n.length--;
    return n;
}

BigNum CopyBigNum(BigNum src) {
    BigNum copy;
    copy.length = src.length;
    copy.sign = src.sign;
    copy.digits = malloc(copy.length * sizeof(int));
    memcpy(copy.digits, src.digits, copy.length * sizeof(int));
    return copy;
}

void FreeBigNum(BigNum *n) {
    if (n->digits) free(n->digits);
    n->digits = NULL;
    n->length = 0;
    n->sign = 1;
}

bool IsZero(BigNum n) { return (n.length == 1 && n.digits[0] == 0); }
bool IsEven(BigNum n) { return (n.digits[0] % 2 == 0); }

int Compare(BigNum a, BigNum b) {
    if (a.length != b.length) return (a.length > b.length) ? 1 : -1;
    for (int i = a.length - 1; i >= 0; i--)
        if (a.digits[i] != b.digits[i])
            return (a.digits[i] > b.digits[i]) ? 1 : -1;
    return 0;
}

// ------------------------------------------------------
// Арифметика
// ------------------------------------------------------
BigNum Add(BigNum a, BigNum b) {
    int len = (a.length > b.length ? a.length : b.length) + 1;
    BigNum res = {.digits = calloc(len, sizeof(int)), .length = len};
    int carry = 0;
    for (int i = 0; i < len; i++) {
        int s = (i < a.length ? a.digits[i] : 0) + (i < b.length ? b.digits[i] : 0) + carry;
        res.digits[i] = s % BASE;
        carry = s / BASE;
    }
    while (res.length > 1 && res.digits[res.length - 1] == 0)
        res.length--;
    return res;
}

BigNum Sub(BigNum a, BigNum b) {
    BigNum res = {.digits = calloc(a.length, sizeof(int)), .length = a.length};
    int borrow = 0;
    for (int i = 0; i < a.length; i++) {
        int s = a.digits[i] - (i < b.length ? b.digits[i] : 0) - borrow;
        if (s < 0) { s += BASE; borrow = 1; } else borrow = 0;
        res.digits[i] = s;
    }
    while (res.length > 1 && res.digits[res.length - 1] == 0)
        res.length--;
    return res;
}

BigNum Mul(BigNum a, BigNum b) {
    BigNum res = {.digits = calloc(a.length + b.length, sizeof(int)),
                  .length = a.length + b.length};
    for (int i = 0; i < a.length; i++)
        for (int j = 0; j < b.length; j++)
            res.digits[i + j] += a.digits[i] * b.digits[j];
    for (int i = 0; i < res.length - 1; i++) {
        res.digits[i + 1] += res.digits[i] / BASE;
        res.digits[i] %= BASE;
    }
    while (res.length > 1 && res.digits[res.length - 1] == 0)
        res.length--;
    return res;
}

BigNum DivByTwo(BigNum n) {
    int carry = 0;
    for (int i = n.length - 1; i >= 0; i--) {
        int cur = n.digits[i] + carry * BASE;
        n.digits[i] = cur / 2;
        carry = cur % 2;
    }
    while (n.length > 1 && n.digits[n.length - 1] == 0)
        n.length--;
    return n;
}

// ------------------------------------------------------
// Вспомогательные мелкие операции для ModFast
// ------------------------------------------------------
static void trim(BigNum *x) {
    while (x->length > 1 && x->digits[x->length - 1] == 0)
        x->length--;
}

static BigNum MulSmall(BigNum a, int k) {
    BigNum r; r.length = a.length + 2;
    r.digits = calloc(r.length, sizeof(int));
    int carry = 0;
    for (int i = 0; i < a.length; ++i) {
        int t = a.digits[i] * k + carry;
        r.digits[i] = t % BASE;
        carry = t / BASE;
    }
    int i = a.length;
    while (carry) {
        r.digits[i++] = carry % BASE;
        carry /= BASE;
    }
    r.length = i;
    trim(&r);
    return r;
}

static BigNum AddSmall(BigNum a, int k) {
    BigNum r = CopyBigNum(a);
    int i = 0, carry = k;
    while (carry) {
        if (i >= r.length) {
            r.digits = realloc(r.digits, (r.length + 1) * sizeof(int));
            r.digits[r.length++] = 0;
        }
        int t = r.digits[i] + carry;
        r.digits[i] = t % BASE;
        carry = t / BASE;
        ++i;
    }
    return r;
}

// ------------------------------------------------------
// Быстрый модуль (длинное деление в базе 10)
// ------------------------------------------------------
BigNum Mod(BigNum a, BigNum m) {
    BigNum rem = CreateFromInt(0);
    for (int i = a.length - 1; i >= 0; --i) {
        BigNum t = MulSmall(rem, BASE); FreeBigNum(&rem); rem = t;
        t = AddSmall(rem, a.digits[i]); FreeBigNum(&rem); rem = t;
        for (int c = 0; c < 10 && Compare(rem, m) >= 0; ++c) {
            BigNum tmp = Sub(rem, m);
            FreeBigNum(&rem);
            rem = tmp;
        }
    }
    FreeBigNum(&a);
    trim(&rem);
    return rem;
}

// ------------------------------------------------------
// Утилита для краткого вывода
// ------------------------------------------------------
void get_number_preview(BigNum n, char *buf, int bufsize) {
    int show = (n.length < bufsize - 1) ? n.length : (bufsize - 1);
    for (int i = 0; i < show; i++)
        buf[i] = n.digits[n.length - 1 - i] + '0';
    buf[show] = '\0';
}

// ------------------------------------------------------
// Миллер–Рабин
// ------------------------------------------------------
static uint64_t mulmod_u64(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t res = 0;
    a %= m;
    while (b) {
        if (b & 1ULL) {
            res += a;
            if (res >= m) res -= m;
        }
        a <<= 1;
        if (a >= m) a -= m;
        b >>= 1ULL;
    }
    return res;
}

static uint64_t powmod_u64(uint64_t a, uint64_t e, uint64_t m) {
    uint64_t r = 1;
    a %= m;
    while (e) {
        if (e & 1ULL) r = mulmod_u64(r, a, m);
        a = mulmod_u64(a, a, m);
        e >>= 1ULL;
    }
    return r;
}

static int miller_u64(uint64_t n) {
    if (n < 2) return 0;
    static const uint64_t bases[] = {2,3,5,7,11,13,17};
    for (int i = 0; i < 7; i++) if (n == bases[i]) return 1;
    for (int i = 0; i < 7; i++) if (n % bases[i] == 0) return 0;

    uint64_t d = n - 1; int r = 0;
    while ((d & 1ULL) == 0) { d >>= 1ULL; r++; }

    for (int i = 0; i < 7; i++) {
        uint64_t a = bases[i];
        if (a >= n) continue;
        uint64_t x = powmod_u64(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0;
        for (int j = 1; j < r; j++) {
            x = mulmod_u64(x, x, n);
            if (x == n - 1) { ok = 1; break; }
        }
        if (!ok) return 0;
    }
    return 1;
}

// --- для больших чисел ---
static int big_mod_small(BigNum n, int p) {
    int rem = 0;
    for (int i = n.length - 1; i >= 0; --i)
        rem = (rem * BASE + n.digits[i]) % p;
    return rem;
}

static BigNum bn_powmod(BigNum a, BigNum e, BigNum n) {
    BigNum r = CreateFromInt(1);
    BigNum base = Mod(CopyBigNum(a), CopyBigNum(n));
    BigNum exp = CopyBigNum(e);
    while (!IsZero(exp)) {
        if (!IsEven(exp)) {
            BigNum t = Mul(r, base); FreeBigNum(&r);
            r = Mod(t, CopyBigNum(n));
        }
        BigNum t = Mul(base, base); FreeBigNum(&base);
        base = Mod(t, CopyBigNum(n));
        exp = DivByTwo(exp);
    }
    FreeBigNum(&base);
    FreeBigNum(&exp);
    return r;
}

static int is_probable_prime_bignum(BigNum n) {
    if (n.length == 1 && (n.digits[0] == 2 || n.digits[0] == 3)) return 1;
    if (IsEven(n)) return 0;

    static const int smallp[] = {3,5,7,11,13,17,19,23,29,31,37};
    for (int i = 0; i < (int)(sizeof(smallp)/sizeof(smallp[0])); ++i) {
        int p = smallp[i];
        if (big_mod_small(n, p) == 0) {
            BigNum P = CreateFromInt(p);
            int eq = (Compare(n, P) == 0);
            FreeBigNum(&P);
            return eq ? 1 : 0;
        }
    }

    BigNum one = CreateFromInt(1);
    BigNum n1 = Sub(CopyBigNum(n), one);
    BigNum d = CopyBigNum(n1);
    int r = 0;
    while (IsEven(d)) { d = DivByTwo(d); r++; }

    static const int bases[] = {2,3,5,7,11};
    for (int i = 0; i < (int)(sizeof(bases)/sizeof(bases[0])); ++i) {
        BigNum a = CreateFromInt(bases[i]);
        BigNum x = bn_powmod(CopyBigNum(a), CopyBigNum(d), CopyBigNum(n));
        if (Compare(x, one) == 0 || Compare(x, n1) == 0) {
            FreeBigNum(&a); FreeBigNum(&x); continue;
        }
        int witness = 1;
        for (int j = 1; j < r; j++) {
            BigNum xx = Mul(x, x); FreeBigNum(&x);
            x = Mod(xx, CopyBigNum(n));
            if (Compare(x, n1) == 0) { witness = 0; break; }
        }
        FreeBigNum(&a); FreeBigNum(&x);
        if (witness) { FreeBigNum(&d); FreeBigNum(&n1); FreeBigNum(&one); return 0; }
    }

    FreeBigNum(&d); FreeBigNum(&n1); FreeBigNum(&one);
    return 1;
}

int MillerTest(BigNum n) {
    if (n.length <= 19) {
        char buf[32];
        for (int i = 0; i < n.length; i++)
            buf[i] = (char)(n.digits[n.length - 1 - i] + '0');
        buf[n.length] = '\0';
        uint64_t num = strtoull(buf, NULL, 10);
        return miller_u64(num);
    }
    return is_probable_prime_bignum(n);
}
