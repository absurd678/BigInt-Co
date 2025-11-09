// Этот код только рисует графики!
#include "generate.h"   // Генерация чисел 100, 1000, 10000
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// Функция чтения CSV в вектор int
vector<int> readCSV(const string& filepath) {
    vector<int> data;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filepath << endl;
        return data;
    }

    string line;
    while (getline(file, line)) {
        try {
            int val = stoi(line);
            data.push_back(val);
        } catch (...) {
            // Пропускаем некорректные строки
        }
    }
    file.close();
    return data;
}

// Вычисление среднего
double mean(const vector<int>& data) {
    if (data.empty()) return 0.0;
    long long sum = 0;
    for (int x : data) sum += x;
    return static_cast<double>(sum) / data.size();
}

// Вычисление дисперсии
double variance(const vector<int>& data, double mean_val) {
    if (data.size() < 2) return 0.0;
    double var = 0.0;
    for (int x : data) {
        double diff = x - mean_val;
        var += diff * diff;
    }
    return var / data.size(); // Деление на N (можно на N-1 для несмещённой)
}

// Подсчёт совпадающих значений (сколько раз повторяются числа)
int countDuplicates(const vector<int>& data) {
    map<int, int> freq;
    for (int x : data) {
        freq[x]++; //  сколько раз встречается число
    }
    int duplicates = 0;
    for (const auto& p : freq) {
        if (p.second > 1) {
            duplicates += p.second - 1; //  сколько совпадений

        }
    }
    return duplicates;
}

// Сохранение данных в формате X Y (индекс, значение) для gnuplot
void saveForGnuplot(const vector<int>& data, const string& filename) {
    ofstream file(filename);
    for (size_t i = 0; i < data.size(); ++i) {
        file << i << " " << data[i] << "\n";
    }
    file.close();
}

// Вызов gnuplot с командой
void runGnuplot(const string& plotCmd) {
    string cmd = "echo \"" + plotCmd + "\" | gnuplot -persist";
    system(cmd.c_str());
}

// Отрисовка графика через gnuplot
void plotData(const string& dataFile, const string& title, double mean_val, double var_val, int dup_val) {
    stringstream cmd;
    cmd << "set title '" << title << "' font \",14\";\n"
        << "set xlabel 'Index';\n"
        << "set ylabel 'Value';\n"
        << "set grid;\n"
        << "set key top right;\n"
        << "stats '" << dataFile << "' nooutput;\n"
        << "mean_f = " << mean_val << ";\n"
        << "var_f = " << var_val << ";\n"
        << "set label 1 sprintf('Mean: %.2f\\nVar: %.2f\\nDups: %d', mean_f, var_f, " << dup_val << ") at graph 0.15, 0.85;\n"
        << "plot '" << dataFile << "' with dots lc rgb 'blue' title 'Data';\n";

    runGnuplot(cmd.str());
}

int main() {
    run(); // Генерация данных
    vector<string> prefixes = {"lcg", "mt", "entropy"};
    vector<int> sizes = {100, 1000, 10000};

    // Создаём временную директорию для gnuplot данных
    const string tempDir = "../plots_data";
    if (!fs::exists(tempDir)) {
        fs::create_directories(tempDir);
    }

    cout << "Reading files and computing statistics...\n";

    for (const auto& prefix : prefixes) {
        for (int size : sizes) {
            string filename = "../data/" + prefix + to_string(size) + ".csv";
            cout << "\nProcessing: " << filename << endl;

            vector<int> data = readCSV(filename);
            if (data.empty()) {
                cerr << "No data read from " << filename << endl;
                continue;
            }

            double mean_val = mean(data);
            double var_val = variance(data, mean_val);
            int dup_count = countDuplicates(data);

            cout << "  Mean: " << mean_val << endl;
            cout << "  Variance: " << var_val << endl;
            cout << "  Duplicate values count: " << dup_count << endl;

            // Сохраняем данные в формате для gnuplot: index value
            string tempFile = tempDir + "/" + prefix + to_string(size) + ".dat";
            saveForGnuplot(data, tempFile);

            // Строим график
            string title = prefix + " " + to_string(size) + " values";
            plotData(tempFile, title, mean_val, var_val, dup_count);
        }
    }

    cout << "\nAll plots displayed. Close each window to continue." << endl;
    return 0;
}
