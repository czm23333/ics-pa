#pragma once
#include <tuple>
#include <string_view>
#include <optional>
#include <charconv>
#include <string>

template<typename Arg>
std::tuple<std::optional<Arg> > parse_ker(std::string_view &sv);

template<>
inline std::tuple<std::optional<int> > parse_ker<int>(std::string_view &sv) {
    int res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) {
        sv.remove_prefix(ptr - sv.data());
        return std::make_tuple(res);
    }
    return std::make_tuple(std::nullopt);
}

template<>
inline std::tuple<std::optional<double> > parse_ker<double>(std::string_view &sv) {
    double res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) {
        sv.remove_prefix(ptr - sv.data());
        return std::make_tuple(res);
    }
    return std::make_tuple(std::nullopt);
}

template<>
inline std::tuple<std::optional<std::string> > parse_ker<std::string>(std::string_view &sv) {
    auto pos = sv.find_first_of(' ');
    auto res = std::string(sv.substr(0, pos));
    if (pos != std::string_view::npos) sv.remove_prefix(pos + 1);
    return std::move(res);
}

template<size_t = 0>
std::tuple<> parse_impl(std::string_view) {
    return std::make_tuple();
}

template<typename Arg, typename... Rest>
std::tuple<std::optional<Arg>, std::optional<Rest>...> parse_impl(std::string_view sv) {
    while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
    auto res = parse_ker<Arg>(sv);
    return std::tuple_cat(res, parse_impl<Rest...>(sv));
}

template<typename... Args>
std::tuple<std::optional<Args>...> parse(std::string_view sv) {
    return parse_impl<Args...>(sv);
}

template<typename... Args>
std::tuple<std::optional<Args>...> parse(const char *ptr) {
    if (ptr == nullptr) return std::make_tuple(std::optional<Args>{}...);
    return parse_impl<Args...>(std::string_view{ptr});
}
