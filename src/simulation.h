#ifndef SIMULATION_H
#define SIMULATION_H

#include "common_sim.h"
#include "spatialgrid.h"
#include "hair.h"
#include "fluid.h"
#include "shader.h"

using namespace CommonSim;
namespace Sim {

class Simulation {
   public:
    Simulation(HairConfigs hairConfigs, FluidConfig fluidConfig);
    ~Simulation() {}

    // Perform all pre-processing steps
    void preprocess();

    // Load all buffers, excluding Particles and predicted positions
    void populateBuffers();

    void update();

    void tick();

    void render();

    Rods::Hair* hair;
    PBF::Fluid* fluid;
    Shader* simulationShader;
    unsigned VAO;
};
}  // namespace Sim

#endif /* SIMULATION_H */
