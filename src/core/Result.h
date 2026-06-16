#pragma once

#include <QString>
#include <utility>
#include <variant>

namespace pacn {

// Minimal Result<T> — a portable stand-in that avoids relying on std::expected
// being present in every toolchain. Carries either a value or an error message.
template <typename T>
class Result {
public:
    static Result ok(T value) { return Result(std::move(value)); }
    static Result err(QString message) { return Result(Error{std::move(message)}); }

    bool hasValue() const { return std::holds_alternative<T>(data_); }
    explicit operator bool() const { return hasValue(); }

    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }
    T valueOr(T fallback) const { return hasValue() ? std::get<T>(data_) : fallback; }

    QString error() const {
        return hasValue() ? QString() : std::get<Error>(data_).message;
    }

private:
    struct Error { QString message; };
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error e) : data_(std::move(e)) {}
    std::variant<T, Error> data_;
};

}  // namespace pacn
