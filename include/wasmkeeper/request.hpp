#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace wasmkeeper {
struct Request {
  bool parse(std::string&& raw) noexcept {
    try {
      auto json = nlohmann::json::parse(std::move(raw));
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
}
