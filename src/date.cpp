#include "date.h"
#include <chrono>
#include <cstdio>
#include <ctime>

static bool razobratDatu(const std::string& s, std::chrono::year_month_day& data) {
    if (s.size() != 10 || s[2] != '.' || s[5] != '.') return false;
    for (std::size_t i = 0; i < s.size(); i++) {
        if (i == 2 || i == 5) continue;
        if (s[i] < '0' || s[i] > '9') return false;
    }

    int d = (s[0] - '0') * 10 + s[1] - '0';
    int m = (s[3] - '0') * 10 + s[4] - '0';
    int g = (s[6] - '0') * 1000 + (s[7] - '0') * 100
          + (s[8] - '0') * 10 + s[9] - '0';

    data = std::chrono::year{g} / std::chrono::month{static_cast<unsigned>(m)}
                              / std::chrono::day{static_cast<unsigned>(d)};
    return data.ok();
}

bool dataHoroshaya(const std::string& s) {
    std::chrono::year_month_day data;
    return razobratDatu(s, data);
}

int skolkoDney(const std::string& nach, const std::string& kon) {
    std::chrono::year_month_day nachalo, konec;
    if (!razobratDatu(nach, nachalo) || !razobratDatu(kon, konec)) return -1;
    auto raznica = std::chrono::sys_days{konec} - std::chrono::sys_days{nachalo};
    if (raznica.count() < 0) return -1;
    return static_cast<int>(raznica.count()) + 1;
}

bool dataNePozdnee(const std::string& a, const std::string& b) {
    std::chrono::year_month_day da, db;
    if (!razobratDatu(a, da) || !razobratDatu(b, db)) return false;
    return std::chrono::sys_days{da} <= std::chrono::sys_days{db};
}

bool dataRanshe(const std::string& a, const std::string& b) {
    std::chrono::year_month_day da, db;
    if (!razobratDatu(a, da) || !razobratDatu(b, db)) return false;
    return std::chrono::sys_days{da} < std::chrono::sys_days{db};
}

std::string segodnya() {
    std::time_t vremya = std::time(nullptr);
    std::tm* tm = std::localtime(&vremya);
    if (tm == nullptr) return "";
    char bufer[11];
    std::snprintf(bufer, sizeof(bufer), "%02d.%02d.%04d",
                  tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
    return bufer;
}
