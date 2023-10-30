## wasmkeeper

A simple http server that runs wasm functions.

### Required

- emscripten
- https://emscripten.org/docs/getting_started/downloads.html

```bash
emcc -o example/func.wasm example/func.c
```

## build

### Required

- wasmedge v0.11.2
- jsoncpp v1.9.5
- https://wasmedge.org/docs/embed/c/reference/0.11.x#wasmedge-installation

configure

```bash
mkdir build && cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake

# for example
cmake .. -DCMAKE_TOOLCHAIN_FILE=/root/vcpkg/scripts/buildsystems/vcpkg.cmake

ln -s build/compile_commands.json .
```

build

```bash
mkdir build && cd build

cmake --build .
```

## test

```bash
curl -X POST -H "Content-Type: application/json" -d '{"args":["White", "Hank"]}' localhost:10086

# benchmark
ab -n 750 -c 10 -p data.json -T application/json http://localhost:10086/
```
