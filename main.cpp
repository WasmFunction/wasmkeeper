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

  std::cout << "[INFO] using netns: " << netns << '\n';
  setup_net_ns(netns);

  std::cout << "[INFO] using mod path: " << modPath << '\n';
  try {
    wasmkeeper::ModuleLoader::build(modPath);
    wasmkeeper::Config::build();
  } catch (const wasmkeeper::Error& e) {
    std::cerr << "[ERROR] " << e.what() << '\n';
    return 1;
  }

  httplib::Server server;
  server.Post("/", [&](const httplib::Request& req, httplib::Response& res) {
    wasmkeeper::Vm vm;
    try {
      if (!req.body.empty()) {
        auto args = get_args(req.body);
        vm.wasi_init(args, {}, {});
      }
      vm.load_wasm_from_loader(wasmkeeper::ModuleLoader::build(modPath));
      vm.run();

    } catch (const wasmkeeper::Error& e) {
      std::cerr << "[ERROR] " << e.what() << '\n';
      res.set_content("{\"status\": 1}", "text/plain");
      return;
    }
    res.set_content("{\"status\": 0}", "text/plain");
  });

  std::cout << "[INFO] listening at 0.0.0.0:10086.\n";
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
