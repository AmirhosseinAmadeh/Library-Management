#pragma once
#include <sqlite3.h>

#include <stdexcept>
#include <string>

namespace lms {

class DbError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// RAII wrapper around a prepared statement.
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement& bind(int index, const std::string& value);
    Statement& bind(int index, const char* value);
    Statement& bind(int index, long long value);
    Statement& bind(int index, int value);
    Statement& bindNull(int index);

    bool step();  // true while a row is available
    void run();   // execute a statement that returns no rows

    std::string text(int column) const;
    long long integer(int column) const;
    bool isNull(int column) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
};

// RAII wrapper around a SQLite connection (use ":memory:" for tests).
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() const { return db_; }
    void exec(const std::string& sql);
    long long lastInsertId() const;

private:
    sqlite3* db_ = nullptr;
};

}  // namespace lms
