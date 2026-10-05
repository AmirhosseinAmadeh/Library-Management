#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lms/date.h"
#include "lms/html_export.h"
#include "lms/library.h"

using namespace lms;

namespace {

const char* kUsage = R"(LibraryOS - library management CLI

usage: lms [--db FILE] <command> [args]

  seed                                      load demo books, members and loans
  book add <title> <author> <isbn> <genre> [copies]
  book list
  book search <query>
  book remove <book_id>
  member add <name> <email>
  member list
  borrow <member_id> <book_id>              14-day loan, max 5 per member
  return <loan_id>                          fine: 0.50 per overdue day
  loans                                     books currently out
  overdue                                   overdue loans with accrued fines
  history <member_id>
  stats
  export-html <file>                        write the dashboard as a static page
  genres

default database: library.db)";

using Row = std::vector<std::string>;

std::string cut(const std::string& s, size_t n) { return s.size() <= n ? s : s.substr(0, n - 3) + "..."; }

std::string money(long long cents) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%lld.%02lld", cents / 100, cents % 100);
    return buf;
}

void printTable(const Row& header, const std::vector<Row>& rows) {
    if (rows.empty()) {
        std::cout << "(nothing to show)\n";
        return;
    }
    std::vector<size_t> w(header.size());
    for (size_t i = 0; i < header.size(); ++i) w[i] = header[i].size();
    for (const Row& r : rows)
        for (size_t i = 0; i < r.size(); ++i) w[i] = std::max(w[i], r[i].size());
    auto line = [&](const Row& r) {
        for (size_t i = 0; i < r.size(); ++i) {
            std::cout << r[i] << std::string(w[i] - r[i].size() + 2, ' ');
        }
        std::cout << "\n";
    };
    line(header);
    size_t total = 0;
    for (size_t x : w) total += x + 2;
    std::cout << std::string(total, '-') << "\n";
    for (const Row& r : rows) line(r);
}

void printBooks(const std::vector<Book>& books) {
    std::vector<Row> rows;
    for (const Book& b : books)
        rows.push_back({std::to_string(b.id), cut(b.title, 38), cut(b.author, 24), b.genre, b.isbn,
                        std::to_string(b.available) + "/" + std::to_string(b.copies)});
    printTable({"ID", "TITLE", "AUTHOR", "GENRE", "ISBN", "AVAIL"}, rows);
}

void printLoans(const std::vector<Loan>& loans) {
    std::vector<Row> rows;
    for (const Loan& l : loans)
        rows.push_back({std::to_string(l.id), cut(l.bookTitle, 34), cut(l.memberName, 18), l.borrowed, l.due,
                        l.returned.empty() ? (l.daysOverdue ? std::to_string(l.daysOverdue) + " days late" : "on loan")
                                           : "returned " + l.returned,
                        money(l.fineCents)});
    printTable({"LOAN", "BOOK", "MEMBER", "BORROWED", "DUE", "STATE", "FINE"}, rows);
}

void seed(Library& lib) {
    const std::string now = todayIso();
    struct B { const char* t; const char* a; const char* i; const char* g; int c; };
    const B books[] = {
        {"The C++ Programming Language", "Bjarne Stroustrup", "978-0321563842", "Technology", 2},
        {"Effective Modern C++", "Scott Meyers", "978-1491903995", "Technology", 2},
        {"Clean Code", "Robert C. Martin", "978-0132350884", "Technology", 1},
        {"Introduction to Algorithms", "Thomas H. Cormen", "978-0262033848", "Technology", 3},
        {"Design Patterns", "Erich Gamma", "978-0201633610", "Technology", 1},
        {"Dune", "Frank Herbert", "978-0441013593", "Fiction", 2},
        {"Neuromancer", "William Gibson", "978-0441569595", "Fiction", 1},
        {"1984", "George Orwell", "978-0451524935", "Fiction", 3},
        {"The Hobbit", "J.R.R. Tolkien", "978-0547928227", "Fantasy", 2},
        {"Pride and Prejudice", "Jane Austen", "978-0141439518", "Romance", 1},
        {"Sapiens", "Yuval Noah Harari", "978-0062316097", "History", 2},
        {"The Shining", "Stephen King", "978-0307743657", "Horror", 1},
    };
    for (const B& b : books) lib.addBook(b.t, b.a, b.i, b.g, b.c);

    const long long ali = lib.addMember("Ali Rezaei", "ali@example.com");
    const long long sara = lib.addMember("Sara Ahmadi", "sara@example.com");
    const long long omid = lib.addMember("Omid Karimi", "omid@example.com");
    const long long neda = lib.addMember("Neda Hosseini", "neda@example.com");

    // Back-dated loans so the demo shows overdue items and fines.
    lib.setClock([&] { return addDays(now, -25); });
    lib.borrow(ali, 1);    // C++ Programming Language -> 11 days late
    lib.borrow(sara, 6);   // Dune -> 11 days late
    lib.setClock([&] { return addDays(now, -16); });
    lib.borrow(omid, 3);   // Clean Code -> 2 days late
    lib.setClock([&] { return addDays(now, -5); });
    lib.borrow(neda, 8);   // 1984
    lib.borrow(neda, 9);   // The Hobbit
    Loan done = lib.borrow(neda, 12);
    lib.setClock([&] { return addDays(now, -2); });
    lib.giveBack(done.id);
    lib.setClock(nullptr);
    lib.borrow(neda, 4);
    std::cout << "Demo data loaded.\n";
}

int run(int argc, char** argv) {
    std::string dbPath = "library.db";
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--db" && i + 1 < argc) dbPath = argv[++i];
        else args.push_back(a);
    }
    if (args.empty() || args[0] == "help" || args[0] == "--help") {
        std::cout << kUsage << "\n";
        return args.empty() ? 1 : 0;
    }

    const std::string cmd = args[0];
    auto need = [&](size_t n) {
        if (args.size() < n + 1) throw LibraryError("missing arguments - run 'lms help'");
    };
    auto num = [](const std::string& s) -> long long {
        try {
            size_t pos = 0;
            long long v = std::stoll(s, &pos);
            if (pos != s.size()) throw std::invalid_argument(s);
            return v;
        } catch (const std::exception&) {
            throw LibraryError("expected a number, got '" + s + "'");
        }
    };

    Library lib(dbPath);

    if (cmd == "seed") {
        seed(lib);
    } else if (cmd == "genres") {
        for (const auto& g : genres()) std::cout << g << "\n";
    } else if (cmd == "book") {
        need(1);
        const std::string sub = args[1];
        if (sub == "add") {
            need(5);
            int copies = args.size() > 6 ? static_cast<int>(num(args[6])) : 1;
            long long id = lib.addBook(args[2], args[3], args[4], args[5], copies);
            std::cout << "Added book #" << id << "\n";
        } else if (sub == "list") {
            printBooks(lib.listBooks());
        } else if (sub == "search") {
            need(2);
            printBooks(lib.searchBooks(args[2]));
        } else if (sub == "remove") {
            need(2);
            lib.removeBook(num(args[2]));
            std::cout << "Removed.\n";
        } else {
            throw LibraryError("unknown book command: " + sub);
        }
    } else if (cmd == "member") {
        need(1);
        const std::string sub = args[1];
        if (sub == "add") {
            need(3);
            std::cout << "Registered member #" << lib.addMember(args[2], args[3]) << "\n";
        } else if (sub == "list") {
            std::vector<Row> rows;
            for (const Member& m : lib.listMembers())
                rows.push_back({std::to_string(m.id), m.name, m.email, m.joined, std::to_string(m.activeLoans)});
            printTable({"ID", "NAME", "EMAIL", "JOINED", "LOANS"}, rows);
        } else {
            throw LibraryError("unknown member command: " + sub);
        }
    } else if (cmd == "borrow") {
        need(2);
        Loan l = lib.borrow(num(args[1]), num(args[2]));
        std::cout << "Loan #" << l.id << ": '" << l.bookTitle << "' -> " << l.memberName << ", due " << l.due << "\n";
    } else if (cmd == "return") {
        need(1);
        Loan l = lib.giveBack(num(args[1]));
        std::cout << "Returned '" << l.bookTitle << "'.";
        if (l.fineCents > 0) std::cout << " Fine due: " << money(l.fineCents);
        std::cout << "\n";
    } else if (cmd == "loans") {
        printLoans(lib.activeLoans());
    } else if (cmd == "overdue") {
        printLoans(lib.overdueLoans());
    } else if (cmd == "history") {
        need(1);
        printLoans(lib.loanHistory(num(args[1])));
    } else if (cmd == "stats") {
        Stats s = lib.stats();
        std::cout << "Titles:         " << s.titles << "\nCopies:         " << s.copies
                  << "\nAvailable:      " << s.available << "\nMembers:        " << s.members
                  << "\nActive loans:   " << s.activeLoans << "\nOverdue loans:  " << s.overdueLoans
                  << "\nTotal loans:    " << s.totalLoans << "\nFines accrued:  " << money(s.outstandingFineCents)
                  << "\n";
    } else if (cmd == "export-html") {
        need(1);
        std::ofstream out(args[1], std::ios::binary);
        if (!out) throw LibraryError("cannot write " + args[1]);
        out << renderDashboard(lib);
        std::cout << "Wrote " << args[1] << "\n";
    } else {
        throw LibraryError("unknown command: " + cmd + " - run 'lms help'");
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const LibraryError& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
