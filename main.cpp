#include <httplib.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>

#include "wasmkeeper/request.hpp"
#include "wasmkeeper/utils.hpp"
#include "wasmkeeper/wasmedgepp.hpp"

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
  if (!setup_net_ns(netns)) {
    errorln("failed to set up netNS. use host instead.");
  }

  info() << "using mod path: " << modPath << std::endl;

  std::shared_ptr<wasmkeeper::Config> config;
  std::unique_ptr<wasmkeeper::Module> module;
  try {
    config = std::make_shared<wasmkeeper::Config>();
    module = std::make_unique<wasmkeeper::Module>(config, modPath);
  } catch (const wasmkeeper::Error& e) {
    error() << e.what() << std::endl;
    return 1;
  }

  httplib::Server server;
  server.Post("/", [&](const httplib::Request& req, httplib::Response& res) {
    auto vm = std::make_unique<wasmkeeper::Vm>(config);
    try {
      if (!req.body.empty()) {
        wasmkeeper::Request request;
        if (request.parse(std::string(req.body))) {
          vm->wasi_init(request.args, {}, {});
        }
      }
      vm->load_wasm_from_loader(*module);
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
