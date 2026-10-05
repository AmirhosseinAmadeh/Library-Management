#pragma once
#include <string>

namespace lms {

// Dates are handled as ISO-8601 strings ("YYYY-MM-DD") so they sort correctly in SQLite.
std::string todayIso();
bool isValidIso(const std::string& iso);
std::string addDays(const std::string& iso, int days);
// Number of days from `from` to `to` (negative when `to` is earlier).
int diffDays(const std::string& from, const std::string& to);

}  // namespace lms
