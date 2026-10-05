#include <iostream>
#include <string>

#include "lms/date.h"
#include "lms/library.h"

using namespace lms;

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++checks;                                                                \
        if (!(cond)) {                                                           \
            ++failures;                                                          \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
        }                                                                        \
    } while (0)

#define CHECK_THROWS(expr)                         \
    do {                                           \
        ++checks;                                  \
        bool threw = false;                        \
        try { expr; } catch (const LibraryError&) { threw = true; } \
        if (!threw) {                              \
            ++failures;                            \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  expected LibraryError from " #expr "\n"; \
        }                                          \
    } while (0)

static const char* kIsbnA = "978-0201889543";
static const char* kIsbnB = "978-1491903995";

static void testDates() {
    CHECK(addDays("2026-02-27", 2) == "2026-03-01");
    CHECK(addDays("2024-02-28", 2) == "2024-03-01");  // leap year
    CHECK(addDays("2026-01-01", -1) == "2025-12-31");
    CHECK(diffDays("2026-01-01", "2026-01-15") == 14);
    CHECK(diffDays("2026-01-15", "2026-01-01") == -14);
    CHECK(isValidIso("2026-02-28"));
    CHECK(!isValidIso("2026-02-31"));
    CHECK(!isValidIso("26-02-01"));
}

static void testIsbn() {
    CHECK(Library::isValidIsbn(kIsbnA));
    CHECK(Library::isValidIsbn("0-306-40615-2"));  // ISBN-10
    CHECK(Library::isValidIsbn("080442957X"));     // ISBN-10 with X check digit
    CHECK(!Library::isValidIsbn("978-0201889544"));
    CHECK(!Library::isValidIsbn("12345"));
    CHECK(Library::normalizeIsbn("978-0201 889543") == "9780201889543");
}

static void testCatalog() {
    Library lib(":memory:");
    long long id = lib.addBook("  The C++ Programming Language ", "Bjarne Stroustrup", kIsbnA, "Technology", 2);
    CHECK(id == 1);
    CHECK_THROWS(lib.addBook("Dup", "X", kIsbnA, "Technology"));
    CHECK_THROWS(lib.addBook("Bad ISBN", "X", "123", "Technology"));
    CHECK_THROWS(lib.addBook("Bad genre", "X", kIsbnB, "Cooking"));
    CHECK_THROWS(lib.addBook("", "X", kIsbnB, "Technology"));
    CHECK_THROWS(lib.addBook("Zero", "X", kIsbnB, "Technology", 0));

    lib.addBook("Effective Modern C++", "Scott Meyers", kIsbnB, "Technology");
    CHECK(lib.listBooks().size() == 2);
    CHECK(lib.searchBooks("meyers").size() == 1);
    CHECK(lib.searchBooks("c++").size() == 2);
    CHECK(lib.searchBooks("100%").empty());  // LIKE wildcards are escaped
    CHECK(lib.searchBooks("nothing here").empty());
    CHECK(lib.listBooks().front().title == "Effective Modern C++");  // sorted, and trimmed on insert
}

static void testCirculation() {
    Library lib(":memory:");
    std::string now = "2026-03-01";
    lib.setClock([&] { return now; });

    long long book = lib.addBook("Dune", "Frank Herbert", "978-0441013593", "Fiction", 1);
    long long ali = lib.addMember("Ali", "ali@example.com");
    long long sara = lib.addMember("Sara", "sara@example.com");
    CHECK_THROWS(lib.addMember("Ali again", "ali@example.com"));
    CHECK_THROWS(lib.addMember("No Mail", "nomail"));

    Loan loan = lib.borrow(ali, book);
    CHECK(loan.borrowed == "2026-03-01");
    CHECK(loan.due == "2026-03-15");
    CHECK(lib.listBooks().front().available == 0);
    CHECK_THROWS(lib.borrow(sara, book));  // only copy is out
    CHECK_THROWS(lib.removeBook(book));    // cannot delete a book on loan
    CHECK_THROWS(lib.borrow(999, book));
    CHECK_THROWS(lib.borrow(ali, 999));

    now = "2026-03-15";  // due today: not late yet
    CHECK(lib.overdueLoans().empty());
    now = "2026-03-20";  // 5 days late
    auto overdue = lib.overdueLoans();
    CHECK(overdue.size() == 1);
    CHECK(overdue[0].daysOverdue == 5);
    CHECK(overdue[0].fineCents == 5 * Library::kFinePerDayCents);
    CHECK(lib.stats().overdueLoans == 1);
    CHECK(lib.stats().outstandingFineCents == 250);

    Loan back = lib.giveBack(loan.id);
    CHECK(back.returned == "2026-03-20");
    CHECK(back.fineCents == 250);
    CHECK_THROWS(lib.giveBack(loan.id));  // already returned
    CHECK(lib.listBooks().front().available == 1);
    CHECK(lib.stats().outstandingFineCents == 0);
    CHECK(lib.loanHistory(ali).size() == 1);
    CHECK_THROWS(lib.removeBook(book));  // history keeps the book
}

static void testLimits() {
    Library lib(":memory:");
    lib.setClock([] { return std::string("2026-03-01"); });
    const char* isbns[] = {"978-0201889543", "978-1491903995", "978-0132350884",
                           "978-0262033848", "978-0201633610", "978-0441013593"};
    long long member = lib.addMember("Heavy Reader", "reader@example.com");
    for (int i = 0; i < 6; ++i) lib.addBook("Book " + std::to_string(i), "Author", isbns[i], "Fiction");
    for (int i = 1; i <= Library::kMaxActiveLoans; ++i) lib.borrow(member, i);
    CHECK_THROWS(lib.borrow(member, 6));  // loan limit reached

    // A member with an overdue book cannot borrow more.
    Library lib2(":memory:");
    std::string now = "2026-03-01";
    lib2.setClock([&] { return now; });
    long long m = lib2.addMember("Late", "late@example.com");
    lib2.addBook("A", "X", isbns[0], "Fiction");
    lib2.addBook("B", "X", isbns[1], "Fiction");
    lib2.borrow(m, 1);
    now = "2026-04-01";
    CHECK_THROWS(lib2.borrow(m, 2));
}

int main() {
    testDates();
    testIsbn();
    testCatalog();
    testCirculation();
    testLimits();
    std::cout << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
