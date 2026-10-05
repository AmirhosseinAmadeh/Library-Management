#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "lms/database.h"
#include "lms/models.h"

namespace lms {

// Raised for business-rule violations (unknown ISBN, no copies left, ...).
class LibraryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Library {
public:
    static constexpr int kLoanDays = 14;
    static constexpr int kMaxActiveLoans = 5;
    static constexpr int kFinePerDayCents = 50;

    using Clock = std::function<std::string()>;  // returns today's ISO date

    explicit Library(const std::string& dbPath, Clock clock = nullptr);

    // Replace the clock (used by tests and by the demo data seeder).
    void setClock(Clock clock);
    std::string today() const;

    // Catalog
    long long addBook(const std::string& title, const std::string& author,
                      const std::string& isbn, const std::string& genre, int copies = 1);
    void removeBook(long long bookId);
    std::vector<Book> listBooks() const;
    std::vector<Book> searchBooks(const std::string& query) const;

    // Members
    long long addMember(const std::string& name, const std::string& email);
    std::vector<Member> listMembers() const;

    // Circulation
    Loan borrow(long long memberId, long long bookId);
    Loan giveBack(long long loanId);
    std::vector<Loan> activeLoans() const;
    std::vector<Loan> overdueLoans() const;
    std::vector<Loan> loanHistory(long long memberId) const;

    Stats stats() const;

    // ISBN-10 / ISBN-13 helpers
    static std::string normalizeIsbn(const std::string& raw);
    static bool isValidIsbn(const std::string& raw);

private:
    std::vector<Loan> queryLoans(const std::string& where, const std::string& arg = "") const;
    Loan getLoan(long long loanId) const;
    void decorate(Loan& loan) const;

    std::unique_ptr<Database> db_;
    Clock clock_;
};

}  // namespace lms
