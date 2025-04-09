#include "simulation.h"

namespace Sim {

Simulation::Simulation(HairConfigs hairConfigs, FluidConfig fluidConfig) {
    hairVertexStartIdx = 0;
    hair = new Rods::Hair(hairConfigs);
    nHairParticles = particles.size();
    fluidVertexStartIdx = nHairParticles;
    fluid = new PBF::Fluid(fluidConfig);
    nFluidParticles = particles.size() - nHairParticles;
    nTotalParticles = nFluidParticles + nHairParticles;

    preprocess();
    populateBuffers();
    printf("%d hair strands, %d hair particles, %d fluid particles, %d total particles\n",
           hair->numStrands, nHairParticles, nFluidParticles, particles.size());
}

// Perform all pre-processing steps
void Simulation::preprocess() {
    grid->init();  // all particles should be initialised on object creation, but not buffered yet
    fluid->createFramebuffers();

    simulationShader = new Shader("simulation compute step",
                               {{DIR("Shaders/sim/hair/compute/hair.comp"), GL_COMPUTE_SHADER},
                                {DIR("Shaders/sim/fluid/compute/fluid.comp"), GL_COMPUTE_SHADER},
                                {DIR("Shaders/sim/grid_helper.comp"), GL_COMPUTE_SHADER},
                                {DIR("Shaders/sim/kernels.comp"), GL_COMPUTE_SHADER},
                                {DIR("Shaders/sim/simulation.comp"), GL_COMPUTE_SHADER}});
}

// Load all buffers, excluding Particles and predicted positions
void Simulation::populateBuffers() {
    grid->populateBuffers();   // load particles, predicted positions, and grid data
    hair->populateBuffers();   // load hair data (rods, darboux vectors, etc.)
    fluid->populateBuffers();  // load fluid data (densities, etc.)

    glCreateVertexArrays(1, &VAO);
}

void Simulation::update() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, predictedPositionBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, hair->rodBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, hair->hairStrandBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, hair->predictedRotationBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, hair->restDarbouxBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, hair->vertexStrandMapBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, hair->rodStrandMapBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, fluid->densitiesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, fluid->lambdasBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, fluid->curvatureNormalsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, fluid->omegasBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, grid->startIndicesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, grid->cellEntriesBuffer);


    glBindVertexArray(VAO);
    simulationShader->use();

    // todo: uniform buffer objects (again)
    simulationShader->setFloat("dt", sdt);
    simulationShader->setVec3("gravity", gravityDir);
    simulationShader->setFloat("dt", sdt);
    simulationShader->setFloat("smoothingRadius", smoothingRadius);
    simulationShader->setFloat("collisionDamping", collisionDamping);
    simulationShader->setFloat("restDensity", restDensity);
    simulationShader->setFloat("restDensityInv", restDensityInv);
    simulationShader->setFloat("relaxationEpsilon", relaxationEpsilon);
    simulationShader->setFloat("SOR", SOR);
    simulationShader->setFloat("f_cohesion", f_cohesion);
    simulationShader->setFloat("f_curvature", f_curvature);
    simulationShader->setFloat("f_viscosity", f_viscosity);
    simulationShader->setFloat("f_adhesion", f_adhesion);
    simulationShader->setFloat("boundaryDensityCoeff", boundaryDensityCoeff);
    simulationShader->setFloat("gridCellSize", grid->cellSize);
    simulationShader->setInt("nFluidParticles", nFluidParticles);
    simulationShader->setInt("nTotalParticles", nTotalParticles);
    simulationShader->setFloat("particleRadius", particleRadius);
    simulationShader->setVec3("bounds", fluid->bounds);
    simulationShader->setVec3("centre", fluid->centre);
    simulationShader->setVec3("gravityDir", gravityDir);
    simulationShader->setInt("substepCount", simulationSubsteps);
    simulationShader->setVec4("up", vec4(0, 1, 0, 0));
    simulationShader->setVec3("gravity", gravityDir);
    simulationShader->setVec4("torque", vec4(torque, 0));
    simulationShader->setMat3("inertia", hair->inertia);
    simulationShader->setFloat("l_drag", drag);
    simulationShader->setFloat("a_drag", a_drag);
    simulationShader->setFloat("ss_k", hair->ss_k);
    simulationShader->setFloat("bt_k", hair->bt_k);
    simulationShader->setFloat("ss_SOR", hair->ss_SOR);
    simulationShader->setFloat("bt_SOR", hair->bt_SOR);
    simulationShader->setFloat("headRad", hair->renderHeadRadius);

    // todo: check over this several times

    simulationShader->rmv();
    glBindVertexArray(0);
}

void Simulation::tick() {
    
}

void Simulation::render() {
    
}

}  // namespace Sim