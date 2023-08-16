#pragma once

#include <wasmedge/wasmedge.h>

#include <exception>
#include <string>
#include <vector>

namespace wasmkeeper {
class Error : public std::exception {
 public:
  Error(const char* msg) : message(msg) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class Config {
 public:
  static Config& build();

  Config();

  ~Config();

  WasmEdge_ConfigureContext* get_conf_cxt() { return ctx; }

 private:
  WasmEdge_ConfigureContext* ctx;
};

class Vm {
 public:
  Vm();

  ~Vm();

  void wasi_init(const std::vector<std::string>& args,
                const std::vector<std::string>& envs,
                const std::vector<std::string>& preopens);

  void run_start(const char* filepath);

 private:
  WasmEdge_VMContext* ctx;
};
}  // namespace wasmedge
