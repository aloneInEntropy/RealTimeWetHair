#include "common_sim.h"
#include "spatialgrid.h"

namespace CommonSim {
std::vector<Particle> particles;
std::vector<vec4> ps;
float particleRadius = 0.15f;
SpatialGrid* grid = new SpatialGrid();
unsigned particleBuffer = 0;
unsigned predictedPositionBuffer = 0;

bool play = false;
float dt = 1.f / 30;
float sdt = 1.f / 30;
int simulationSubsteps = 5;
int simulationIterations = 5;
int simulationTick = 0;
int nextTick = 113;
bool ticking = false;            // is the simulation actively moving towards the next tick?
bool renderWhileTicking = true;  // render the scene while ticking to the target tick
int hairParticleStartIdx = 0;
int hairParticleCount = 0;
int fluidParticleStartIdx = 0;
int fluidParticleCount = 0;
int porousParticleStartIdx = 0;
int porousParticleCount = 0;
int totalParticleCount = 0;
vec3 centre{150, 50, 150};
vec3 bounds{70, 90, 70};
vec3 fv_gravity = vec3(0, -30, 0);

bool hairLoaded = false;
bool poresLoaded = false;
bool fluidLoaded = false;
}  // namespace CommonSim
