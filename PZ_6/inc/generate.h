#include "bigint.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

// Вспомогательная функция записи в CSV
void saveToCSV(const vector<int>& data, const string& filename) {
    ofstream file("../data/" + filename);
    // if (!file.is_open()) {
    //     cerr << "Cannot open file for writing: " << filename << endl;
    //     return;
    // }
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i];
        if (i != data.size() - 1) file << '\n';
    }
    file.close();
}

// Генерация N чисел с помощью LCG
vector<int> generateLCG(size_t count, size_t numDigits) {
    vector<int> results;
    BigInt temp;
    for (size_t i = 0; i < count; ++i) {
        temp.initLCG(numDigits);  // Автоматически по времени
        try {
            int val = temp.toInt();
            results.push_back(val);
        } catch (const overflow_error&) {
            // Если не влезло в int — кладём 0 (или можно пропустить)
            results.push_back(0);
        }
    }
    return results;
}

// Генерация N чисел с помощью MT (своя реализация)
vector<int> generateMT(size_t count, size_t numDigits) {
    vector<int> results;
    BigInt temp;
    for (size_t i = 0; i < count; ++i) {
        temp.initMT(numDigits);
        try {
            int val = temp.toInt();
            results.push_back(val);
        } catch (const overflow_error&) {
            results.push_back(0);
        }
    }
    return results;
}

// Генерация N чисел с помощью энтропии мыши
vector<int> generateMouseEntropy(size_t count, size_t numDigits) {
    vector<int> results;
    BigInt temp;
    for (size_t i = 0; i < count; ++i) {
        temp.initFromMouseEntropy(numDigits, 50, 200); // быстро собрать энтропию
        try {
            int val = temp.toInt();
            results.push_back(val);
        } catch (const overflow_error&) {
            results.push_back(0);
        }
    }
    return results;
}

int run() {
    // Создаём папку data, если её нет (требуется вручную или через скрипт)
    // В Linux: system("mkdir -p data"); (опционально раскомментировать)
    system("mkdir -p ../data");

    vector<size_t> sizes = {100, 1000, 10000};
    vector<string> prefixes = {"lcg", "mt", "entropy"};

    for (size_t size : sizes) {
        cout << "Generating " << size << " numbers..." << endl;

        // LCG
        cout << "  Using LCG..." << endl;
        auto lcg_data = generateLCG(size, 10); // 10 цифр — точно влезет в int
        saveToCSV(lcg_data, prefixes[0] + to_string(size) + ".csv");

        // MT
        cout << "  Using MT..." << endl;
        auto mt_data = generateMT(size, 10);
        saveToCSV(mt_data, prefixes[1] + to_string(size) + ".csv");

        // Mouse Entropy
        cout << "  Using Mouse Entropy..." << endl;
        auto ent_data = generateMouseEntropy(size, 10);
        saveToCSV(ent_data, prefixes[2] + to_string(size) + ".csv");
    }

    cout << "All data generated and saved to 'data/' directory." << endl;
    return 0;
}
