#include "lms/html_export.h"

#include <cstdio>
#include <sstream>

namespace lms {
namespace {

std::string esc(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

std::string money(long long cents) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%lld.%02lld", cents / 100, cents % 100);
    return buf;
}

void card(std::ostringstream& o, const std::string& label, const std::string& value, const char* cls = "") {
    o << "<div class=\"card " << cls << "\"><div class=\"v\">" << esc(value) << "</div><div class=\"l\">"
      << esc(label) << "</div></div>\n";
}

}  // namespace

std::string renderDashboard(const Library& lib) {
    const Stats st = lib.stats();
    std::ostringstream o;
    o << R"(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LibraryOS Dashboard</title>
<style>
:root{--bg:#f4f6fb;--panel:#fff;--ink:#1c2333;--mute:#6b7488;--line:#e3e7f0;--brand:#2f5bea;--warn:#d92d20;--ok:#12805c}
@media(prefers-color-scheme:dark){:root{--bg:#10141f;--panel:#181e2d;--ink:#e8ebf4;--mute:#95a0b8;--line:#262e42;--brand:#6b8cff;--warn:#ff6b60;--ok:#3ccf9a}}
*{box-sizing:border-box}body{margin:0;font:14px/1.5 -apple-system,Segoe UI,Roboto,Arial,sans-serif;background:var(--bg);color:var(--ink)}
header{padding:28px 32px 8px}h1{margin:0;font-size:24px}header p{margin:4px 0 0;color:var(--mute)}
main{padding:16px 32px 40px;max-width:1180px;margin:0 auto}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px;margin:12px 0 24px}
.card{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:14px 16px}
.card .v{font-size:26px;font-weight:700}.card .l{color:var(--mute);font-size:12px;text-transform:uppercase;letter-spacing:.04em}
.card.warn .v{color:var(--warn)}.card.ok .v{color:var(--ok)}
section{background:var(--panel);border:1px solid var(--line);border-radius:12px;margin-bottom:20px;overflow:hidden}
section h2{margin:0;padding:14px 18px;font-size:15px;border-bottom:1px solid var(--line)}
table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:9px 18px;border-bottom:1px solid var(--line)}
th{font-size:11px;text-transform:uppercase;letter-spacing:.05em;color:var(--mute)}tr:last-child td{border-bottom:0}
.pill{display:inline-block;padding:1px 9px;border-radius:99px;font-size:12px;font-weight:600}
.pill.ok{background:rgba(18,128,92,.14);color:var(--ok)}.pill.no{background:rgba(217,45,32,.14);color:var(--warn)}
.empty{padding:18px;color:var(--mute)}.late{color:var(--warn);font-weight:600}
@media(max-width:640px){header,main{padding-left:16px;padding-right:16px}th,td{padding:8px 10px}}
</style></head><body>
<header><h1>LibraryOS</h1><p>Library status as of )"
      << esc(lib.today()) << R"(</p></header><main>
<div class="cards">
)";
    card(o, "Titles", std::to_string(st.titles));
    card(o, "Copies", std::to_string(st.copies));
    card(o, "Available", std::to_string(st.available), "ok");
    card(o, "Members", std::to_string(st.members));
    card(o, "On loan", std::to_string(st.activeLoans));
    card(o, "Overdue", std::to_string(st.overdueLoans), st.overdueLoans ? "warn" : "");
    card(o, "Fines accrued", money(st.outstandingFineCents), st.outstandingFineCents ? "warn" : "");
    o << "</div>\n";

    // Overdue
    o << "<section><h2>Overdue loans</h2>";
    const auto overdue = lib.overdueLoans();
    if (overdue.empty()) {
        o << "<div class=\"empty\">Nothing overdue.</div>";
    } else {
        o << "<table><tr><th>Book</th><th>Member</th><th>Due</th><th>Days late</th><th>Fine</th></tr>";
        for (const Loan& l : overdue)
            o << "<tr><td>" << esc(l.bookTitle) << "</td><td>" << esc(l.memberName) << "</td><td>" << l.due
              << "</td><td class=\"late\">" << l.daysOverdue << "</td><td>" << money(l.fineCents) << "</td></tr>";
        o << "</table>";
    }
    o << "</section>\n";

    // Active loans
    o << "<section><h2>Active loans</h2>";
    const auto active = lib.activeLoans();
    if (active.empty()) {
        o << "<div class=\"empty\">No books are out.</div>";
    } else {
        o << "<table><tr><th>#</th><th>Book</th><th>Member</th><th>Borrowed</th><th>Due</th></tr>";
        for (const Loan& l : active)
            o << "<tr><td>" << l.id << "</td><td>" << esc(l.bookTitle) << "</td><td>" << esc(l.memberName)
              << "</td><td>" << l.borrowed << "</td><td" << (l.daysOverdue ? " class=\"late\"" : "") << ">" << l.due
              << "</td></tr>";
        o << "</table>";
    }
    o << "</section>\n";

    // Catalog
    o << "<section><h2>Catalog</h2><table><tr><th>Title</th><th>Author</th><th>Genre</th><th>ISBN</th>"
         "<th>Availability</th></tr>";
    for (const Book& b : lib.listBooks())
        o << "<tr><td>" << esc(b.title) << "</td><td>" << esc(b.author) << "</td><td>" << esc(b.genre) << "</td><td>"
          << esc(b.isbn) << "</td><td><span class=\"pill " << (b.available ? "ok" : "no") << "\">" << b.available
          << " / " << b.copies << "</span></td></tr>";
    o << "</table></section>\n</main></body></html>\n";
    return o.str();
}

}  // namespace lms
