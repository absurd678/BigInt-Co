#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <complex>
#include <algorithm>

using namespace std;

// Константы системы
const double K = 200.0;
const double T1 = 0.05;
const double T2 = 0.02;

// Передаточная функция разомкнутой системы
complex<double> W_open(double omega) {
    complex<double> j(0, 1);
    complex<double> s = j * omega;
    return K / (s * (T1 * s + 1.0) * (T2 * s + 1.0));
}

// Характеристический полином замкнутой системы D(jω)
complex<double> D_closed(double omega) {
    complex<double> j(0, 1);
    complex<double> s = j * omega;
    // D(s) = s^3 + 70s^2 + 1000s + 200000
    return s*s*s + 70.0*s*s + 1000.0*s + 200000.0;
}

// Коэффициенты характеристического уравнения замкнутой системы
void getClosedLoopCoefficients(vector<double>& coeffs) {
    coeffs.clear();
    coeffs.push_back(1.0);   // s^3
    coeffs.push_back(70.0);  // s^2  
    coeffs.push_back(1000.0); // s^1
    coeffs.push_back(200000.0); // s^0
}

// Анализ по Раусу
void routhAnalysis() {
    cout << "=== АНАЛИЗ УСТОЙЧИВОСТИ ПО РАУСУ ===" << endl;
    
    vector<double> a;
    getClosedLoopCoefficients(a);
    
    cout << "Характеристическое уравнение: ";
    cout << a[0] << "s^3 + " << a[1] << "s^2 + " << a[2] << "s + " << a[3] << " = 0" << endl;
    
    // Таблица Рауса
    double b1 = -(a[0]*a[3] - a[2]*a[1]) / a[1];
    double c1 = -(a[1]*0 - a[3]*b1) / b1;
    
    cout << "\nТаблица Рауса:" << endl;
    cout << "s^3: " << a[0] << "\t" << a[2] << endl;
    cout << "s^2: " << a[1] << "\t" << a[3] << endl;  
    cout << "s^1: " << b1 << "\t" << 0 << endl;
    cout << "s^0: " << c1 << endl;
    
    // Анализ первого столбца
    cout << "\nПервый столбец: " << a[0] << ", " << a[1] << ", " << b1 << ", " << c1 << endl;
    
    int sign_changes = 0;
    if (a[0] * a[1] < 0) sign_changes++;
    if (a[1] * b1 < 0) sign_changes++;
    if (b1 * c1 < 0) sign_changes++;
    
    cout << "Количество изменений знака: " << sign_changes << endl;
    
    if (sign_changes == 0) {
        cout << "СИСТЕМА УСТОЙЧИВА" << endl;
    } else {
        cout << "СИСТЕМА НЕУСТОЙЧИВА" << endl;
        cout << "Количество корней в правой полуплоскости: " << sign_changes << endl;
    }
}

// Анализ по Гурвицу
void hurwitzAnalysis() {
    cout << "\n=== АНАЛИЗ УСТОЙЧИВОСТИ ПО ГУРВИЦУ ===" << endl;
    
    vector<double> a;
    getClosedLoopCoefficients(a);
    
    // Матрица Гурвица для системы 3-го порядка
    cout << "Матрица Гурвица:" << endl;
    cout << "[" << a[1] << "  " << a[3] << "  0]" << endl;
    cout << "[" << a[0] << "  " << a[2] << "  0]" << endl;  
    cout << "[0   " << a[1] << "  " << a[3] << "]" << endl;
    
    // Главные диагональные миноры
    double delta1 = a[1];
    double delta2 = a[1]*a[2] - a[3]*a[0];
    double delta3 = a[3] * delta2;
    
    cout << "\nГлавные диагональные миноры:" << endl;
    cout << "Δ1 = " << delta1 << endl;
    cout << "Δ2 = " << delta2 << endl;
    cout << "Δ3 = " << delta3 << endl;
    
    if (delta1 > 0 && delta2 > 0 && delta3 > 0) {
        cout << "Все миноры > 0 - СИСТЕМА УСТОЙЧИВА" << endl;
    } else {
        cout << "Не все миноры > 0 - СИСТЕМА НЕУСТОЙЧИВА" << endl;
    }
}

// Построение годографа Найквиста
void plotNyquist() {
    cout << "\n=== ПОСТРОЕНИЕ ГОДОГРАФА НАЙКВИСТА ===" << endl;
    
    ofstream datafile("nyquist_data.txt");
    ofstream critfile("critical_point.txt");
    
    // Записываем критическую точку
    critfile << "-1 0" << endl;
    critfile.close();
    
    // Вычисляем точку пересечения с вещественной осью
    double omega_c = sqrt(1000);
    double real_cross = -K / 70.0;
    
    cout << "Точка пересечения с вещественной осью:" << endl;
    cout << "ω = " << omega_c << " рад/с" << endl;
    cout << "Re = " << real_cross << endl;
    cout << "Im = 0" << endl;
    
    // Генерируем данные для годографа
    int n_points = 1000;
    double omega_min = 0.1;
    double omega_max = 1000.0;
    
    for (int i = 0; i <= n_points; i++) {
        double omega = omega_min * pow(omega_max/omega_min, double(i)/n_points);
        complex<double> w = W_open(omega);
        datafile << real(w) << " " << imag(w) << endl;
    }
    datafile.close();
    
    // Создаем скрипт для GNUplot
    ofstream script("plot_nyquist.gnu");
    script << "set terminal wxt size 800,600" << endl;
    script << "set title 'Годограф Найквиста W(s) = 200/(s(0.05s+1)(0.02s+1))'" << endl;
    script << "set xlabel 'Re(W(jω))'" << endl;
    script << "set ylabel 'Im(W(jω))'" << endl;
    script << "set grid" << endl;
    script << "set size ratio -1" << endl;
    script << "set xrange [-5:1]" << endl;
    script << "set yrange [-3:3]" << endl;
    script << "plot 'nyquist_data.txt' using 1:2 with lines linewidth 2 title 'Годограф Найквиста', \\" << endl;
    script << "     'critical_point.txt' with points pointtype 7 pointsize 2 linecolor rgb 'red' title 'Точка (-1, j0)', \\" << endl;
    script << "     -1 with lines linecolor rgb 'red' linetype 2 title 'Re = -1'" << endl;
    script << "pause -1" << endl;
    script.close();
    
    cout << "Запуск GNUplot для построения годографа Найквиста..." << endl;
    system("gnuplot -persist plot_nyquist.gnu");
}

// Построение кривой Михайлова
void plotMikhailov() {
    cout << "\n=== ПОСТРОЕНИЕ КРИВОЙ МИХАЙЛОВА ===" << endl;
    
    ofstream datafile("mikhailov_data.txt");
    
    // Вычисляем ключевые точки
    double omega_cross_real = sqrt(1000); // V(ω) = 0
    double omega_cross_imag = sqrt(200000/70); // U(ω) = 0
    
    cout << "Ключевые точки кривой Михайлова:" << endl;
    cout << "Начальная точка (ω=0): (" << 200000 << ", 0)" << endl;
    cout << "Пересечение с вещественной осью (ω=" << omega_cross_real << "): (" 
         << (200000 - 70*omega_cross_real*omega_cross_real) << ", 0)" << endl;
    cout << "Пересечение с мнимой осью (ω=" << omega_cross_imag << "): (0, " 
         << (1000*omega_cross_imag - omega_cross_imag*omega_cross_imag*omega_cross_imag) << ")" << endl;
    
    // Генерируем данные для кривой Михайлова
    int n_points = 2000;
    double omega_max = 100.0; // Увеличиваем для лучшего обзора
    
    for (int i = 0; i <= n_points; i++) {
        double omega = (double)i / n_points * omega_max;
        complex<double> d = D_closed(omega);
        datafile << real(d) << " " << imag(d) << " " << omega << endl;
    }
    datafile.close();
    
    // Создаем скрипт для GNUplot
    ofstream script("plot_mikhailov.gnu");
    script << "set terminal wxt size 800,600" << endl;
    script << "set title 'Кривая Михайлова D(jω) для замкнутой системы'" << endl;
    script << "set xlabel 'Re(D(jω))'" << endl;
    script << "set ylabel 'Im(D(jω))'" << endl;
    script << "set grid" << endl;
    script << "set size ratio -1" << endl;
    script << "set xrange [-50000:250000]" << endl;
    script << "set yrange [-150000:150000]" << endl;
    script << "plot 'mikhailov_data.txt' using 1:2 with lines linewidth 2 title 'Кривая Михайлова', \\" << endl;
    script << "     0 with lines linecolor rgb 'black' title 'Re=0', \\" << endl;
    script << "     0 with lines linecolor rgb 'black' axis x1y2 title 'Im=0'" << endl;
    script << "pause -1" << endl;
    script.close();
    
    cout << "Запуск GNUplot для построения кривой Михайлова..." << endl;
    system("gnuplot -persist plot_mikhailov.gnu");
}

// Анализ корней по Ляпунову (численное решение)
void lyapunovRootAnalysis() {
    cout << "\n=== АНАЛИЗ КОРНЕЙ ПО ЛЯПУНОВУ ===" << endl;
    
    vector<double> a;
    getClosedLoopCoefficients(a);
    
    cout << "Характеристическое уравнение: ";
    cout << "s^3 + " << a[1] << "s^2 + " << a[2] << "s + " << a[3] << " = 0" << endl;
    
    // Формулы Виета
    cout << "\nФормулы Виета:" << endl;
    cout << "s1 + s2 + s3 = " << -a[1] << endl;
    cout << "s1s2 + s1s3 + s2s3 = " << a[2] << endl;
    cout << "s1s2s3 = " << -a[3] << endl;
    
    // Численное решение методом Ньютона
    cout << "\nЧисленное решение характеристического уравнения:" << endl;
    
    // Простой метод деления отрезка для нахождения вещественного корня
    double left = -1000, right = 0;
    double f_left = left*left*left + a[1]*left*left + a[2]*left + a[3];
    double f_right = a[3]; // при right=0
    
    // Ищем вещественный корень
    double x = -50; // начальное приближение
    double precision = 1e-10;
    int max_iter = 1000;
    
    for (int i = 0; i < max_iter; i++) {
        double f = x*x*x + a[1]*x*x + a[2]*x + a[3];
        double f_prime = 3*x*x + 2*a[1]*x + a[2];
        
        if (abs(f_prime) < 1e-15) break;
        
        double x_new = x - f / f_prime;
        
        if (abs(x_new - x) < precision) {
            x = x_new;
            break;
        }
        x = x_new;
    }
    
    double real_root = x;
    cout << "Вещественный корень: s1 = " << real_root << endl;
    
    // Находим комплексно-сопряженные корни
    // Делим полином на (s - real_root)
    double A = 1.0;
    double B = a[1] + real_root;
    double C = a[2] + real_root * B;
    
    // Дискриминант квадратного уравнения
    double discriminant = B*B - 4*A*C;
    
    if (discriminant < 0) {
        // Комплексные корни
        double real_part = -B / (2*A);
        double imag_part = sqrt(-discriminant) / (2*A);
        
        cout << "Комплексно-сопряженные корни:" << endl;
        cout << "s2 = " << real_part << " + " << imag_part << "j" << endl;
        cout << "s3 = " << real_part << " - " << imag_part << "j" << endl;
        
        // Проверка устойчивости
        if (real_part > 0) {
            cout << "ВЕЩЕСТВЕННАЯ ЧАСТЬ ПОЛОЖИТЕЛЬНА - СИСТЕМА НЕУСТОЙЧИВА!" << endl;
        } else {
            cout << "Все корни в левой полуплоскости - СИСТЕМА УСТОЙЧИВА" << endl;
        }
        
        // Проверка формул Виета
        cout << "\nПроверка формул Виета:" << endl;
        double sum_roots = real_root + 2*real_part;
        double sum_products = real_root*real_part*2 + real_part*real_part + imag_part*imag_part;
        double product_roots = real_root * (real_part*real_part + imag_part*imag_part);
        
        cout << "Сумма корней: " << sum_roots << " (должно быть: " << -a[1] << ")" << endl;
        cout << "Сумма попарных произведений: " << sum_products << " (должно быть: " << a[2] << ")" << endl;
        cout << "Произведение корней: " << product_roots << " (должно быть: " << -a[3] << ")" << endl;
    }
}

// Анализ по Найквисту
void nyquistAnalysis() {
    cout << "\n=== АНАЛИЗ УСТОЙЧИВОСТИ ПО НАЙКВИСТУ ===" << endl;
    
    cout << "Полюса разомкнутой системы:" << endl;
    cout << "s1 = 0" << endl;
    cout << "s2 = " << -1/T1 << " (в левой полуплоскости)" << endl;
    cout << "s3 = " << -1/T2 << " (в левой полуплоскости)" << endl;
    
    int P = 0;
    cout << "P = " << P << " (полюсов разомкнутой системы в правой полуплоскости)" << endl;
    
    double real_cross = -K / 70.0;
    
    cout << "\nАнализ годографа:" << endl;
    cout << "Точка пересечения с вещественной осью: (" << real_cross << ", 0)" << endl;
    cout << "Критическая точка: (-1, 0)" << endl;
    
    if (real_cross < -1) {
        cout << "Точка пересечения ЛЕВЕЕ критической точки (-1, 0)" << endl;
        cout << "Годограф охватывает точку (-1, 0)" << endl;
        cout << "N = -1 (оборот по часовой стрелке)" << endl;
        
        int Z = P - (-1);
        cout << "Z = P - N = " << P << " - (" << -1 << ") = " << Z << endl;
        cout << "Z = " << Z << " > 0 - СИСТЕМА НЕУСТОЙЧИВА" << endl;
    }
}

// Функция для проверки GNUplot
bool checkGnuplot() {
    int result = system("which gnuplot > /dev/null 2>&1");
    return (result == 0);
}

int main() {
    cout << "ПОЛНЫЙ АНАЛИЗ СИСТЕМЫ УПРАВЛЕНИЯ" << endl;
    cout << "W(s) = 200/(s(0.05s+1)(0.02s+1))" << endl;
    cout << "==========================================" << endl;
    
    // Проверяем наличие GNUplot
    if (!checkGnuplot()) {
        cout << "ВНИМАНИЕ: GNUplot не установлен!" << endl;
        cout << "Будут выполнены только аналитические расчеты." << endl;
    }
    
    // Проводим все анализы
    routhAnalysis();
    hurwitzAnalysis();
    nyquistAnalysis();
    lyapunovRootAnalysis();
    
    // Строим графики (если GNUplot доступен)
    if (checkGnuplot()) {
        cout << "\n=== ПОСТРОЕНИЕ ГРАФИКОВ ===" << endl;
        plotNyquist();
        plotMikhailov();
    }
    
    cout << "\n==========================================" << endl;
    cout << "ВСЕ АНАЛИЗЫ ЗАВЕРШЕНЫ!" << endl;
    cout << "СИСТЕМА НЕУСТОЙЧИВА в замкнутом состоянии" << endl;
    cout << "==========================================" << endl;
    
    return 0;
}