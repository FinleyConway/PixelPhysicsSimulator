# PixelPhysicsSimulator
A Noita-like simulation using cellular automata

## Building
See the official Raylib build instructions:
https://github.com/raysan5/raylib#build-and-installation

```bash
git clone https://github.com/FinleyConway/PixelPhysicsSimulator.git
cd PixelPhysicsSimulator
cmake -S . -B build && cmake --build build
./build/PixelPhysicsSimulator/PixelPhysicsSimulator # run simulator
```

>[!NOTE]
>Wayland is hardcoded, swap GLFW_BUILD_WAYLAND and GLFW_BUILD_X11 flags for X11 legacy systems
