#pragma once

#include <iostream>
#include <string>

bool setup_net_ns(const std::string& netns) noexcept;

inline auto& info() noexcept {
  std::cout << "[INFO] ";
  return std::cout;
}

inline auto& error() noexcept {
  std::cerr << "[ERROR] ";
  return std::cerr;
}

inline void infoln(const char* message) noexcept {
  info() << message << std::endl;
}

inline void errorln(const char* message) noexcept {
  error() << message << std::endl;
}

