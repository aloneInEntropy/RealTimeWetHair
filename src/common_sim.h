#ifndef COMMON_SIM_H
#define COMMON_SIM_H

#include "sm.h"
#include "util.h"

#define DISPATCH_SIZE 1024  // intel i7-1260p

class SpatialGrid;

namespace CommonSim {

// The phase a particle is in during computation
enum Phase {
    HAIR,   // The particle is treated as hair
    PORE,   // The particle is treated as a hair boundary/porous particle
    FLUID,  // The particle is treated as a fluid
    SOLID   // The particle is treated as part of a rigid body
};

// The stage to dispatch the simulation to
enum SimulationStage {
    APPLY_EXTERNAL_FORCES,
    DIFFUSION,
    REP_VOLUME,
    COMPUTE_DENSITIES,
    COMPUTE_VISCOSITES,
    COMPUTE_FLUID_AUX,
    PREDICT,
    PREDICT_POROUS,
    RESOLVE_COLLISIONS,
    STRETCH_SHEAR_CONSTRAINT,
    BEND_TWIST_CONSTRAINT,
    DENSITY_CONSTRAINT,
    CLUMPING,
    UPDATE_VELOCITIES,
    UPDATE_POROUS,
    N_SIM_STAGES = 15
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
        d = 0;
    }
    vec4 x;   // particle position
    vec4 v;   // particle velocity
    float w;  // particle inverse mass
    Phase t;  // particle type. one of HAIR, SOLID, or FLUID
    float d;  // hair wetness for HAIR particles. mass diffusion for FLUID particles. undefined for any PORE particles
    int s;    // strand index for HAIR particles. undefined otherwise
};

// Indirect drawing command for `glMultiDrawArraysIndirect`
struct IndirectArrayDrawCommand {
    unsigned int vertexCount;
    unsigned int instanceCount;
    unsigned int baseVertex;
    unsigned int baseInstance;
};
// Indirect drawing command for `glMultiDrawElementsIndirect`
struct IndirectElementDrawCommand {
    unsigned int indexCount;
    unsigned int instanceCount;
    unsigned int baseIndex;
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
extern int hairParticleStartIdx;
extern int hairParticleCount;
extern int fluidParticleStartIdx;
extern int fluidParticleCount;
extern int porousParticleStartIdx;
extern int porousParticleCount;
extern int totalParticleCount;
extern vec3 centre;
extern vec3 bounds;
extern vec3 fv_gravity;

extern bool hairLoaded;
extern bool poresLoaded;
extern bool fluidLoaded;
}  // namespace CommonSim

#endif /* COMMON_SIM_H */
