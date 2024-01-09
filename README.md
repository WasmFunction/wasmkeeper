## wasmkeeper

A simple http server that runs wasm functions.

### Required

- wasmedge v0.11.2

  https://wasmedge.org/docs/embed/c/reference/0.11.x#wasmedge-installation

- vcpkg

- g++

- cmake (version >= 3.19)

- make

### Build

**configure**

set `toolchainFile` to the path of `[vcpkg root]/scripts/buildsystems/vcpkg.cmake` in `CMakePresets.json`:

```json
{
  "version": 3,
  "configurePresets": [
    {
      ...
      "toolchainFile": "/home/youtirsin/vcpkg/scripts/buildsystems/vcpkg.cmake"
      ...
    }
  ]
}
```

and then run:

```bash
cmake --preset=default
```

**build**

```bash
cmake --build build --target wasmkeeper
```

**run**

```bash
build/wasmkeeper --netns xxx --mod-path ./example/func.wasm
```

## test

```bash
curl -X POST -H "Content-Type: application/json" -d '{"args":["White", "Hank"]}' localhost:10086

# benchmark
ab -n 750 -c 10 -p data.json -T application/json http://localhost:10086/
```
