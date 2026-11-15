# cpp3d-wgpu
A simple 3D rendering library built on **WebGPU** and **GLFW**, designed to run natively and compile to the web via **Emscripten** (`emcc`/`em++`).

## About

The point of this project is to have fun, and gain experience with new concepts. I also wanted to make a 3D rendering library that I could personally use
to make 3D games that I can compile with Emscripten and also run it natively. I know that Raylib does this already, but making my own is more fun, and I
know exactly what goes on under the hood (Except for the math, I don't think I'll ever be able to wrap my head around projection matrices*). This is also
my first time with GLFW and WebGPU, so don't expect the craziest best thing in the world (if anything, expect something mid to horrible).

*I might not use projection matrices, actually. I might just divide by Z.

## How to use

### Including

To include this library you can just write:

```cpp
#include <cpp3d-wgpu.hpp>
```
Or if you are forking / modifying my code:

```cpp
#include "cpp3d-wgpu.hpp"
```

### Namespaces

* `Engine` — The main namespace for the project.
* `RenderConfig` — Contains render settings (functions, variables, and enums).

```cpp
RenderConfig::RenderMethod; // Enumerator for render methods
Engine::Renderer;           // Main renderer class
```

### Dependencies

* `webgpu.h` - Will work with both `wgpu-native` and `Dawn` (after changing include block a bit), this implementation is made to use `wgpu-native`'s `webgpu-headers/webgpu.h`.
* `glfw3` - You can install it in multiple ways depending on OS.
    * **macOS:** `brew install glfw`
    * **Ubuntu/Devian:** `sudo apt install libglfw3-dev`
    * **Windows (vcpkg):** `vcpkg install glfw3`
* `glfw3webgpu` - Install through **GitHub**, link is [`glfw3webgpu`](https://github.com/eliemichel/glfw3webgpu)

```bash
brew install 
```


## Things to Know

This is made by one person, so therefore it might not be very optimized, efficient, and may not produce the best results.
I am sorry if my API is not great, as it is my first time making a library.

## Note

I hope you have a good experience with my library. Any feedback is extremely important to me, as it is probably something that should be there, but I neglected.
Please report any bugs you can find in an issue, because I probably won't be using this library too much. I will still probably use it every so often, and I hope you do too.

## License

This project is licensed under the **MIT License**.
See the [LICENSE](LICENSE) for details.