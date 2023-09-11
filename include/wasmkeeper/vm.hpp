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

  WasmEdge_ConfigureContext* get_conf_cxt() { return cxt; }

 private:
  WasmEdge_ConfigureContext* cxt;
};

class ModuleLoader {
 public:
  ModuleLoader(const std::string& filepath);

  ~ModuleLoader();

  static ModuleLoader& build(const std::string& filepath);

  WasmEdge_ASTModuleContext* get_module_cxt() const { return cxt; }

 private:
  WasmEdge_ASTModuleContext* cxt;
};

class Vm {
 public:
  Vm();

  ~Vm();

  void wasi_init(const std::vector<std::string>& args,
                 const std::vector<std::string>& envs,
                 const std::vector<std::string>& preopens);

  void load_wasm_from_loader(const ModuleLoader& loader);

  void run();

 private:
  WasmEdge_VMContext* cxt;
};
}  // namespace wasmkeeper
