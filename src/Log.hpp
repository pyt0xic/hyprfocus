#pragma once
#define WLR_USE_UNSTABLE
#include <hyprland/src/debug/log/Logger.hpp>

template <typename... Args>
void hyprfocus_log(Hyprutils::CLI::eLogLevel level, std::format_string<Args...> fmt,
                   Args &&...args) {
  auto msg = std::vformat(fmt.get(), std::make_format_args(args...));
  Log::logger->log(level, "[hyprfocus] {}", msg);
}
