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

// Read ciphertext from file (multiple lines)
static bool read_ciphertext(const string& path, vector<string>& out) {
    ifstream fin(path);
    if (!fin) {
        cerr << "Cannot open file: " << path << endl;
        return false;
    }
    
    out.clear();
    string line;
    
    // Read number of blocks (first line)
    if (!getline(fin, line)) {
        cerr << "Cannot read number of blocks from: " << path << endl;
        return false;
    }
    
    int numBlocks;
    try {
        numBlocks = stoi(line);
    } catch (...) {
        cerr << "Invalid number of blocks in: " << path << endl;
        return false;
    }
    
    // Read each block
    for (int i = 0; i < numBlocks; ++i) {
        if (!getline(fin, line)) {
            cerr << "Cannot read block " << i << " from: " << path << endl;
            return false;
        }
        out.push_back(line);
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
    cout << "   plaintext length = " << plaintext.length() << " characters" << endl;
    
    cout << "\n2. Converting to BigInt..." << endl;
    // Преобразование строк в BigInt для работы с большими числами
    BigInt p(p_str), q(q_str);
    
    cout << "\n3. Generating RSA keys..." << endl;
    // Генерация RSA ключей: n = p*q, вычисление e и d
    BigInt n, e, d;
    RSA_Initialize_FromPQ(p, q, n, e, d);
    
    //cout << "\n4. Calculating maximum block size..." << endl;
    size_t maxBlockSize = GetMaxMessageSize(n);
    //cout << "   Can encrypt " << maxBlockSize << " characters per block" << endl;
    
    if (maxBlockSize == 0) {
        cerr << "Error: RSA modulus n is too small to encrypt even one character!" << endl;
        cerr << "n = " << n << endl;
        cerr << "Try using larger prime numbers." << endl;
        return 1;
    }
    
    cout << "\n4. Encrypting plaintext..." << endl;
    vector<string> cipher;
    RSA_Encrypt_Block(plaintext, n, e, cipher);
    
    if (cipher.empty()) {
        cerr << "Error: Encryption failed!" << endl;
        return 1;
    }
    
    cout << "\n5. Writing ciphertext to file..." << endl;
    ofstream fout_ct(ct_file);
    if (!fout_ct) { 
        cerr << "Error writing to " << ct_file << endl; 
        return 1; 
    }

    // Write number of blocks first
    fout_ct << cipher.size() << "\n";
    for (size_t i = 0; i < cipher.size(); ++i) {
        fout_ct << cipher[i] << "\n";
    }
    fout_ct.close();
    cout << "   Ciphertext written to " << ct_file << endl; //" (" << cipher.size() << " blocks)" << endl;
    
    cout << "\n6. Decrypting ciphertext..." << endl;
    string recovered = RSA_Decrypt_Block(cipher, n, d, p, q);
    
    cout << "\n7. Writing decrypted text to file..." << endl;
    ofstream fout_dt(dt_file);
    if (!fout_dt) { 
        cerr << "Error writing to " << dt_file << endl; 
        return 1; 
    }
    fout_dt << recovered << "\n";
    fout_dt.close();
    cout << "   Decrypted text written to " << dt_file << endl;
    
    cout << "\n9. Verifying decryption..." << endl;
    cout << "   Original plaintext: \"" << plaintext << "\"" << endl;
    cout << "   Recovered plaintext: \"" << recovered << "\"" << endl;
    
    if (recovered == plaintext) {
        cout << "\n✓ SUCCESS: Original and recovered texts match!" << endl;
    } else {
        cout << "\n✗ FAILURE: Original and recovered texts DO NOT match!" << endl;
        // Find where they differ
        size_t minLen = min(plaintext.length(), recovered.length());
        for (size_t i = 0; i < minLen; ++i) {
            if (plaintext[i] != recovered[i]) {
                cout << "   First difference at position " << i 
                     << ": original='" << plaintext[i] 
                     << "' (ASCII " << (int)plaintext[i] << "), "
                     << "recovered='" << recovered[i] 
                     << "' (ASCII " << (int)recovered[i] << ")" << endl;
                break;
            }
        }
        if (plaintext.length() != recovered.length()) {
            cout << "   Length mismatch: original=" << plaintext.length() 
                 << ", recovered=" << recovered.length() << endl;
        }
    }
    
    cout << "\n=== Program completed ===" << endl;
    return 0;
}