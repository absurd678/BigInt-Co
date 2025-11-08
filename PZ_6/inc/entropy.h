#include <chrono>
#include <thread>
#include <cstdint>
#include <vector>
#include <cstdlib>

#if defined(__linux__)
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#endif

// Быстрый «whitening» миксер: splitmix64
static inline uint64_t mix64lab(uint64_t z){
    z += 0x9E3779B97F4A7C15ull;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// Сэмплер курсора для Linux/X11 (работает и в WSL2 при наличии X-сервера)
struct CursorSamplerX11 {
    struct Sample { int x, y; uint64_t t; };

    static std::vector<Sample> collect(int duration_ms, int poll_hz){
        std::vector<Sample> out;

    #if defined(__linux__)
        const char* dispName = std::getenv("DISPLAY");
        Display* dpy = XOpenDisplay(dispName ? dispName : nullptr);
        if(!dpy){
            // Нет X-сервера / DISPLAY — вернём пусто (перейдём на fallback)
            return out;
        }

        using clock = std::chrono::high_resolution_clock;
        using namespace std::chrono;
        const auto t_end = clock::now() + milliseconds(duration_ms);
        const auto period = duration<double>(1.0 / std::max(1, poll_hz));
        out.reserve((size_t)(duration_ms * (std::max(1,poll_hz)) / 1000 + 8));

        while(clock::now() < t_end){
            Window root = DefaultRootWindow(dpy);
            Window ret_root, ret_child;
            int root_x=0, root_y=0, win_x=0, win_y=0; unsigned int mask=0;
            XQueryPointer(dpy, root, &ret_root, &ret_child,
                            &root_x, &root_y, &win_x, &win_y, &mask);

            uint64_t tns = (uint64_t)duration_cast<nanoseconds>(clock::now().time_since_epoch()).count();
            out.push_back({root_x, root_y, tns});

            std::this_thread::sleep_for(duration_cast<nanoseconds>(period));
        }
        XCloseDisplay(dpy);
    #endif
        return out;
    }
};

// Сборка пула энтропии из сэмплов (dx,dy,dt -> mix64lab)
static uint64_t entropy_pool_from_samples(const std::vector<CursorSamplerX11::Sample>& s){
    if(s.empty()){
        auto now_ns = (uint64_t)std::chrono::high_resolution_clock::now()
                            .time_since_epoch().count();
        return mix64lab(now_ns); // fallback
    }
    uint64_t pool = 0;
    for(size_t i=1;i<s.size();++i){
        int dx = s[i].x - s[i-1].x;
        int dy = s[i].y - s[i-1].y;
        uint64_t dt = s[i].t - s[i-1].t;
        uint64_t z  = ((uint64_t)(uint32_t)dx << 32)
                    ^ ((uint64_t)(uint32_t)dy << 16)
                    ^ dt;
        pool ^= mix64lab(z ^ (0x9E3779B97F4A7C15ull * (uint64_t)i));
    }
    pool = mix64lab(pool ^ (uint64_t)s.size() * 0xD1342543DE82EF95ull);
    if(pool == 0){
        auto now_ns = (uint64_t)std::chrono::high_resolution_clock::now()
                            .time_since_epoch().count();
        pool = mix64lab(now_ns);
    }
    return pool;
}
