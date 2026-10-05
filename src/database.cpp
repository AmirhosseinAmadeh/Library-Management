#include "lms/database.h"

namespace lms {

namespace {
std::string message(sqlite3* db, const std::string& prefix) {
    return prefix + ": " + sqlite3_errmsg(db);
}
}  // namespace

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db), stmt_(nullptr) {
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK)
        throw DbError(message(db_, "prepare failed"));
}

Statement::~Statement() { sqlite3_finalize(stmt_); }

Statement& Statement::bind(int index, const std::string& value) {
    if (sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT) != SQLITE_OK)
        throw DbError(message(db_, "bind failed"));
    return *this;
}

Statement& Statement::bind(int index, const char* value) { return bind(index, std::string(value)); }

Statement& Statement::bind(int index, long long value) {
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) throw DbError(message(db_, "bind failed"));
    return *this;
}

Statement& Statement::bind(int index, int value) { return bind(index, static_cast<long long>(value)); }

Statement& Statement::bindNull(int index) {
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) throw DbError(message(db_, "bind failed"));
    return *this;
}

bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DbError(message(db_, "step failed"));
}

void Statement::run() {
    while (step()) {
    }
}

std::string Statement::text(int column) const {
    const unsigned char* t = sqlite3_column_text(stmt_, column);
    return t ? reinterpret_cast<const char*>(t) : std::string();
}

long long Statement::integer(int column) const { return sqlite3_column_int64(stmt_, column); }

bool Statement::isNull(int column) const { return sqlite3_column_type(stmt_, column) == SQLITE_NULL; }

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "out of memory";
        sqlite3_close(db_);
        throw DbError("cannot open database '" + path + "': " + err);
    }
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() { sqlite3_close(db_); }

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw DbError("exec failed: " + msg);
    }
}

long long Database::lastInsertId() const { return sqlite3_last_insert_rowid(db_); }

}  // namespace lms
