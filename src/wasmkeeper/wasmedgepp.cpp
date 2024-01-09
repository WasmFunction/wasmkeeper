#include "wasmkeeper/wasmedgepp.hpp"

#include <wasmedge/wasmedge.h>

#include <cstring>

static auto convert_arr(const std::vector<std::string>& arr)
    -> std::vector<const char*> {
  if (arr.empty()) {
    return {};
  }
  std::vector<const char*> res(arr.size());
  for (int i = 0; i < arr.size(); ++i) {
    res[i] = arr[i].c_str();
  }
  return res;
}

namespace wasmkeeper {

Config::Config() : m_ctx(WasmEdge_ConfigureCreate()) {
  if (!m_ctx) {
    throw Error("failed to create wasmedge configuration.");
  }
  WasmEdge_ConfigureAddHostRegistration(m_ctx, WasmEdge_HostRegistration_Wasi);
}

Module::Module(std::shared_ptr<Config> config, const std::string& filepath)
    : m_config(config) {
  WasmEdge_LoaderContext* load_cxt = WasmEdge_LoaderCreate(m_config->raw());
  WasmEdge_Result res =
      WasmEdge_LoaderParseFromFile(load_cxt, &m_ctx, filepath.c_str());
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_LoaderDelete(load_cxt);
    throw Error("failed to load wasm module.");
  }
  WasmEdge_LoaderDelete(load_cxt);
}

Vm::Vm(std::shared_ptr<Config> config) : m_config(config) {
  m_ctx = WasmEdge_VMCreate(m_config->raw(), NULL);
  if (!m_ctx) {
    throw Error("failed to create wasmedge vm.");
  }
}

void Vm::wasi_init(const std::vector<std::string>& args,
                   const std::vector<std::string>& envs,
                   const std::vector<std::string>& preopens) {
  WasmEdge_ModuleInstanceContext* wasi_module =
      WasmEdge_VMGetImportModuleContext(m_ctx, WasmEdge_HostRegistration_Wasi);
  if (!wasi_module) {
    throw Error("failed to init wasi module.");
  }

  auto cargs = convert_arr(args);
  auto cenvs = convert_arr(envs);
  auto cpreopens = convert_arr(preopens);

  WasmEdge_ModuleInstanceInitWASI(wasi_module, cargs.data(), cargs.size(),
                                  cenvs.data(), cenvs.size(), cpreopens.data(),
                                  cpreopens.size());
}

void Vm::load_wasm_from_loader(const Module& loader) {
  WasmEdge_Result res = WasmEdge_VMLoadWasmFromASTModule(m_ctx, loader.raw());
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to load wasi module from loader.");
  }
  res = WasmEdge_VMValidate(m_ctx);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to validate wasi module for vm.");
  }
}

void wasmkeeper::Vm::run() {
  WasmEdge_Result res = WasmEdge_VMInstantiate(m_ctx);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to init vm.");
  }
  WasmEdge_String func_name = WasmEdge_StringCreateByCString("_start");
  res = WasmEdge_VMExecute(m_ctx, func_name, NULL, 0, NULL, 0);
  if (!WasmEdge_ResultOK(res)) {
    throw Error("failed to run vm.");
  }
  WasmEdge_StringDelete(func_name);
}
}  // namespace wasmkeeper
