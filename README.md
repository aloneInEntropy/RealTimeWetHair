# Real-Time Wet Hair Simulation using Afro-Textured Hair

![An image of wet, afro-textured hair. The hair is clumped up and darkened in wet areas.](docs/image.png)

By Iris Onwa

This repository is the implementation of my thesis by the above name, submitted for my Master's thesis at Trinity College Dublin.

It features an implementation of 
- Position and Orientation Based Cosserat Rods by Kugelstadt and Schömer (2016),
- Position Based Fluids by Macklin and Müller (2013), and
- Coupling Hair with Smoothed Particle Hydrodynamics Fluids by Lin (2014)

It was submitted on April 18th, 2025. (Currently ungraded).

The code is implemented using OpenGL 4.6 and C++. GLFW was used for windowing and GLAD for headers. I also used Dear ImGui for the GUI.

[Read the thesis here.](docs/thesis.pdf)

The version of this repo that was submitted is available at <https://github.com/irisonwa/RealTimeWetHair/tree/b1c7393d38a897683c609c5c499475d405f57956>

repo will be cleaned up over time

## Simulation

Full video demonstration: <https://youtu.be/z68gn46AZ0U>

### How to run

To run this project, you will require:
- [Dear ImGui](https://github.com/ocornut/imgui) [Present]
- [GLAD](https://github.com/Dav1dde/glad) [Present]
- [Assimp](https://github.com/assimp/assimp)
- [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h)
- [GLFW](https://github.com/glfw/glfw)
- [CMake](https://cmake.org/)

First, build the project using CMake. This project was built using GCC on Windows through [MSYS2](https://www.msys2.org/), so it is build from the project directory like so:
```
cmake . -G MinGW Makefiles -DCMAKE_CXX_COMPILER = C:/msys64/mingw64/bin/g++.exe -DCMAKE_C_COMPILER = C:/msys64/mingw64/bin/gcc.exe -B build
```
then
```
cd build

cmake --build build -j 12
```
to use 12 cores to build the project. Finally, from `/build`, run
```
./main.exe
```

The number of hair strands must be determined in the program before the application is run. They are set in the constructor of the `Simulation` class. The exact number is determined by the number of vertices on the head model. Specifically, one hair strand is generated for every (upward-facing) vertex on the [head model](Models/largehead/largehead.gltf). The Blender files are provided if you want to change the size of the head. The head's radius must also be set in the `Hair` class for collision detection.

As an example, to create a head with a radius of 20 units:
1. Configure the head shape in Blender.
    1. In the [guidehead](models/guidehead/guidehead.blend) and [largehead](models/largehead/largehead.blend) files, create an Isosphere. You can control the number of hair strands here using `Subdivisions` option. For reference, 5 subdivisons results in 1,365 strands and 6 subdivisions results in 5,287 strands. The radius can be changed by either setting the `Radius` option upon creation, or by leaving it as 1 and scaling the object.
    2. Normals must be merged to prevent multiple hair strands generating from the same location, which would decreate the different locations strands can be generated in. After the sphere is created, switch to `Edit Mode` and go to `Mesh -> Normals -> Merge` to merge split-vertex normals. 
    3. Create a simple texture and export as a separated GLTF file.
2. Set the head radius in the [`Hair` class](https://github.com/irisonwa/RealTimeWetHair/blob/main/src/hair.h#L375): 
```cpp
/* src/hair.cpp:375 */
float renderHeadRadius = 20; // 20 = radius set in Blender
```
3. Generate hair strands based on the head sphere's normals. In this example, we only generate strands that are point out/up.
```cpp
/* src/main.cpp:21 */
int strandVertexCount = 15;
HairConfigs hs;
for (int i = 0; i < largeHead->vertexData.size(); ++i) {
    if (largeHead->vertexData[i].pos.y >= 0) {
        // only normals facing out/up
        hs.push_back({strandVertexCount, vec4(largeHead->vertexData[i].pos, 1), largeHead->vertexData[i].norm});
    }
}
```

### Abstract

The upkeep of afro-textured hair has had a profound impact on African and coily-haired people, requiring constant care and preening to ensure it remains healthy. This comes through the use of various oils, conditioners, and shampoos. Afro-textured hair is also much drier and porous than its counterparts, which leads to interesting interactions with these fluids. However, simulations of these interactions are sparse. Simulating afro-textured hair accurately has an impact on performance, as it tends to be far denser than straighter hair, leading to constant collision handling. Therefore, any simulation involving both hair and fluids must make sacrifices in accuracy for performance. These sacrifices are great when implemented in real-time applications, such as video games. We present an application that showcases the interactions of afro-textured hair and fluids in real-time while minimising a loss in accuracy. We will use the Position-Based Dynamics (PBD) framework, which trades some accuracy for performance, for both the hair and fluid simulations. Cosserat theory will be used to implement angular constraints to ensure hair strands remain coily. To boost the application’s performance and obtain more accuracy, we will implement both simulations on the GPU, allowing for immense parallelisation. Our application can be run on low-end consumer devices such as laptops, proving its low performance impact. Future work lies in improving the accuracy of the simulation while balancing performance.

### Details

This thesis was written to determine the efficacy of wet, afro-textured hair simulations in real-time applications such as video games. Ultimately, I came to the conclusion that, using the method I implemented, it is possible, although a "low-end" laptop likely won't be sufficient for a relatively dense field of hair.

The simulation is run on the GPU using [compute shaders](https://github.com/aloneInEntropy/RealTimeWetHair/tree/main/Shaders/sim). I used atomic bump allocation (see [Section 3.7.3](docs/thesis.pdf) in the thesis) for creating a fixed grid and searching for nearby neighbours. Hair-fluid interactions were implemented through the use of *porous particles*, which were sampled between each hair vertex. They underwent separate physics interactions with fluids and applied their velocities to their adjacent hair vertices.

Hair clumps when wet, so the end result is that nearby strands are attracted to each other, resulting in the desired clumped look. Fluid density and saturation and hair porosity and density are used to determine wetness. As water diffuses out of the hair, they gradually return to their rest shape.

Fluid diffusion is handled by gradually increasing the force of gravity of fluid particles near porous particles. The amount that gravity increases is dependent on the fluid density, so small droplets of fluid particles will take longer to diffuse out of hair strands than larger droplets. This, coupled with an adhesion force, allows for a semi-realistic dripping behaviour. Fluid slowly drips out of the hair in real-time, and the hair slowly returns to normal.
