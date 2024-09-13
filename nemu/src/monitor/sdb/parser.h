#pragma once
#include <tuple>
#include <string_view>
#include <optional>
#include <charconv>
#include <string>

template<typename Arg>
std::tuple<std::optional<Arg>> parse(std::string_view sv);

template<>
std::tuple<std::optional<int>> parse<int>(std::string_view sv) {
    int res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) return std::make_tuple(res);
    return std::make_tuple(std::nullopt);
}

template<>
std::tuple<std::optional<double>> parse<double>(std::string_view sv) {
    double res = 0;
    if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res); ec == std::errc()) return std::make_tuple(res);
    return std::make_tuple(std::nullopt);
}

template<>
std::tuple<std::optional<std::string>> parse<std::string>(std::string_view sv) {
    return std::string(sv);
}

template<typename Arg, typename... Rest>
std::tuple<std::optional<Arg>, std::optional<Rest>...> parse(std::string_view sv) {
    while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
    auto spacePos = sv.find_first_of(' ');
    return std::tuple_cat(parse<Arg>(sv.substr(0, spacePos)), parse<Rest...>(spacePos == std::string_view::npos ? std::string_view{} : sv.substr(spacePos + 1)));
}
