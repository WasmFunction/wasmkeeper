#include <httplib.h>
#include <json/json.h>

#include <CLI11.hpp>
#include <iostream>

#include "wasmkeeper/utils.hpp"
#include "wasmkeeper/vm.hpp"

std::vector<std::string> get_args(const std::string& reqBody);

int main(int argc, char** argv) {
  CLI::App app{"A simple http server that runs wasm functions."};
  std::string netns = "";
  std::string modPath = "";
  app.add_option("-n,--netns", netns, "Network namespace.")->required();
  app.add_option("-m,--mod-path", modPath, "The path to the wasm module.")
      ->required();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }

  info() << "using netns: " << netns << std::endl;
  setup_net_ns(netns);

  info() << "using mod path: " << modPath << std::endl;
  try {
    wasmkeeper::Config::build();
    wasmkeeper::Module::build(modPath);
  } catch (const wasmkeeper::Error& e) {
    error() << e.what() << std::endl;
    return 1;
  }

  httplib::Server server;
  server.Post("/", [&](const httplib::Request& req, httplib::Response& res) {
    auto vm = wasmkeeper::Vm::make();
    try {
      if (!req.body.empty()) {
        auto args = get_args(req.body);
        vm->wasi_init(args, {}, {});
      }
      auto& module = wasmkeeper::Module::build(modPath);
      vm->load_wasm_from_loader(module);
      vm->run();

    } catch (const wasmkeeper::Error& e) {
      error() << e.what() << std::endl;
      res.set_content("{\"status\": 1}", "text/plain");
      return;
    }
    res.set_content("{\"status\": 0}", "text/plain");
  });

  info() << "listening at 0.0.0.0:10086." << std::endl;
  server.listen("0.0.0.0", 10086);

  return 0;
}

std::vector<std::string> get_args(const std::string& reqBody) {
  Json::Value val;
  Json::Reader reader;
  if (!reader.parse(reqBody, val)) {
    throw wasmkeeper::Error("error parsing request data.");
  }
  if (!val["args"].isArray()) {
    throw wasmkeeper::Error("error getting args.");
  }
  std::vector<std::string> res;
  for (const Json::Value& element : val["args"]) {
    if (element.isString()) {
      res.push_back(element.asString());
    }
  }
  return res;
}
