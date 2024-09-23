#include <httplib.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <unistd.h>    // for pipe, dup2, read, close
#include <sstream>     // for std::ostringstream
#include <cstring>     // for strerror
#include <cerrno>      // for errno
#include "wasmkeeper/request.hpp"
#include "wasmkeeper/utils.hpp"
#include "wasmkeeper/wasmedgepp.hpp"



std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
        case '"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if ('\x00' <= *c && *c <= '\x1f') {
                o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
            } else {
                o << *c;
            }
        }
    }
    return o.str();
}

std::string format_output(const std::string& raw_output) {
    std::istringstream stream(raw_output);
    std::string line;
    std::string formatted_output = "[\n";
    
    while (std::getline(stream, line)) {
        formatted_output += "  \"" + escape_json(line) + "\",\n"; // 使用 escape_json 转义
    }

    if (formatted_output.size() > 2) {
        formatted_output.pop_back();
        formatted_output.pop_back();
    }

    formatted_output += "\n]";
    return formatted_output;
}

std::string capture_stdout_with_pipe(wasmkeeper::Vm &vm) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe: " << strerror(errno) << std::endl;
        return "";
    }

    int stdout_save = dup(STDOUT_FILENO);
    if (stdout_save == -1) {
        std::cerr << "Error saving stdout: " << strerror(errno) << std::endl;
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }

    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
        std::cerr << "Error redirecting stdout: " << strerror(errno) << std::endl;
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }
    close(pipefd[1]); 

    vm.run();  

    if (dup2(stdout_save, STDOUT_FILENO) == -1) {
        std::cerr << "Error restoring stdout: " << strerror(errno) << std::endl;
        close(pipefd[0]);
        return "";
    }
    close(stdout_save);  

    std::ostringstream oss;
    char buffer[256];
    ssize_t count;
    while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        oss.write(buffer, count);
    }
    close(pipefd[0]);

    return oss.str();
}

int main(int argc, char** argv) {
  constexpr auto ADDR = "0.0.0.0";
  constexpr int PORT = 10086;

  constexpr auto SUCCESS_RESP = "{\"status\": 0}";
  constexpr auto FAIL_RESP = "{\"status\": 1}";

  CLI::App app{"A simple http server that runs wasm functions."};
  std::string netns = "";
  std::string mod_path = "";
  app.add_option("-n,--netns", netns, "Network namespace.")->required();
  app.add_option("-m,--mod-path", mod_path, "The path to the wasm module.")
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

  info() << "using mod path: " << mod_path << std::endl;

  std::shared_ptr<wasmkeeper::Config> config;
  std::unique_ptr<wasmkeeper::Module> module;
  try {
    config = std::make_shared<wasmkeeper::Config>();
    module = std::make_unique<wasmkeeper::Module>(config, mod_path);
  } catch (const wasmkeeper::Error& e) {
    error() << e.what() << std::endl;
    return 1;
  }

  httplib::Server server;
  server.Post("/", [&](const httplib::Request& req, httplib::Response& res) {
    std::string captured_output;
    auto vm = std::make_unique<wasmkeeper::Vm>(config);
    wasmkeeper::Request request; 
    try {
      std::vector<std::string> args = {"program"};
      if (!req.body.empty()) {
        if (request.parse(req.body)) {
            args.insert(args.end(), request.args.begin(), request.args.end());
        }
      }
      vm->load_wasm_from_loader(*module);
      vm->wasi_init(args, {}, {});
      captured_output = capture_stdout_with_pipe(*vm);
      std::string formatted_output = format_output(captured_output);
      std::string success_response = "{\"status\": 0, \"output\": " + formatted_output + "}";
      res.set_content(success_response, "application/json");
    } catch (const wasmkeeper::Error& e) {
      std::string error_response = "{\"status\": 1, \"error\": \"" + escape_json(e.what()) + "\"}";
      res.set_content(error_response, "application/json");
      return;
    }
  });

  info() << "wasmkeeper listening at " << ADDR << ":" << PORT << "..." << std::endl;
  server.listen(ADDR, PORT);

  return 0;
}
