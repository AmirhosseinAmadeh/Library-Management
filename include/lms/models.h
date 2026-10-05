#pragma once
#include <string>
#include <vector>

namespace lms {

struct Book {
    long long id = 0;
    std::string title;
    std::string author;
    std::string isbn;   // normalized: digits only (plus a possible trailing 'X')
    std::string genre;
    int copies = 0;     // physical copies owned by the library
    int available = 0;  // copies - active loans
};

struct Member {
    long long id = 0;
    std::string name;
    std::string email;
    std::string joined;  // ISO date
    int activeLoans = 0;
};

struct Loan {
    long long id = 0;
    long long bookId = 0;
    long long memberId = 0;
    std::string bookTitle;
    std::string memberName;
    std::string borrowed;  // ISO date
    std::string due;       // ISO date
    std::string returned;  // empty while the book is still out
    int fineCents = 0;     // final fine once returned, accrued fine while overdue
    int daysOverdue = 0;
};

struct Stats {
    int titles = 0;
    int copies = 0;
    int available = 0;
    int members = 0;
    int activeLoans = 0;
    int overdueLoans = 0;
    int totalLoans = 0;
    long long outstandingFineCents = 0;
};

// Genres the catalog accepts.
const std::vector<std::string>& genres();

}  // namespace lms
