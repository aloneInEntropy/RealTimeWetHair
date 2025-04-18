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
    void simulate();

    void tickTo(int t) {
        nextTick = t;
        ticking = true;
    }

    void printPhysicsSettings() {
        printf("TODO\n");
        // printf(
        //     "Gravity: [%f, %f, %f]\n"
        //     "Torque: [%f, %f, %f]\n"
        //     "Linear Drag: [%f]\n"
        //     "Angular Drag: [%f]\n",
        //     fv_gravity.x, fv_gravity.y, fv_gravity.z, hair->torque.x, hair->torque.y, hair->torque.z, hair->f_l_drag, hair->f_a_drag);
    }

    void printHairSettings() {
        printf("TODO\n");
    }
    
    void printSimulationSettings() {
        printf("TODO\n");
        // printf(
        //     "Stretch and Shear Compliance: %f\n"
        //     "Stretch and Shear SOR: %f\n"
        //     "Bend and Twist Compliance: %f\n"
        //     "Bend and Twist SOR: %f\n"
        //     "Substeps: %d\n"
        //     "Iterations: %d\n",
        //     hair->ss_k, hair->ss_SOR, hair->bt_k, hair->bt_SOR, simulationSubsteps, simulationIterations);
    }

    Rods::Hair* hair;
    PBF::Fluid* fluid;
    SpatialGrid* grid;
    Shader* simulationShader;
    unsigned VAO;
};
}  // namespace Sim

#endif /* SIMULATION_H */
