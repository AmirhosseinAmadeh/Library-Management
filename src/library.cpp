#include "lms/library.h"

#include <algorithm>
#include <cctype>

#include "lms/date.h"

namespace lms {

const std::vector<std::string>& genres() {
    static const std::vector<std::string> kGenres = {
        "Fiction",   "Non-Fiction", "Poetry",  "Drama",      "Horror",   "Fantasy",
        "Thriller",  "Comedy",      "Adventure", "Young Adult", "Children", "Romance",
        "Technology", "Science",    "History"};
    return kGenres;
}

namespace {

const char* kSchema = R"sql(
CREATE TABLE IF NOT EXISTS books (
    id     INTEGER PRIMARY KEY AUTOINCREMENT,
    title  TEXT NOT NULL,
    author TEXT NOT NULL,
    isbn   TEXT NOT NULL UNIQUE,
    genre  TEXT NOT NULL,
    copies INTEGER NOT NULL CHECK (copies > 0)
);
CREATE TABLE IF NOT EXISTS members (
    id     INTEGER PRIMARY KEY AUTOINCREMENT,
    name   TEXT NOT NULL,
    email  TEXT NOT NULL UNIQUE,
    joined TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS loans (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    book_id    INTEGER NOT NULL REFERENCES books(id),
    member_id  INTEGER NOT NULL REFERENCES members(id),
    borrowed   TEXT NOT NULL,
    due        TEXT NOT NULL,
    returned   TEXT,
    fine_cents INTEGER NOT NULL DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_loans_open ON loans(returned, due);
)sql";

const char* kBookSelect =
    "SELECT b.id, b.title, b.author, b.isbn, b.genre, b.copies, "
    "b.copies - (SELECT COUNT(*) FROM loans l WHERE l.book_id = b.id AND l.returned IS NULL) "
    "FROM books b ";

const char* kLoanSelect =
    "SELECT l.id, l.book_id, l.member_id, b.title, m.name, l.borrowed, l.due, l.returned, l.fine_cents "
    "FROM loans l JOIN books b ON b.id = l.book_id JOIN members m ON m.id = l.member_id ";

Book readBook(const Statement& s) {
    Book b;
    b.id = s.integer(0);
    b.title = s.text(1);
    b.author = s.text(2);
    b.isbn = s.text(3);
    b.genre = s.text(4);
    b.copies = static_cast<int>(s.integer(5));
    b.available = static_cast<int>(s.integer(6));
    return b;
}

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string escapeLike(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '%' || c == '_' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

}  // namespace

Library::Library(const std::string& dbPath, Clock clock)
    : db_(new Database(dbPath)), clock_(clock ? std::move(clock) : Clock(todayIso)) {
    db_->exec(kSchema);
}

void Library::setClock(Clock clock) { clock_ = clock ? std::move(clock) : Clock(todayIso); }

std::string Library::today() const { return clock_(); }

// ---------------------------------------------------------------- ISBN

std::string Library::normalizeIsbn(const std::string& raw) {
    std::string out;
    for (char c : raw) {
        if (c == '-' || c == ' ') continue;
        out += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return out;
}

bool Library::isValidIsbn(const std::string& raw) {
    const std::string s = normalizeIsbn(raw);
    if (s.size() == 13) {
        int sum = 0;
        for (size_t i = 0; i < 13; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
            sum += (s[i] - '0') * (i % 2 == 0 ? 1 : 3);
        }
        return sum % 10 == 0;
    }
    if (s.size() == 10) {
        int sum = 0;
        for (size_t i = 0; i < 10; ++i) {
            int v;
            if (s[i] == 'X' && i == 9) v = 10;
            else if (std::isdigit(static_cast<unsigned char>(s[i]))) v = s[i] - '0';
            else return false;
            sum += v * static_cast<int>(10 - i);
        }
        return sum % 11 == 0;
    }
    return false;
}

// ---------------------------------------------------------------- catalog

long long Library::addBook(const std::string& title, const std::string& author, const std::string& isbn,
                           const std::string& genre, int copies) {
    const std::string t = trim(title), a = trim(author);
    if (t.empty() || a.empty()) throw LibraryError("title and author are required");
    if (!isValidIsbn(isbn)) throw LibraryError("invalid ISBN: " + isbn);
    if (copies < 1) throw LibraryError("copies must be at least 1");
    if (std::find(genres().begin(), genres().end(), genre) == genres().end())
        throw LibraryError("unknown genre: " + genre);

    const std::string norm = normalizeIsbn(isbn);
    Statement dup(db_->handle(), "SELECT 1 FROM books WHERE isbn = ?");
    dup.bind(1, norm);
    if (dup.step()) throw LibraryError("a book with ISBN " + norm + " already exists");

    Statement ins(db_->handle(), "INSERT INTO books (title, author, isbn, genre, copies) VALUES (?, ?, ?, ?, ?)");
    ins.bind(1, t).bind(2, a).bind(3, norm).bind(4, genre).bind(5, copies).run();
    return db_->lastInsertId();
}

void Library::removeBook(long long bookId) {
    Statement open(db_->handle(), "SELECT COUNT(*) FROM loans WHERE book_id = ? AND returned IS NULL");
    open.bind(1, bookId);
    open.step();
    if (open.integer(0) > 0) throw LibraryError("book is currently on loan and cannot be removed");

    Statement hist(db_->handle(), "SELECT COUNT(*) FROM loans WHERE book_id = ?");
    hist.bind(1, bookId);
    hist.step();
    if (hist.integer(0) > 0) throw LibraryError("book has loan history and cannot be removed");

    Statement del(db_->handle(), "DELETE FROM books WHERE id = ?");
    del.bind(1, bookId).run();
    if (sqlite3_changes(db_->handle()) == 0) throw LibraryError("no such book: " + std::to_string(bookId));
}

std::vector<Book> Library::listBooks() const {
    std::vector<Book> out;
    Statement s(db_->handle(), std::string(kBookSelect) + "ORDER BY b.title");
    while (s.step()) out.push_back(readBook(s));
    return out;
}

std::vector<Book> Library::searchBooks(const std::string& query) const {
    std::vector<Book> out;
    const std::string like = "%" + escapeLike(trim(query)) + "%";
    Statement s(db_->handle(), std::string(kBookSelect) +
                                   "WHERE b.title LIKE ?1 ESCAPE '\\' OR b.author LIKE ?1 ESCAPE '\\' "
                                   "OR b.genre LIKE ?1 ESCAPE '\\' OR b.isbn LIKE ?1 ESCAPE '\\' "
                                   "ORDER BY b.title");
    s.bind(1, like);
    while (s.step()) out.push_back(readBook(s));
    return out;
}

// ---------------------------------------------------------------- members

long long Library::addMember(const std::string& name, const std::string& email) {
    const std::string n = trim(name), e = trim(email);
    if (n.empty()) throw LibraryError("member name is required");
    const auto at = e.find('@');
    if (at == std::string::npos || at == 0 || at + 1 >= e.size()) throw LibraryError("invalid email: " + email);

    Statement dup(db_->handle(), "SELECT 1 FROM members WHERE email = ?");
    dup.bind(1, e);
    if (dup.step()) throw LibraryError("a member with email " + e + " already exists");

    Statement ins(db_->handle(), "INSERT INTO members (name, email, joined) VALUES (?, ?, ?)");
    ins.bind(1, n).bind(2, e).bind(3, today()).run();
    return db_->lastInsertId();
}

std::vector<Member> Library::listMembers() const {
    std::vector<Member> out;
    Statement s(db_->handle(),
                "SELECT m.id, m.name, m.email, m.joined, "
                "(SELECT COUNT(*) FROM loans l WHERE l.member_id = m.id AND l.returned IS NULL) "
                "FROM members m ORDER BY m.name");
    while (s.step()) {
        Member m;
        m.id = s.integer(0);
        m.name = s.text(1);
        m.email = s.text(2);
        m.joined = s.text(3);
        m.activeLoans = static_cast<int>(s.integer(4));
        out.push_back(m);
    }
    return out;
}

// ---------------------------------------------------------------- circulation

void Library::decorate(Loan& loan) const {
    // Closed loans keep their stored fine; open loans accrue it day by day.
    if (!loan.returned.empty()) return;
    const int late = diffDays(loan.due, today());
    loan.daysOverdue = late > 0 ? late : 0;
    loan.fineCents = loan.daysOverdue * kFinePerDayCents;
}

std::vector<Loan> Library::queryLoans(const std::string& where, const std::string& arg) const {
    std::vector<Loan> out;
    Statement s(db_->handle(), std::string(kLoanSelect) + where);
    if (!arg.empty()) s.bind(1, arg);
    while (s.step()) {
        Loan l;
        l.id = s.integer(0);
        l.bookId = s.integer(1);
        l.memberId = s.integer(2);
        l.bookTitle = s.text(3);
        l.memberName = s.text(4);
        l.borrowed = s.text(5);
        l.due = s.text(6);
        l.returned = s.text(7);
        l.fineCents = static_cast<int>(s.integer(8));
        decorate(l);
        out.push_back(l);
    }
    return out;
}

Loan Library::getLoan(long long loanId) const {
    auto rows = queryLoans("WHERE l.id = " + std::to_string(loanId));
    if (rows.empty()) throw LibraryError("no such loan: " + std::to_string(loanId));
    return rows.front();
}

Loan Library::borrow(long long memberId, long long bookId) {
    Statement mem(db_->handle(), "SELECT 1 FROM members WHERE id = ?");
    mem.bind(1, memberId);
    if (!mem.step()) throw LibraryError("no such member: " + std::to_string(memberId));

    Statement bk(db_->handle(), std::string(kBookSelect) + "WHERE b.id = ?");
    bk.bind(1, bookId);
    if (!bk.step()) throw LibraryError("no such book: " + std::to_string(bookId));
    const Book book = readBook(bk);
    if (book.available < 1) throw LibraryError("no copies of '" + book.title + "' are available");

    const std::string now = today();
    Statement counts(db_->handle(),
                     "SELECT COUNT(*), COALESCE(SUM(due < ?), 0) FROM loans WHERE member_id = ? AND returned IS NULL");
    counts.bind(1, now).bind(2, memberId);
    counts.step();
    if (counts.integer(0) >= kMaxActiveLoans)
        throw LibraryError("member already has " + std::to_string(kMaxActiveLoans) + " books on loan");
    if (counts.integer(1) > 0) throw LibraryError("member has overdue books and must return them first");

    Statement ins(db_->handle(), "INSERT INTO loans (book_id, member_id, borrowed, due) VALUES (?, ?, ?, ?)");
    ins.bind(1, bookId).bind(2, memberId).bind(3, now).bind(4, addDays(now, kLoanDays)).run();
    return getLoan(db_->lastInsertId());
}

Loan Library::giveBack(long long loanId) {
    Loan loan = getLoan(loanId);
    if (!loan.returned.empty()) throw LibraryError("loan " + std::to_string(loanId) + " was already returned");

    Statement upd(db_->handle(), "UPDATE loans SET returned = ?, fine_cents = ? WHERE id = ?");
    upd.bind(1, today()).bind(2, loan.fineCents).bind(3, loanId).run();
    return getLoan(loanId);
}

std::vector<Loan> Library::activeLoans() const { return queryLoans("WHERE l.returned IS NULL ORDER BY l.due"); }

std::vector<Loan> Library::overdueLoans() const {
    std::vector<Loan> out;
    for (const Loan& l : activeLoans())
        if (l.daysOverdue > 0) out.push_back(l);
    return out;
}

std::vector<Loan> Library::loanHistory(long long memberId) const {
    return queryLoans("WHERE l.member_id = " + std::to_string(memberId) + " ORDER BY l.borrowed DESC, l.id DESC");
}

Stats Library::stats() const {
    Stats st;
    Statement s(db_->handle(), "SELECT COUNT(*), COALESCE(SUM(copies), 0) FROM books");
    s.step();
    st.titles = static_cast<int>(s.integer(0));
    st.copies = static_cast<int>(s.integer(1));

    Statement m(db_->handle(), "SELECT COUNT(*) FROM members");
    m.step();
    st.members = static_cast<int>(m.integer(0));

    Statement t(db_->handle(), "SELECT COUNT(*) FROM loans");
    t.step();
    st.totalLoans = static_cast<int>(t.integer(0));

    for (const Loan& l : activeLoans()) {
        ++st.activeLoans;
        if (l.daysOverdue > 0) ++st.overdueLoans;
        st.outstandingFineCents += l.fineCents;
    }
    st.available = st.copies - st.activeLoans;
    return st;
}

}  // namespace lms
