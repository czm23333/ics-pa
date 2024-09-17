#pragma once
#include <tuple>
#include <string_view>
#include <optional>
#include <charconv>
#include <string>
#include "expr.h"

template<typename Arg>
std::tuple<std::optional<Arg> > _parse_arg_ker(std::string_view &sv);

template<>
inline std::tuple<std::optional<int> > _parse_arg_ker<int>(std::string_view &sv) {
    int res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) {
        sv.remove_prefix(ptr - sv.data());
        return std::make_tuple(res);
    }
    return std::make_tuple(std::nullopt);
}

template<>
inline std::tuple<std::optional<double> > _parse_arg_ker<double>(std::string_view &sv) {
    double res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) {
        sv.remove_prefix(ptr - sv.data());
        return std::make_tuple(res);
    }
    return std::make_tuple(std::nullopt);
}

template<>
inline std::tuple<std::optional<std::string> > _parse_arg_ker<std::string>(std::string_view &sv) {
    auto pos = sv.find_first_of(' ');
    auto res = std::string(sv.substr(0, pos));
    if (pos != std::string_view::npos) sv.remove_prefix(pos + 1);
    return std::move(res);
}

template<>
inline std::tuple<std::optional<std::unique_ptr<expression>> > _parse_arg_ker<std::unique_ptr<expression>>(std::string_view &sv) {
    auto res = parse_expr(PRECEDENCE_LIMIT, sv);
    if (res) sv = std::string_view{};
    return res;
}

template<size_t = 0>
std::tuple<> _parse_arg_impl(std::string_view) {
    return std::make_tuple();
}

template<typename Arg, typename... Rest>
std::tuple<std::optional<Arg>, std::optional<Rest>...> _parse_arg_impl(std::string_view sv) {
    while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
    auto res = _parse_arg_ker<Arg>(sv);
    return std::tuple_cat(std::move(res), _parse_arg_impl<Rest...>(sv));
}

template<typename... Args>
std::tuple<std::optional<Args>...> parse_arg(std::string_view sv) {
    return _parse_arg_impl<Args...>(sv);
}

template<typename... Args>
std::tuple<std::optional<Args>...> parse_arg(const char *ptr) {
    if (ptr == nullptr) return std::make_tuple(std::optional<Args>{}...);
    return _parse_arg_impl<Args...>(std::string_view{ptr});
}
