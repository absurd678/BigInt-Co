/*
g++ -o frequency_response frequency_response.cpp -lm -std=c++11
./frequency_response
rm -f plot_graphs.gnu

for script in plot_*.gnu; do 
    echo "Запускаю: $script"
    gnuplot "$script"
done


*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <complex>

using namespace std;

// Ваша передаточная функция H(jw) = 200 / (0.001*((jw)^3) + 0.07*((jw)^2) + jw + 200)
complex<double> H(double w) {
    complex<double> j(0, 1); // мнимая единица
    complex<double> numerator = 200.0;
    
    // Знаменатель: 0.001*(jw)^3 + 0.07*(jw)^2 + jw + 200
    complex<double> term1 = 0.001 * pow(j * w, 3); // 0.001*(jw)^3
    complex<double> term2 = 0.07 * pow(j * w, 2);  // 0.07*(jw)^2  
    complex<double> term3 = j * w;                 // jw
    complex<double> term4 = 200.0;                 // 200
    
    complex<double> denominator = term1 + term2 + term3 + term4;
    
    return numerator / denominator;
}

// Функции для асимптотической ЛАЧХ
double asymptotic_L(double w) {
    double L = 0;
    
    // K = 200 -> 20*log10(200) = 46.02 дБ
    L += 20 * log10(200.0);
    
    // Апериодическое звено: 1/(0.05s + 1)
    if (w > 20) { // ω_cp = 1/0.05 = 20
        L -= 20 * log10(w/20.0);
    }
    
    // Форсирующее звено: (0.02s + 1)
    if (w > 50) { // ω_cp = 1/0.02 = 50
        L += 20 * log10(w/50.0);
    }
    
    // Форсирующее звено: (0.001s + 1)
    if (w > 1000) { // ω_cp = 1/0.001 = 1000
        L += 20 * log10(w/1000.0);
    }
    
    // Апериодическое звено: 1/(0.0001s + 1)
    if (w > 10000) { // ω_cp = 1/0.0001 = 10000
        L -= 20 * log10(w/10000.0);
    }
    
    return L;
}

// Функция для асимптотической ФЧХ (в градусах)
double asymptotic_phase(double w) {
    double phase = 0;
    
    // Усилительное звено: фаза = 0
    
    // Апериодическое звено: 1/(0.05s + 1)
    if (w < 2) { // 0.1 * ω_cp
        phase += 0;
    } else if (w > 200) { // 10 * ω_cp
        phase += -90;
    } else {
        phase += -45 * log10(w/20.0) / log10(10); // линейная аппроксимация
    }
    
    // Форсирующее звено: (0.02s + 1)
    if (w < 5) { // 0.1 * ω_cp
        phase += 0;
    } else if (w > 500) { // 10 * ω_cp
        phase += 90;
    } else {
        phase += 45 * log10(w/50.0) / log10(10); // линейная аппроксимация
    }
    
    // Форсирующее звено: (0.001s + 1)
    if (w < 100) { // 0.1 * ω_cp
        phase += 0;
    } else if (w > 10000) { // 10 * ω_cp
        phase += 90;
    } else {
        phase += 45 * log10(w/1000.0) / log10(10); // линейная аппроксимация
    }
    
    // Апериодическое звено: 1/(0.0001s + 1)
    if (w < 1000) { // 0.1 * ω_cp
        phase += 0;
    } else if (w > 100000) { // 10 * ω_cp
        phase += -90;
    } else {
        phase += -45 * log10(w/10000.0) / log10(10); // линейная аппроксимация
    }
    
    return phase;
}

int main() {
    vector<vector<double>> results;
    
    // Параметры расчета
    double w_start = 0.1;
    double w_end = 100000.0; // увеличим диапазон для асимптотики
    int points = 1000;
    
    cout << "Расчет частотных характеристик системы" << endl;
    cout << "H(jw) = 200 / (0.001*(jw)^3 + 0.07*(jw)^2 + jw + 200)" << endl;
    cout << "Создание графиков с ЛФЧХ..." << endl;
    cout << "======================================" << endl;
    
    // Основной цикл расчета
    for (int i = 0; i <= points; i++) {
        // Логарифмическая шкала частот
        double log_w_start = log10(w_start);
        double log_w_end = log10(w_end);
        double w = pow(10, log_w_start + (log_w_end - log_w_start) * i / points);
        
        // Вычисляем H(jw)
        complex<double> H_jw = H(w);
        
        // Извлекаем действительную и мнимую части
        double Re = real(H_jw);
        double Im = imag(H_jw);
        
        // Вычисляем модуль и фазу
        double A = abs(H_jw);
        double L = 20 * log10(A);
        double phase_rad = arg(H_jw);
        double phase_deg = phase_rad * 180 / M_PI;
        
        // Асимптотическая ЛАЧХ и ФЧХ
        double L_asympt = asymptotic_L(w);
        double phase_asympt = asymptotic_phase(w);
        
        vector<double> point = {w, Re, Im, A, L, phase_deg, L_asympt, phase_asympt};
        results.push_back(point);
    }
    
    // Сохранение данных в файлы
    ofstream file_nyquist("nyquist_data.txt");
    ofstream file_amplitude("amplitude_data.txt");
    ofstream file_phase("phase_data.txt");
    ofstream file_bode("bode_data.txt");
    ofstream file_real_imag("real_imag_data.txt");
    ofstream file_asymptotic("asymptotic_data.txt");
    ofstream file_lfch("lfch_data.txt");
    
    for (const auto& point : results) {
        file_nyquist << point[1] << " " << point[2] << endl;
        file_amplitude << point[0] << " " << point[3] << endl;
        file_phase << point[0] << " " << point[5] << endl;
        file_bode << point[0] << " " << point[4] << " " << point[5] << endl;
        file_real_imag << point[0] << " " << point[1] << " " << point[2] << endl;
        file_asymptotic << point[0] << " " << point[6] << endl;
        file_lfch << point[0] << " " << point[5] << " " << point[7] << endl;
    }
    
    file_nyquist.close();
    file_amplitude.close();
    file_phase.close();
    file_bode.close();
    file_real_imag.close();
    file_asymptotic.close();
    file_lfch.close();
    
    // Создание отдельных скриптов для каждого графика
    ofstream gnuplot_nyquist("plot_nyquist.gnu");
    ofstream gnuplot_amplitude("plot_amplitude.gnu");
    ofstream gnuplot_phase("plot_phase.gnu");
    ofstream gnuplot_bode("plot_bode.gnu");
    ofstream gnuplot_real_imag("plot_real_imag.gnu");
    ofstream gnuplot_lach("plot_lach.gnu");
    ofstream gnuplot_formulas("plot_formulas.gnu");
    ofstream gnuplot_links("plot_links.gnu");
    ofstream gnuplot_asymptotic("plot_asymptotic.gnu");
    ofstream gnuplot_lfch("plot_lfch.gnu");
    
    // 1. Годограф Найквиста
    gnuplot_nyquist << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_nyquist << "set output 'nyquist_plot.png'" << endl;
    gnuplot_nyquist << "set title 'Годограф Найквиста'" << endl;
    gnuplot_nyquist << "set xlabel 'Re(H(jω))' font 'Arial,12'" << endl;
    gnuplot_nyquist << "set ylabel 'Im(H(jω))' font 'Arial,12'" << endl;
    gnuplot_nyquist << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_nyquist << "set size square" << endl;
    gnuplot_nyquist << "set key top left" << endl;
    gnuplot_nyquist << "set arrow 1 from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_nyquist << "set arrow 2 from first 0, graph 0 to first 0, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_nyquist << "plot 'nyquist_data.txt' with lines lw 3 lc rgb 'blue' title 'Годограф'" << endl;
    
    // 2. АЧХ
    gnuplot_amplitude << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_amplitude << "set output 'amplitude_plot.png'" << endl;
    gnuplot_amplitude << "set title 'Амплитудно-частотная характеристика (АЧХ)'" << endl;
    gnuplot_amplitude << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_amplitude << "set ylabel 'A(ω)' font 'Arial,12'" << endl;
    gnuplot_amplitude << "set logscale x" << endl;
    gnuplot_amplitude << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_amplitude << "set key top right" << endl;
    gnuplot_amplitude << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_amplitude << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_amplitude << "plot 'amplitude_data.txt' with lines lw 3 lc rgb 'red' title 'A(ω)'" << endl;
    
    // 3. ЛАЧХ с асимптотикой
    gnuplot_lach << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_lach << "set output 'lach_plot.png'" << endl;
    gnuplot_lach << "set title 'ЛАЧХ точная и асимптотическая'" << endl;
    gnuplot_lach << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_lach << "set ylabel 'L(ω), дБ' font 'Arial,12'" << endl;
    gnuplot_lach << "set logscale x" << endl;
    gnuplot_lach << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_lach << "set key top right" << endl;
    gnuplot_lach << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_lach << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_lach << "plot 'bode_data.txt' using 1:2 with lines lw 3 lc rgb 'green' title 'Точная ЛАЧХ', \\" << endl;
    gnuplot_lach << "     'asymptotic_data.txt' with lines lw 2 lc rgb 'red' dt 2 title 'Асимптотическая ЛАЧХ'" << endl;
    
    // 4. ФЧХ
    gnuplot_phase << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_phase << "set output 'phase_plot.png'" << endl;
    gnuplot_phase << "set title 'Фазо-частотная характеристика (ФЧХ)'" << endl;
    gnuplot_phase << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_phase << "set ylabel 'φ(ω), градусы' font 'Arial,12'" << endl;
    gnuplot_phase << "set logscale x" << endl;
    gnuplot_phase << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_phase << "set key top right" << endl;
    gnuplot_phase << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_phase << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_phase << "plot 'phase_data.txt' with lines lw 3 lc rgb 'purple' title 'φ(ω)'" << endl;
    
    // 5. ЛФЧХ с асимптотикой
    gnuplot_lfch << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_lfch << "set output 'lfch_plot.png'" << endl;
    gnuplot_lfch << "set title 'Логарифмическая ФЧХ (ЛФЧХ)'" << endl;
    gnuplot_lfch << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_lfch << "set ylabel 'φ(ω), градусы' font 'Arial,12'" << endl;
    gnuplot_lfch << "set logscale x" << endl;
    gnuplot_lfch << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_lfch << "set key top right" << endl;
    gnuplot_lfch << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_lfch << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_lfch << "plot 'lfch_data.txt' using 1:2 with lines lw 3 lc rgb 'purple' title 'Точная ФЧХ', \\" << endl;
    gnuplot_lfch << "     'lfch_data.txt' using 1:3 with lines lw 2 lc rgb 'orange' dt 2 title 'Асимптотическая ФЧХ'" << endl;
    
    // 6. Действительная и мнимая части
    gnuplot_real_imag << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_real_imag << "set output 'real_imag_plot.png'" << endl;
    gnuplot_real_imag << "set title 'Действительная и мнимая части передаточной функции'" << endl;
    gnuplot_real_imag << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_real_imag << "set ylabel 'Re(H), Im(H)' font 'Arial,12'" << endl;
    gnuplot_real_imag << "set logscale x" << endl;
    gnuplot_real_imag << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_real_imag << "set key top right" << endl;
    gnuplot_real_imag << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_real_imag << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_real_imag << "plot 'real_imag_data.txt' using 1:2 with lines lw 3 lc rgb 'blue' title 'Re(H)', \\" << endl;
    gnuplot_real_imag << "     'real_imag_data.txt' using 1:3 with lines lw 3 lc rgb 'orange' title 'Im(H)'" << endl;
    
    // 7. Диаграмма Боде (ЛАФЧХ)
    gnuplot_bode << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_bode << "set output 'bode_plot.png'" << endl;
    gnuplot_bode << "set title 'Диаграмма Боде (ЛАФЧХ)'" << endl;
    gnuplot_bode << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_bode << "set ylabel 'L(ω), дБ' font 'Arial,12'" << endl;
    gnuplot_bode << "set y2label 'φ(ω), градусы' font 'Arial,12'" << endl;
    gnuplot_bode << "set logscale x" << endl;
    gnuplot_bode << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_bode << "set y2tics" << endl;
    gnuplot_bode << "set key top right" << endl;
    gnuplot_bode << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_bode << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_bode << "plot 'bode_data.txt' using 1:2 with lines lw 3 lc rgb 'green' title 'L(ω)' axes x1y1, \\" << endl;
    gnuplot_bode << "     'bode_data.txt' using 1:3 with lines lw 3 lc rgb 'purple' title 'φ(ω)' axes x1y2" << endl;
    
    // 8. Формулы - отдельное изображение
    gnuplot_formulas << "set terminal pngcairo size 1000,600 enhanced font 'Arial,16'" << endl;
    gnuplot_formulas << "set output 'formulas_plot.png'" << endl;
    gnuplot_formulas << "unset border" << endl;
    gnuplot_formulas << "unset tics" << endl;
    gnuplot_formulas << "unset key" << endl;
    gnuplot_formulas << "set xrange [0:1]" << endl;
    gnuplot_formulas << "set yrange [0:1]" << endl;
    gnuplot_formulas << "set label 1 at 0.5,0.9 'Исходная передаточная функция:' center front font 'Arial-Bold,18'" << endl;
    gnuplot_formulas << "set label 2 at 0.5,0.8 'H(jω) = {200} / {0.001(jω)^3 + 0.07(jω)^2 + jω + 200}' center front font 'Arial,16'" << endl;
    gnuplot_formulas << "set label 3 at 0.5,0.65 'Действительная часть:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_formulas << "set label 4 at 0.5,0.55 'Re(H) = {200(200 - 0.07ω^2)} / {(200 - 0.07ω^2)^2 + ω^2(1 - 0.001ω^2)^2}' center front font 'Arial,14'" << endl;
    gnuplot_formulas << "set label 5 at 0.5,0.4 'Мнимая часть:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_formulas << "set label 6 at 0.5,0.3 'Im(H) = {-200ω(1 - 0.001ω^2)} / {(200 - 0.07ω^2)^2 + ω^2(1 - 0.001ω^2)^2}' center front font 'Arial,14'" << endl;
    gnuplot_formulas << "set label 7 at 0.5,0.15 'Амплитуда:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_formulas << "set label 8 at 0.5,0.05 'A(ω) = {200} / {√[(200 - 0.07ω^2)^2 + ω^2(1 - 0.001ω^2)^2]}' center front font 'Arial,14'" << endl;
    gnuplot_formulas << "plot 2" << endl;
    
        // 9. Разложение на простые звенья - ИСПРАВЛЕННЫЙ ВАРИАНТ
    gnuplot_links << "set terminal pngcairo size 1400,1000 enhanced font 'Arial,14'" << endl;
    gnuplot_links << "set output 'links_plot.png'" << endl;
    gnuplot_links << "unset border" << endl;
    gnuplot_links << "unset tics" << endl;
    gnuplot_links << "unset key" << endl;
    gnuplot_links << "set xrange [0:1]" << endl;
    gnuplot_links << "set yrange [0:1.2]" << endl;  // Увеличили высоту области
    
    // Заголовки
    gnuplot_links << "set label 1 at 0.5,1.15 'Разложение передаточной функции на простые звенья' center front font 'Arial-Bold,20'" << endl;
    gnuplot_links << "set label 2 at 0.5,1.08 'Исходная функция:' center front font 'Arial-Bold,18'" << endl;
    gnuplot_links << "set label 3 at 0.5,1.02 'H(s) = {200} / {0.001s^3 + 0.07s^2 + s + 200}' center front font 'Arial,16'" << endl;
    gnuplot_links << "set label 4 at 0.5,0.95 'Разложение на множители:' center front font 'Arial-Bold,18'" << endl;
    gnuplot_links << "set label 5 at 0.5,0.89 'H(s) = 200 × {1} / {0.05s + 1} × (0.02s + 1) × (0.001s + 1) × {1} / {0.0001s + 1}' center front font 'Arial,14'" << endl;
    
    // Заголовок раздела звеньев
    gnuplot_links << "set label 6 at 0.5,0.82 'Простые звенья:' center front font 'Arial-Bold,18'" << endl;
    
    // СЕТКА ЗВЕНЬЕВ - равномерное распределение по 5 строкам
    
    // Строка 1
    gnuplot_links << "set label 7 at 0.2,0.75 '1. Усилительное звено:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_links << "set label 8 at 0.2,0.70 'K = 200' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 9 at 0.2,0.65 'L(ω) = 20lg(200) ≈ 46 дБ' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 10 at 0.2,0.60 'φ(ω) = 0°' center front font 'Arial,14'" << endl;
    
    // Строка 2
    gnuplot_links << "set label 11 at 0.5,0.75 '2. Апериодическое звено:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_links << "set label 12 at 0.5,0.70 'W(s) = 1 / (0.05s + 1)' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 13 at 0.5,0.65 'ω_{ср} = 1/0.05 = 20 рад/с' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 14 at 0.5,0.60 'φ(ω): 0° → -90°' center front font 'Arial,14'" << endl;
    
    // Строка 3
    gnuplot_links << "set label 15 at 0.8,0.75 '3. Форсирующее звено:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_links << "set label 16 at 0.8,0.70 'W(s) = 0.02s + 1' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 17 at 0.8,0.65 'ω_{ср} = 1/0.02 = 50 рад/с' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 18 at 0.8,0.60 'φ(ω): 0° → +90°' center front font 'Arial,14'" << endl;
    
    // Строка 4
    gnuplot_links << "set label 19 at 0.2,0.45 '4. Форсирующее звено:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_links << "set label 20 at 0.2,0.40 'W(s) = 0.001s + 1' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 21 at 0.2,0.35 'ω_{ср} = 1/0.001 = 1000 рад/с' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 22 at 0.2,0.30 'φ(ω): 0° → +90°' center front font 'Arial,14'" << endl;
    
    // Строка 5
    gnuplot_links << "set label 23 at 0.5,0.45 '5. Апериодическое звено:' center front font 'Arial-Bold,16'" << endl;
    gnuplot_links << "set label 24 at 0.5,0.40 'W(s) = 1 / (0.0001s + 1)' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 25 at 0.5,0.35 'ω_{ср} = 1/0.0001 = 10000 рад/с' center front font 'Arial,14'" << endl;
    gnuplot_links << "set label 26 at 0.5,0.30 'φ(ω): 0° → -90°' center front font 'Arial,14'" << endl;
    
    // Итоговая строка
    gnuplot_links << "set label 27 at 0.5,0.15 'Асимптотическая ЛАЧХ и ЛФЧХ строятся суммированием характеристик всех звеньев' center front font 'Arial,12'" << endl;
    
    // Разделительные линии для наглядности (опционально)
    gnuplot_links << "set arrow from 0.05,0.5 to 0.95,0.5 nohead lc 'gray' dt 2" << endl;
    gnuplot_links << "set arrow from 0.33,0.2 to 0.33,0.8 nohead lc 'gray' dt 2" << endl;
    gnuplot_links << "set arrow from 0.66,0.2 to 0.66,0.8 nohead lc 'gray' dt 2" << endl;
    
    gnuplot_links << "plot 2" << endl;
    
    // 10. Асимптотическая ЛАЧХ отдельно
    gnuplot_asymptotic << "set terminal pngcairo size 800,600 enhanced font 'Arial,14'" << endl;
    gnuplot_asymptotic << "set output 'asymptotic_plot.png'" << endl;
    gnuplot_asymptotic << "set title 'Асимптотическая ЛАЧХ'" << endl;
    gnuplot_asymptotic << "set xlabel 'ω, рад/с' font 'Arial,12'" << endl;
    gnuplot_asymptotic << "set ylabel 'L(ω), дБ' font 'Arial,12'" << endl;
    gnuplot_asymptotic << "set logscale x" << endl;
    gnuplot_asymptotic << "set grid linecolor rgb '#DDDDDD'" << endl;
    gnuplot_asymptotic << "set key top right" << endl;
    gnuplot_asymptotic << "set xrange [0.1:100000]" << endl;
    gnuplot_asymptotic << "set arrow from graph 0, first 0 to graph 1, first 0 nohead lc 'black' lw 1.5" << endl;
    gnuplot_asymptotic << "set arrow from first 0.1, graph 0 to first 0.1, graph 1 nohead lc 'black' lw 1.5" << endl;
    gnuplot_asymptotic << "plot 'asymptotic_data.txt' with lines lw 3 lc rgb 'red' title 'Асимптотическая ЛАЧХ'" << endl;
    
    gnuplot_nyquist.close();
    gnuplot_amplitude.close();
    gnuplot_phase.close();
    gnuplot_bode.close();
    gnuplot_real_imag.close();
    gnuplot_lach.close();
    gnuplot_formulas.close();
    gnuplot_links.close();
    gnuplot_asymptotic.close();
    gnuplot_lfch.close();
    
    cout << "Данные сохранены в файлы:" << endl;
    cout << "nyquist_data.txt      - годограф Найквиста" << endl;
    cout << "amplitude_data.txt    - АЧХ" << endl;
    cout << "phase_data.txt        - ФЧХ" << endl;
    cout << "bode_data.txt         - ЛАФЧХ" << endl;
    cout << "real_imag_data.txt    - Re и Im в табличной форме" << endl;
    cout << "asymptotic_data.txt   - асимптотическая ЛАЧХ" << endl;
    cout << "lfch_data.txt         - ЛФЧХ с асимптотикой" << endl;
    
    cout << "\nСкрипты для построения графиков созданы:" << endl;
    cout << "plot_nyquist.gnu     - годограф Найквиста" << endl;
    cout << "plot_amplitude.gnu   - АЧХ" << endl;
    cout << "plot_lach.gnu        - ЛАЧХ с асимптотикой" << endl;
    cout << "plot_phase.gnu       - ФЧХ" << endl;
    cout << "plot_lfch.gnu        - ЛФЧХ с асимптотикой" << endl;
    cout << "plot_real_imag.gnu   - Re и Im части" << endl;
    cout << "plot_bode.gnu        - диаграмма Боде" << endl;
    cout << "plot_formulas.gnu    - формулы (Re, Im, A)" << endl;
    cout << "plot_links.gnu       - разложение на простые звенья" << endl;
    cout << "plot_asymptotic.gnu  - асимптотическая ЛАЧХ" << endl;
    
    cout << "\nДля построения всех графиков выполните:" << endl;
    cout << "for script in plot_*.gnu; do gnuplot \"$script\"; done" << endl;
    
    return 0;
}