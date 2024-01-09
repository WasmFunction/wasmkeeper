#pragma once

#include <wasmedge/wasmedge.h>

#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace wasmkeeper {
class Error : public std::exception {
 public:
  Error(const char* msg) noexcept : message(msg) {}

  const char* what() const noexcept { return message.c_str(); }

 private:
  std::string message;
};

class Config {
 public:
  Config();

  ~Config() { WasmEdge_ConfigureDelete(m_ctx); }

  const WasmEdge_ConfigureContext* raw() const noexcept { return m_ctx; }

 private:
  WasmEdge_ConfigureContext* m_ctx;
};

class Module {
 public:
  Module(std::shared_ptr<Config> m_config, const std::string& filepath);

  ~Module() { WasmEdge_ASTModuleDelete(m_ctx); }

  const WasmEdge_ASTModuleContext* raw() const noexcept { return m_ctx; }

 private:
  std::shared_ptr<Config> m_config;
  WasmEdge_ASTModuleContext* m_ctx;
};

class Vm {
 public:
  Vm(std::shared_ptr<Config> m_config);

  ~Vm() { WasmEdge_VMDelete(m_ctx); }

  void wasi_init(const std::vector<std::string>& args,
                 const std::vector<std::string>& envs,
                 const std::vector<std::string>& preopens);

  void load_wasm_from_loader(const Module& loader);

  void run();

 private:
  std::shared_ptr<Config> m_config;
  WasmEdge_VMContext* m_ctx;
};
}  // namespace wasmkeeper
