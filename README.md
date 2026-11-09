So this is just a really simple rendering library based on WebGPU and GLFW, and I want it to be compatible with Emscripten (compiled via emcc/em++). 
The point of this project is to have fun and gain experience with new concepts. I also wanted to make a 3D rendering library that I could personally use
to make 3D games that I can compile with Emscripten and also run it natively. I know that Raylib does this already, but making my own is more fun, and I
know exactly what goes on under the hood. (Except for the math, I don't think I'll ever be able to wrap my head around projection matrices*) This is also
my first time with GLFW and WebGPU, so don't expect the craziest best thing in the world (if anything, expect something mid to horrible).

*I might not use projection matrices, actually. I might just divide by Z.