/*# Compile
g++ -std=c++11 -pthread -I. -o rsa ZI_LR2.cpp

# Run
./rsa
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <locale.h>

#include "Prime.h"
#include "RSA.h"

using namespace std;

// Чтение первой непустой строки из файла
static bool read_line_trim(const string& path, string& out) {
    ifstream fin(path);
    if (!fin) {
        cerr << "Cannot open file: " << path << endl;
        return false;
    }
    
    string s;
    while (getline(fin, s)) {
        // Trim пробелы
        size_t l = s.find_first_not_of(" \t\r\n");
        if (l != string::npos) {
            size_t r = s.find_last_not_of(" \t\r\n");
            out = s.substr(l, r - l + 1);
            return true;
        }
    }
    
    cerr << "File is empty: " << path << endl;
    return false;
}

// Чтение всего файла одной строкой (для открытого текста)
static bool read_whole_line(const string& path, string& out) {
    ifstream fin(path);
    if (!fin) {
        cerr << "Cannot open file: " << path << endl;
        return false;
    }
    
    if (!getline(fin, out)) {
        cerr << "Cannot read from file: " << path << endl;
        return false;
    }
    
    return true;
}

int main() {
    setlocale(LC_ALL, "Ru");
    cout << "=== RSA Encryption/Decryption Program ===" << endl;
    
    // Входные файлы
    const string p_file = "p_lit.txt";
    const string q_file = "q_lit.txt";
    const string pt_file = "plaintext.txt";
    
    // Выходные файлы
    const string ct_file = "cipher.txt";
    const string dt_file = "decrypt.txt";
    
    /*
    // Создаем тестовые файлы, если их нет
    {
        
        ofstream f(p_file);
        if (f) f << "101" << endl;  // Простое число
        f.close();
        
        f.open(q_file);
        if (f) f << "103" << endl;  // Простое число  
        f.close();
        

        f.open(pt_file);
        if (f) f << "Hello RSA!" << endl;
        f.close();
    }
    */
    cout << "1. Reading input files..." << endl;
    
    // Читаем p, q и plaintext
    string p_str, q_str, plaintext;
    if (!read_line_trim(p_file, p_str)) {
        cerr << "Error reading " << p_file << endl;
        return 1;
    }
    
    if (!read_line_trim(q_file, q_str)) {
        cerr << "Error reading " << q_file << endl;
        return 1;
    }
    
    if (!read_whole_line(pt_file, plaintext)) {
        cerr << "Error reading " << pt_file << endl;
        return 1;
    }
    
    cout << "   p = " << p_str << endl;
    cout << "   q = " << q_str << endl;
    cout << "   plaintext = \"" << plaintext << "\"" << endl;
    
    cout << "\n2. Converting to BigInt..." << endl;
    // Преобразование строк в BigInt для работы с большими числами
    BigInt p(p_str), q(q_str);
    
    cout << "\n3. Generating RSA keys..." << endl;
    // Генерация RSA ключей: n = p*q, вычисление e и d
    BigInt n, e, d;
    RSA_Initialize_FromPQ(p, q, n, e, d);
    
    cout << "\n4. Encrypting plaintext..." << endl;
    // Шифрование: каждый символ преобразуется в c = m^e mod n
    vector<string> cipher;
    RSA_Encrypt_FromKeys(plaintext, n, e, cipher);
    
    cout << "\n5. Writing ciphertext to file..." << endl;
    // Запись шифртекста в файл
    ofstream fout_ct(ct_file);
    if (!fout_ct) { 
        cerr << "Error writing to " << ct_file << endl; 
        return 1; 
    }
    
    for (size_t i = 0; i < cipher.size(); ++i) {
        if (i) fout_ct << ' ';
        fout_ct << cipher[i];
    }
    fout_ct << "\n";
    fout_ct.close();
    cout << "   Ciphertext written to " << ct_file << endl;
    
    cout << "\n6. Decrypting ciphertext..." << endl;
    // Дешифрование (m = c^d mod n)
    vector<string> decrypted_nums;
    RSA_Decrypt_FromKeys_CRT_Fast(cipher, n, d, p, q, decrypted_nums);
    
    cout << "\n7. Writing decrypted numbers to file..." << endl;
    // Запись расшифрованных чисел в файл
    ofstream fout_dt(dt_file);
    if (!fout_dt) { 
        cerr << "Error writing to " << dt_file << endl; 
        return 1; 
    }
    
    for (size_t i = 0; i < decrypted_nums.size(); ++i) {
        if (i) fout_dt << ' ';
        fout_dt << decrypted_nums[i];
    }
    fout_dt << "\n";
    fout_dt.close();
    cout << "   Decrypted numbers written to " << dt_file << endl;
    
    cout << "\n8. Converting decrypted numbers back to text..." << endl;
    // Преобразование расшифрованных чисел обратно в текст
    bool ok_text = true;
    string recovered;
    
    for (const string& num : decrypted_nums) {
        long long v = 0;
        for (char c : num) { 
            v = v * 10 + (c - '0'); 
            if (v > 255) { 
                ok_text = false; 
                break; 
            }
        }
        
        if (!ok_text) break;
        recovered.push_back(static_cast<char>(v));
    }
    
    if (ok_text) {
        cout << "   Recovered plaintext: \"" << recovered << "\"" << endl;
        
        // Проверка
        if (recovered == plaintext) {
            cout << "\n✓ SUCCESS: Original and recovered texts match!" << endl;
        } else {
            cout << "\n✗ FAILURE: Original and recovered texts DO NOT match!" << endl;
            cout << "   Original: \"" << plaintext << "\"" << endl;
            cout << "   Recovered: \"" << recovered << "\"" << endl;
        }
    } else {
        cout << "   Error: Could not convert numbers back to text" << endl;
    }
    
    cout << "\n=== Program completed ===" << endl;
    return 0;
}