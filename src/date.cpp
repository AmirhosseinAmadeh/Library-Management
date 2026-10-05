#include "lms/date.h"

#include <cstdio>
#include <ctime>
#include <stdexcept>

namespace lms {
namespace {

// Howard Hinnant's civil-calendar algorithms (public domain).
long daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468;
}

void civilFromDays(long z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += m <= 2;
}

bool parse(const std::string& iso, int& y, unsigned& m, unsigned& d) {
    int yy = 0, mm = 0, dd = 0;
    char tail = 0;
    if (std::sscanf(iso.c_str(), "%4d-%2d-%2d%c", &yy, &mm, &dd, &tail) != 3) return false;
    if (iso.size() != 10 || mm < 1 || mm > 12 || dd < 1 || dd > 31) return false;
    // Round-trip to reject impossible dates such as 2026-02-31.
    int ry;
    unsigned rm, rd;
    civilFromDays(daysFromCivil(yy, static_cast<unsigned>(mm), static_cast<unsigned>(dd)), ry, rm, rd);
    if (ry != yy || rm != static_cast<unsigned>(mm) || rd != static_cast<unsigned>(dd)) return false;
    y = yy;
    m = static_cast<unsigned>(mm);
    d = static_cast<unsigned>(dd);
    return true;
}

long toDays(const std::string& iso) {
    int y;
    unsigned m, d;
    if (!parse(iso, y, m, d)) throw std::invalid_argument("invalid date: " + iso);
    return daysFromCivil(y, m, d);
}

}  // namespace

std::string todayIso() {
    std::time_t now = std::time(nullptr);
    std::tm local = *std::localtime(&now);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &local);
    return buf;
}

bool isValidIso(const std::string& iso) {
    int y;
    unsigned m, d;
    return parse(iso, y, m, d);
}

std::string addDays(const std::string& iso, int days) {
    int y;
    unsigned m, d;
    civilFromDays(toDays(iso) + days, y, m, d);
    char buf[16];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02u", y, m, d);
    return buf;
}

int diffDays(const std::string& from, const std::string& to) {
    return static_cast<int>(toDays(to) - toDays(from));
}

}  // namespace lms
