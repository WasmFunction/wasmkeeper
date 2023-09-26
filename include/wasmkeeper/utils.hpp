#pragma once

#include <exception>
#include <string>
#include <vector>

auto setup_net_ns(const std::string& netns) -> bool;

auto info() -> std::ostream&;

auto error() -> std::ostream&;
