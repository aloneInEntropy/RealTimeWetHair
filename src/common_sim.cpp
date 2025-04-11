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
int simulationSubsteps = 10;
int simulationIterations = 1;
int simulationTick = 0;
int nextTick = 113;
bool ticking = false;            // is the simulation actively moving towards the next tick?
bool renderWhileTicking = true;  // render the scene while ticking to the target tick
int hairVertexStartIdx = 0;
int hairParticleCount = 0;
int fluidVertexStartIdx = 0;
int fluidParticleCount = 0;
int porousVertexStartIdx = 0;
int porousParticleCount = 0;
int nTotalParticles = 0;
vec3 centre{50, 50, 50};
vec3 bounds{35, 35, 35};
vec3 fv_gravity = vec3(0, -10, 0);

bool hairLoaded = false;
bool poresLoaded = false;
bool fluidLoaded = false;
}  // namespace CommonSim
