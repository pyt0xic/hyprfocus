#pragma once
#define WLR_USE_UNSTABLE
#include <hyprland/src/debug/log/Logger.hpp>

// Compatibility constants for log levels
constexpr auto LOG = Log::DEBUG;
constexpr auto ERR = Log::ERR;

template <typename... Args>
void hyprfocus_log(Hyprutils::CLI::eLogLevel level, std::format_string<Args...> fmt,
                   Args &&...args) {
  if (!Log::logger)
    return;
  auto msg = std::vformat(fmt.get(), std::make_format_args(args...));
  Log::logger->log(level, "[hyprfocus] {}", msg);
}
