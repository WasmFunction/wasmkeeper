#include "wasmkeeper/utils.hpp"

#include <fcntl.h>
#include <sched.h>
#include <unistd.h>

bool setup_net_ns(const std::string& netns) noexcept {
  if (netns.empty()) {
    return false;
  }

  int net_fd = open(netns.c_str(), O_RDONLY);
  if (net_fd == -1) {
    errorln("failed to open netns.");
    return false;
  }

  if (setns(net_fd, CLONE_NEWNET) == -1) {
    errorln("error setns.");
    close(net_fd);
    return false;
  }
  close(net_fd);
  return true;
}
