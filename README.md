# Description

So, this is just a really simple rendering library based on WebGPU and GLFW, and I want it to be compatible with Emscripten (compiled via emcc/em++). 
The point of this project is to have fun, and gain experience with new concepts. I also wanted to make a 3D rendering library that I could personally use
to make 3D games that I can compile with Emscripten and also run it natively. I know that Raylib does this already, but making my own is more fun, and I
know exactly what goes on under the hood (Except for the math, I don't think I'll ever be able to wrap my head around projection matrices*). This is also
my first time with GLFW and WebGPU, so don't expect the craziest best thing in the world (if anything, expect something mid to horrible).

*I might not use projection matrices, actually. I might just divide by Z.

## How to use

### Including

To include this library you can just write
```cpp
#include <cpp3d-wgpu.hpp>
```
or 
```cpp
#include "cpp3d-wgpu.hpp"
```
if you are forking / modifying my code.

### Namespaces
The main namespace for this project is 'Engine'.
The render settings (functions, variables, enums) are in a separate namespace named 'RenderConfig'.
```cpp
RenderConfig::RenderMethod; // Enumerator for render methods
Engine::Renderer; // Main renderer class
```

## Things to Know

This is made by one person, so therefore it might not be very optimized, efficient, and may not produce the best results.
I am sorry if my API is not great, as it is my first time making a library.

## Note

I hope you have a good experience with my library. Any feedback is extremely important to me, as it is probably something that should be there, but I neglected.
Please report any bugs you can find, because I probably won't be using this library too much. I will still probably use it every so often, and I hope you do too.

## License

MIT License

Copyright (c) 2026 @manjun-stuff

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.