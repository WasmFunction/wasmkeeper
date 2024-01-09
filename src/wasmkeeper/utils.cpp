#include "wasmkeeper/utils.hpp"

#include <fcntl.h>
#include <sched.h>
#include <unistd.h>

bool setup_net_ns(const std::string& netns) noexcept {
  if (netns.empty()) {
    return false;
  }

  int netFd = open(netns.c_str(), O_RDONLY);
  if (netFd == -1) {
    errorln("failed to open netns.");
    return false;
  }

  if (setns(netFd, CLONE_NEWNET) == -1) {
    errorln("error setns.");
    close(netFd);
    return false;
  }
  close(netFd);
  return true;
}
