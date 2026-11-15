##Description

So, this is just a really simple rendering library based on WebGPU and GLFW, and I want it to be compatible with Emscripten (compiled via emcc/em++). 
The point of this project is to have fun, and gain experience with new concepts. I also wanted to make a 3D rendering library that I could personally use
to make 3D games that I can compile with Emscripten and also run it natively. I know that Raylib does this already, but making my own is more fun, and I
know exactly what goes on under the hood (Except for the math, I don't think I'll ever be able to wrap my head around projection matrices*). This is also
my first time with GLFW and WebGPU, so don't expect the craziest best thing in the world (if anything, expect something mid to horrible).

*I might not use projection matrices, actually. I might just divide by Z.

#Things to Know

This is made by one person, so therefore it might not be very optimized, efficient, and may not produce the best results.
I am sorry if my API is not great, as it is my first time making a library.

#Note

I hope you a good experience with this library. Any feedback is extremely important to me, as it is probably something that should be there, but I neglected.
Please report any bugs you can find, because I probably won't be using this library too much. I will still probably use it every so often, and I hope you do too.