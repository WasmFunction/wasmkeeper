#pragma once

#include <wasmedge/wasmedge.h>

#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace wasmkeeper {
class Error : public std::exception {
 public:
  Error(const char* msg) : message(msg) {}

  auto what() const noexcept -> const char* { return message.c_str(); }

 private:
  std::string message;
};

class destroyer {
 public:
  template <typename T>
  void operator()(T* ptr) {
    ptr->destroy();
  }
};

template <class T>
using own = std::unique_ptr<T, destroyer>;

template <class T>
auto make_own(T* ptr) -> own<T> {
  return own<T>(ptr);
}

class Config {
 private:
  WasmEdge_ConfigureContext* cxt;

 private:
  friend class destroyer;
  void destroy() { WasmEdge_ConfigureDelete(cxt); }

 protected:
  Config();

  ~Config();

 public:
  static auto make() -> own<Config> { return make_own(new Config); }

  static auto build() -> const Config&;

  auto raw() const -> const WasmEdge_ConfigureContext* { return cxt; }
};

class Module {
 private:
  WasmEdge_ASTModuleContext* cxt;

 private:
  friend class destroyer;
  void destroy() { WasmEdge_ASTModuleDelete(cxt); }

 protected:
  Module(const std::string& filepath);

  ~Module();

 public:
  static auto make(const std::string& filepath) -> own<Module> {
    return make_own(new Module(filepath));
  }

  static auto build(const std::string& filepath) -> const Module&;

  auto raw() const -> const WasmEdge_ASTModuleContext* { return cxt; }
};

class Vm {
 private:
  WasmEdge_VMContext* cxt;

 private:
  friend class destroyer;
  void destroy() { WasmEdge_VMDelete(cxt); }

 protected:
  Vm();

  ~Vm();

 public:
  static auto make() -> own<Vm> {
    return make_own(new Vm);
  }

  void wasi_init(const std::vector<std::string>& args,
                 const std::vector<std::string>& envs,
                 const std::vector<std::string>& preopens);

  void load_wasm_from_loader(const Module& loader);

  void run();
};
}  // namespace wasmkeeper
