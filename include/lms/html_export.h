#pragma once
#include <string>

#include "lms/library.h"

namespace lms {

// Renders a self-contained HTML dashboard (stats, catalog, loans, overdue list).
std::string renderDashboard(const Library& library);

}  // namespace lms
