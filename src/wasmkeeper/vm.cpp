#include "wasmkeeper/vm.hpp"

#include <wasmedge/wasmedge.h>

#include <cstring>
#include <iostream>

wasmkeeper::Config& wasmkeeper::Config::build() {
  static Config Config;
  return Config;
}

wasmkeeper::Config::Config() : cxt(WasmEdge_ConfigureCreate()) {
  if (!cxt) {
    throw Error("failed to create wasmedge configuration.");
  }
  WasmEdge_ConfigureAddHostRegistration(cxt, WasmEdge_HostRegistration_Wasi);
}

wasmkeeper::Config::~Config() { WasmEdge_ConfigureDelete(cxt); }

wasmkeeper::ModuleLoader& wasmkeeper::ModuleLoader::build(
    const std::string& filepath) {
  static ModuleLoader loader(filepath);
  return loader;
}

wasmkeeper::ModuleLoader::ModuleLoader(const std::string& filepath) {
  WasmEdge_LoaderContext* loadCxt =
      WasmEdge_LoaderCreate(Config::build().get_conf_cxt());
  WasmEdge_Result res =
      WasmEdge_LoaderParseFromFile(loadCxt, &cxt, filepath.c_str());
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_LoaderDelete(loadCxt);
    throw Error("failed to load wasm module.");
  }
  WasmEdge_LoaderDelete(loadCxt);
}

wasmkeeper::ModuleLoader::~ModuleLoader() { WasmEdge_ASTModuleDelete(cxt); }

wasmkeeper::Vm::Vm() {
  cxt = WasmEdge_VMCreate(Config::build().get_conf_cxt(), NULL);
  if (!cxt) {
    throw Error("failed to create wasmedge vm.");
  }
}

static std::vector<const char*> convert_arr(
    const std::vector<std::string>& arr) {
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
      WasmEdge_VMGetImportModuleContext(cxt, WasmEdge_HostRegistration_Wasi);
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

void wasmkeeper::Vm::load_wasm_from_loader(const ModuleLoader& loader) {
  WasmEdge_Result res =
      WasmEdge_VMLoadWasmFromASTModule(cxt, loader.get_module_cxt());
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to load wasi module from loader.");
  }
  res = WasmEdge_VMValidate(cxt);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to validate wasi module for vm.");
  }
}

void wasmkeeper::Vm::run() {
  WasmEdge_Result res = WasmEdge_VMInstantiate(cxt);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to init vm.");
  }
  WasmEdge_String funcName = WasmEdge_StringCreateByCString("_start");
  res = WasmEdge_VMExecute(cxt, funcName, NULL, 0, NULL, 0);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to run vm.");
  }
  WasmEdge_StringDelete(funcName);
}

wasmkeeper::Vm::~Vm() { WasmEdge_VMDelete(cxt); }
