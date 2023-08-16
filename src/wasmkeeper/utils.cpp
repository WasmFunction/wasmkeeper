#include "wasmkeeper/utils.hpp"

#include <fcntl.h>
#include <sched.h>
#include <unistd.h>

#include <iostream>

bool setup_net_ns(const std::string& netns) {
  if (netns.empty()) {
    return false;
  }

  int netFd = open(netns.c_str(), O_RDONLY);
  if (netFd == -1) {
    std::cerr << "failed to open netns.\n";
    return false;
  }

  if (setns(netFd, CLONE_NEWNET) == -1) {
    std::cerr << "error setns.\n";
    close(netFd);
    return false;
  }
  close(netFd);
  return true;
}

