#include <chrono>
// ---------- Собственная реализация MT19937 ----------
struct MT19937lab {
    static constexpr uint32_t N = 624;
    static constexpr uint32_t M = 397;
    static constexpr uint32_t MATRIX_A   = 0x9908B0DFu; // a
    static constexpr uint32_t UPPER_MASK = 0x80000000u; // most significant w-r bits
    static constexpr uint32_t LOWER_MASK = 0x7FFFFFFFu; // least significant r bits

    uint32_t mt[N];
    uint32_t idx;

    explicit MT19937lab(uint32_t seed = 5489u) { seed_mt(seed); }

    void seed_mt(uint32_t seed) {
        mt[0] = seed;
        for (idx = 1; idx < N; ++idx) {
            // mt[i] = (1812433253 * (mt[i-1] ^ (mt[i-1] >> 30)) + i)
            uint32_t x = mt[idx - 1] ^ (mt[idx - 1] >> 30);
            mt[idx] = 1812433253u * x + idx;
        }
        idx = N; // чтобы при первом вызове случился twist()
    }

    void twist() {
        for (uint32_t i = 0; i < N; ++i) {
            uint32_t y = (mt[i] & UPPER_MASK) | (mt[(i + 1) % N] & LOWER_MASK);
            uint32_t yA = y >> 1;
            if (y & 0x1u) yA ^= MATRIX_A;
            mt[i] = mt[(i + M) % N] ^ yA;
        }
        idx = 0;
    }

    uint32_t next_u32() {
        if (idx >= N) twist();

        uint32_t y = mt[idx++];

        // tempering
        y ^= (y >> 11);
        y ^= (y << 7)  & 0x9D2C5680u;
        y ^= (y << 15) & 0xEFC60000u;
        y ^= (y >> 18);

        return y;
    }

    // равномерно в [0, bound] (bound <= 0x7FFFFFFF) — через устранение смещения
    uint32_t uniform_u32(uint32_t bound) {
        // возвращаем x в [0, bound] с отсутствием байаса (rejection sampling)
        uint64_t range = static_cast<uint64_t>(bound) + 1ull;
        uint64_t limit = (0x100000000ull / range) * range; // ближайшее кратное
        uint32_t x;
        do {
            x = next_u32();
        } while (static_cast<uint64_t>(x) >= limit);
        return static_cast<uint32_t>(static_cast<uint64_t>(x) % range);
    }

    // целое в [lo, hi]
    int uniform_int(int lo, int hi) {
        uint32_t bound = static_cast<uint32_t>(hi - lo);
        return lo + static_cast<int>(uniform_u32(bound));
    }
};