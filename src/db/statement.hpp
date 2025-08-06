#pragma once

#include <sqlite3.h>
#include <concepts>
#include <string>
#include <utility>
#include <stdexcept>

namespace abscli::db {
  class Statement {
  public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();

    auto get() const -> sqlite3_stmt*;
    auto step(sqlite3* db) -> bool;
    void reset();

    auto get_column_text(int col) -> std::string;
    auto get_column_int(int col) -> int;
    auto get_column_double(int col) -> double;

    template<typename T>
    requires std::integral<T>
    void bind(const int index, const T& value) {
      if (sqlite3_bind_int(m_stmt, index, value) != SQLITE_OK)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m Failed to bind integral value.");
    }
    template<typename T>
    requires std::floating_point<T>
    void bind(const int index, const T& value) {
      if (sqlite3_bind_double(m_stmt, index, value) != SQLITE_OK)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m Failed to bind floating value.");
    }
    template<typename T>
    void bind(const int index, T&& value) {
      if (sqlite3_bind_text(m_stmt, index, std::forward<T>(value).c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m Failed to bind text value.");
    }

  private:
    sqlite3_stmt* m_stmt;
  };
}
