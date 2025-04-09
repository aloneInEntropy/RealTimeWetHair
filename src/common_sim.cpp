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
int nHairParticles = 0;
int fluidVertexStartIdx = 0;
int nFluidParticles = 0;
int nTotalParticles = 0;

/* Fluid Physics */
float smoothingRadius = 1.4f;
float collisionDamping = 1;
float restDensity = -5e3f;
float restDensityInv = 1 / restDensity;
float relaxationEpsilon = 1e-2f;
float SOR = 1.7f;
float f_cohesion = 40.f;
float f_curvature = 1e-3f;
float f_viscosity = 0.1f;
float f_adhesion = 0;
float boundaryDensityCoeff = 1;
vec3 gravityDir = vec3(0, -10, 0);

/* Hair Physics */
vec3 torque = vec3(0, 0, 0);  // torque around each axis
float drag = 0.98f;           // vertex air resistance
float a_drag = 0.6f;          // rod rotation resistance
}  // namespace CommonSim
