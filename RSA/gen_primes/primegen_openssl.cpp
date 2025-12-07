#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <chrono>
#include <string>
#include <cstring>

#include <openssl/bn.h>
#include <openssl/rand.h>

static std::string bn_to_dec(const BIGNUM* bn) {
    char* s = BN_bn2dec(bn);
    if (!s) return {};
    std::string out(s);
    OPENSSL_free(s);
    return out;
}

// Случайная инициализация OpenSSL RNG на поток
static void seed_thread_rng(uint64_t salt) {
    std::mt19937_64 prng(
        (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count() ^ salt
    );
    unsigned char buf[64];
    for (int i = 0; i < 64; ++i) buf[i] = (unsigned char)(prng() & 0xFF);
    RAND_seed(buf, sizeof(buf));
}

// Генерация одного k-битного простого через BN_generate_prime_ex
static bool generate_prime_bits(int bits, BIGNUM* out, BN_CTX* ctx) {
    // flags=BN_GENCB_NULL, add=NULL, rem=NULL
    // BN_generate_prime_ex: гарантирует старший бит и нечётность
    return BN_generate_prime_ex(out, bits, 0, nullptr, nullptr, nullptr) == 1;
}

struct Job {
    int bits;
    std::atomic<int>* need;
    std::atomic<int>* found;
    std::mutex* mx;
    std::ofstream* fout;
    std::atomic<bool>* stop;
    uint64_t seed;
};

static void worker(Job job) {
    seed_thread_rng(job.seed);

    BN_CTX* ctx = BN_CTX_new();
    BIGNUM* p = BN_new();
    if (!ctx || !p) { if (ctx) BN_CTX_free(ctx); if (p) BN_free(p); return; }

    while (!job.stop->load(std::memory_order_relaxed)) {
        if (job.need->load(std::memory_order_relaxed) <= 0) break;

        if (!generate_prime_bits(job.bits, p, ctx)) continue;

        int prev = job.need->fetch_sub(1);
        if (prev <= 0) break;

        std::string s = bn_to_dec(p);
        {
            std::lock_guard<std::mutex> lk(*job.mx);
            if (job.fout && *job.fout) {
                *job.fout << s << "\n";
                job.fout->flush();
                int idx = job.found->fetch_add(1) + 1;
                std::cerr << "[ok] prime #" << idx << " (" << job.bits << "-bit)\n";
            }
        }
        if (job.need->load() <= 0) break;
    }

    BN_free(p);
    BN_CTX_free(ctx);
}

int main(int argc, char** argv) {
    int bits = 32768;
    int count = 2;
    std::string out = "primes.txt";
    int threads = std::thread::hardware_concurrency() ? (int)std::thread::hardware_concurrency() : 8;

    if (argc >= 2) bits = std::max(128, atoi(argv[1]));
    if (argc >= 3) count = std::max(1, atoi(argv[2]));
    if (argc >= 4) out = argv[3];
    if (argc >= 5) threads = std::max(1, atoi(argv[4]));

    std::cerr << "[cfg] bits=" << bits << ", count=" << count
              << ", threads=" << threads << ", outfile=" << out << "\n";

    std::ofstream fout(out, std::ios::trunc);
    if (!fout) { std::cerr << "Cannot open output: " << out << "\n"; return 1; }

    std::atomic<int> need(count), found(0);
    std::atomic<bool> stop(false);
    std::mutex mx;

    // Размножаем потоки
    std::vector<std::thread> pool;
    pool.reserve(threads);
    uint64_t seed0 = (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();

    for (int i=0;i<threads;i++) {
        Job j { bits, &need, &found, &mx, &fout, &stop, seed0 ^ (0x9E3779B97F4A7C15ULL * (i+1)) };
        pool.emplace_back(worker, j);
    }

    for (auto& t: pool) t.join();
    stop = true;

    std::cerr << "[done] found=" << found.load() << ", saved to " << out << "\n";
    return 0;
}
