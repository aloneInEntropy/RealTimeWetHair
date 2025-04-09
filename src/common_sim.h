#ifndef COMMON_SIM_H
#define COMMON_SIM_H

#include "sm.h"
#include "util.h"

#define DISPATCH_SIZE 8  // intel i7-1260p

class SpatialGrid;

namespace CommonSim {

// The phase a particle is in during computation
enum Phase {
    HAIR,   // The particle is treated as hair
    SOLID,  // The particle is treated as part of a rigid body
    FLUID   // The particle is treated as a fluid
};

// The step of the fluid simulation to run
enum FluidSimulationStep {
    APPLY_EXTERNAL_FORCES,
    COMPUTE_DENSITIES,
    COMPUTE_VISCOSITES,
    PREDICT,
    DENSITY_CONSTRAINT,
    UPDATE_VELOCITIES,
    N_STAGES = 6
};

// Particle distribution
enum PD {
    DAM_BREAK,
    DOUBLE_DAM_BREAK
};

using HairConfig = std::tuple<int, vec4, vec3>;
using HairConfigs = std::vector<HairConfig>;
using FluidConfig = std::tuple<int, PD, vec3>;

// A particle. Its position and velocity are `vec4`s to avoid alignment issues in SSBOs
struct Particle {
    Particle(vec3 pos, vec3 vel, float mass, Phase phase) {
        x = vec4(pos, 0);
        v = vec4(vel, 0);
        w = mass;
        t = phase;
    }
    vec4 x;   // particle position
    vec4 v;   // particle velocity
    float w;  // particle inverse mass
    Phase t;  // particle type. one of HAIR, SOLID, or FLUID
    int pd1, pd2;
};

// Indirect drawing command for `glMultiDrawArraysIndirect`
struct IndirectArrayDrawCommand {
    unsigned int vertexCount;
    unsigned int instanceCount;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

// List of `Particle` structs.
// All simulations will use this buffer and an offset to determine the computation the particles will be used for
extern std::vector<Particle> particles;

// List of `Particle` predicted positions
extern std::vector<vec4> ps;

// Radius of a single particle 
extern float particleRadius;

// The 3D grid containing the simulation.
extern SpatialGrid* grid;

/* SSBOs */
extern unsigned particleBuffer;
extern unsigned predictedPositionBuffer;

extern bool play;
extern float dt;
extern float sdt;
extern int simulationSubsteps;
extern int simulationIterations;
extern int simulationTick;
extern int nextTick;
extern bool ticking;
extern bool renderWhileTicking;
extern int hairVertexStartIdx;
extern int nHairParticles;
extern int fluidVertexStartIdx;
extern int nFluidParticles;
extern int nTotalParticles;

/* Fluid Physics */;
extern float smoothingRadius;
extern float collisionDamping;
extern float restDensity;
extern float restDensityInv;
extern float relaxationEpsilon;
extern float SOR;
extern float f_cohesion;
extern float f_curvature;
extern float f_viscosity;
extern float f_adhesion;
extern float boundaryDensityCoeff;
extern vec3 gravityDir;

/* Hair Physics */;
extern vec3 torque;
extern float drag;
extern float a_drag;

}  // namespace CommonSim

#endif /* COMMON_SIM_H */
