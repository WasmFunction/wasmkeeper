#include "wasmkeeper/utils.hpp"

#include <fcntl.h>
#include <sched.h>
#include <unistd.h>

#include <iostream>

auto setup_net_ns(const std::string& netns) -> bool {
  if (netns.empty()) {
    return false;
  }

  int netFd = open(netns.c_str(), O_RDONLY);
  if (netFd == -1) {
    error() << "failed to open netns." << std::endl;
    return false;
  }

  if (setns(netFd, CLONE_NEWNET) == -1) {
    error() << "error setns." << std::endl;
    close(netFd);
    return false;
  }
  close(netFd);
  return true;
}

auto info() -> std::ostream& {
  std::cout << "[INFO] ";
  return std::cout;
}

auto error() -> std::ostream& {
  std::cerr << "[ERROR] ";
  return std::cerr;
}
