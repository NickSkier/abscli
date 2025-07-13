#include <stdexcept>
#include <string>
#include <sqlite3.h>
#include "statement.hpp"

abscli::db::Statement::Statement(sqlite3* db, const std::string& sql) {
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &m_stmt, nullptr) != SQLITE_OK)
    throw std::runtime_error("Failed to prepare statement");
}
abscli::db::Statement::~Statement() { sqlite3_finalize(m_stmt); }

auto abscli::db::Statement::get() const -> sqlite3_stmt* { return m_stmt; }

auto abscli::db::Statement::step(sqlite3* db) -> bool {
  int responseCode = sqlite3_step(m_stmt);
  if (responseCode == SQLITE_ROW) return true;
  if (responseCode == SQLITE_DONE) return false;
  throw std::runtime_error(sqlite3_errmsg(db));
}

auto abscli::db::Statement::get_column_text(int col) -> std::string {
  const unsigned char* text = sqlite3_column_text(m_stmt, col);
  return bool(text) ? reinterpret_cast<const char*>(text) : "";
}

auto abscli::db::Statement::get_column_int(int col) -> int {
  return sqlite3_column_int(m_stmt, col);
}

void abscli::db::Statement::reset() { sqlite3_reset(m_stmt); }
