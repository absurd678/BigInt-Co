#include "bigint.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;
#define DIGITS_TO_GEN 10

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
    BigInt maxInt(numeric_limits<int>::max());
    
    for (size_t i = 0; i < count; ++i) {
        temp.initLCG(numDigits);
        // Use modulo to fit in int range
        BigInt safeVal = temp % (maxInt + BigInt(1));
        results.push_back(safeVal.toInt());
    }
    return results;
}

// Генерация N чисел с помощью MT (своя реализация)
vector<int> generateMT(size_t count, size_t numDigits) {
    vector<int> results;
    BigInt temp;
    BigInt maxInt(numeric_limits<int>::max());
    
    for (size_t i = 0; i < count; ++i) {
        temp.initMT(numDigits);
        BigInt safeVal = temp % (maxInt + BigInt(1));
        results.push_back(safeVal.toInt());
    }
    return results;
}

// Генерация N чисел с помощью энтропии мыши
vector<int> generateMouseEntropy(size_t count, size_t numDigits) {
    vector<int> results;
    BigInt temp;
    BigInt maxInt(numeric_limits<int>::max());
    
    for (size_t i = 0; i < count; ++i) {
        temp.initFromMouseEntropy(numDigits, 50, 200);
        BigInt safeVal = temp % (maxInt + BigInt(1));
        results.push_back(safeVal.toInt());
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
        auto lcg_data = generateLCG(size, DIGITS_TO_GEN); // 10 цифр — точно влезет в int
        saveToCSV(lcg_data, prefixes[0] + to_string(size) + ".csv");

        // MT
        cout << "  Using MT..." << endl;
        auto mt_data = generateMT(size, DIGITS_TO_GEN);
        saveToCSV(mt_data, prefixes[1] + to_string(size) + ".csv");

        // Mouse Entropy
        cout << "  Using Mouse Entropy..." << endl;
        auto ent_data = generateMouseEntropy(size, DIGITS_TO_GEN);
        saveToCSV(ent_data, prefixes[2] + to_string(size) + ".csv");
    }

    cout << "All data generated and saved to 'data/' directory." << endl;
    return 0;
}
