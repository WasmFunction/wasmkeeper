#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace wasmkeeper {
struct Request {
  template <class string_type = std::string>
  bool parse(string_type&& raw) noexcept {
    try {
      auto json = nlohmann::json::parse(std::forward<string_type>(raw));
      for (auto&& e : json["args"]) {
        args.push_back(e);
      }
      return true;
    } catch (const std::exception& e) {
      return false;
    }
  }

  std::vector<std::string> args;
};
}  // namespace wasmkeeper
