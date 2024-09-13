#pragma once
#include <tuple>
#include <string_view>
#include <optional>
#include <charconv>
#include <string>

template<typename Arg>
std::tuple<std::optional<Arg>> parse_ker(std::string_view sv);

template<>
std::tuple<std::optional<int>> parse_ker<int>(std::string_view sv) {
    int res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) return std::make_tuple(res);
    return std::make_tuple(std::nullopt);
}

template<>
std::tuple<std::optional<double>> parse_ker<double>(std::string_view sv) {
    double res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) return std::make_tuple(res);
    return std::make_tuple(std::nullopt);
}

template<>
std::tuple<std::optional<std::string>> parse_ker<std::string>(std::string_view sv) {
    return std::string(sv);
}

template<size_t unused = 0>
std::tuple<> parse_impl(std::string_view sv) {
    return std::make_tuple();
}

template<typename Arg, typename... Rest>
std::tuple<std::optional<Arg>, std::optional<Rest>...> parse_impl(std::string_view sv) {
    while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
    auto spacePos = sv.find_first_of(' ');
    return std::tuple_cat(parse_ker<Arg>(sv.substr(0, spacePos)), parse_impl<Rest...>(spacePos == std::string_view::npos ? std::string_view{} : sv.substr(spacePos + 1)));
}

template<typename... Args>
std::tuple<std::optional<Args>...> parse(std::string_view sv) {
    return parse_impl<Args...>(sv);
}
