#include "wasmkeeper/vm.hpp"

#include <wasmedge/wasmedge.h>

#include <cstring>
#include <iostream>

wasmkeeper::Config& wasmkeeper::Config::build() {
  static Config Config;
  return Config;
}

wasmkeeper::Config::Config() : ctx(WasmEdge_ConfigureCreate()) {
  if (!ctx) {
    throw Error("failed to create wasmedge configuration.");
  }
  WasmEdge_ConfigureAddHostRegistration(ctx, WasmEdge_HostRegistration_Wasi);
}

wasmkeeper::Config::~Config() { WasmEdge_ConfigureDelete(ctx); }

wasmkeeper::Vm::Vm() {
  ctx = WasmEdge_VMCreate(Config::build().get_conf_cxt(), NULL);
  if (!ctx) {
    throw Error("failed to create wasmedge vm.");
  }
}

static std::vector<const char*> convert_arr(const std::vector<std::string>& arr) {
  if (arr.empty()) {
    return {};
  }
  std::vector<const char*> res(arr.size());
  for (int i = 0; i < arr.size(); ++i) {
    res[i] = arr[i].c_str();
  }
  return res;
}

void wasmkeeper::Vm::wasi_init(const std::vector<std::string>& args,
                               const std::vector<std::string>& envs,
                               const std::vector<std::string>& preopens) {
  WasmEdge_ModuleInstanceContext* wasiModule =
      WasmEdge_VMGetImportModuleContext(ctx, WasmEdge_HostRegistration_Wasi);
  if (!wasiModule) {
    throw Error("failed to init wasi module.");
  }

  auto cargs = convert_arr(args);
  auto cenvs = convert_arr(envs);
  auto cpreopens = convert_arr(preopens);

  WasmEdge_ModuleInstanceInitWASI(wasiModule, cargs.data(), cargs.size(),
                                  cenvs.data(), cenvs.size(), cpreopens.data(),
                                  cpreopens.size());

}

void wasmkeeper::Vm::run_start(const char* filepath) {
  WasmEdge_String start = WasmEdge_StringCreateByCString("_start");
  WasmEdge_Result res =
      WasmEdge_VMRunWasmFromFile(ctx, filepath, start, NULL, 0, NULL, 0);
  WasmEdge_StringDelete(start);

  if (!WasmEdge_ResultOK(res)) {
    std::string err =
        std::string("error running start: ") + WasmEdge_ResultGetMessage(res);
    throw Error(err.c_str());
  }
}

wasmkeeper::Vm::~Vm() { WasmEdge_VMDelete(ctx); }
