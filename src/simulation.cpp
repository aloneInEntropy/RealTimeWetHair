#include "simulation.h"

namespace Sim {

Simulation::Simulation(HairConfigs hairConfigs, FluidConfig fluidConfig) {
    grid = new SpatialGrid();

    hairParticleStartIdx = 0;
    hair = new Rods::Hair(hairConfigs);
    hairParticleCount = particles.size();
    hairLoaded = true;
    fluidParticleStartIdx = hairParticleCount;
    fluid = new PBF::Fluid(fluidConfig);
    fluidParticleCount = particles.size() - hairParticleCount;
    fluidLoaded = true;
    porousParticleStartIdx = fluidParticleCount;
    hair->samplePorousParticles();
    porousParticleCount = particles.size() - hairParticleCount - fluidParticleCount;
    poresLoaded = true;
    totalParticleCount = hairParticleCount + fluidParticleCount + porousParticleCount;

    preprocess();
    populateBuffers();
    printf("%d hair strands, %d hair particles, %d fluid particles, %d porous particles, %d total particles\n",
           hair->numStrands, hairParticleCount, fluidParticleCount, porousParticleCount, particles.size());
}

// Perform all pre-processing steps
void Simulation::preprocess() {
    grid->init();  // all particles should be initialised on object creation, but not buffered yet
    fluid->createFramebuffers();
    simulationShader = new Shader("simulation compute step",
                                  {{DIR("Shaders/sim/compute/helper.comp"), GL_COMPUTE_SHADER},
                                   {DIR("Shaders/sim/compute/kernels.comp"), GL_COMPUTE_SHADER},
                                   {DIR("Shaders/sim/compute/simulation.comp"), GL_COMPUTE_SHADER}});
}

// Load all buffers, excluding Particles and predicted positions
void Simulation::populateBuffers() {
    grid->populateBuffers();   // load particles, predicted positions, and grid data
    hair->populateBuffers();   // load hair data (rods, darboux vectors, etc.) and porous particle data
    fluid->populateBuffers();  // load fluid data (densities, etc.)

    glCreateVertexArrays(1, &VAO);
}

void Simulation::simulate() {
    /* Dispatch grid reconstruction outside substeps */
    grid->dispatchKernels();

    glBindVertexArray(VAO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, predictedPositionBuffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grid->startIndicesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grid->cellEntriesBuffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, fluid->densitiesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, fluid->lambdasBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, fluid->curvatureNormalsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, fluid->omegasBuffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, hair->poreDataBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, hair->rodBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, hair->predictedRotationBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, hair->hairStrandBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, hair->restDarbouxBuffer);

    simulationShader->use();

    // todo: uniform buffer objects
    float sdt = dt / simulationSubsteps;
    fluid->restDensityInv = 1.f / fluid->restDensity;
    simulationShader->setFloat("dt", sdt);
    simulationShader->setInt("hairParticleCount", hairParticleCount);
    simulationShader->setInt("fluidParticleCount", fluidParticleCount);
    simulationShader->setInt("porousParticleCount", porousParticleCount);
    simulationShader->setVec3("bounds", bounds);
    simulationShader->setVec3("centre", centre);
    simulationShader->setFloat("ss_SOR", hair->ss_SOR);
    simulationShader->setFloat("ss_k", hair->ss_k);
    simulationShader->setFloat("bt_SOR", hair->bt_SOR);
    simulationShader->setFloat("bt_k", hair->bt_k);
    simulationShader->setFloat("dn_SOR", fluid->SOR);
    simulationShader->setFloat("dn_k", fluid->k);
    simulationShader->setFloat("particleRadius", particleRadius);
    simulationShader->setFloat("smoothingRadius", fluid->smoothingRadius);
    simulationShader->setFloat("restDensity", fluid->restDensity);
    simulationShader->setFloat("restDensityInv", fluid->restDensityInv);
    simulationShader->setFloat("relaxationEpsilon", fluid->relaxationEpsilon);
    simulationShader->setFloat("f_cohesion", fluid->f_cohesion);
    simulationShader->setFloat("f_curvature", fluid->f_curvature);
    simulationShader->setFloat("f_viscosity", fluid->f_viscosity);
    simulationShader->setFloat("f_adhesion", fluid->f_adhesion);
    simulationShader->setFloat("f_porosity", hair->f_porosity);
    simulationShader->setFloat("f_clumping", hair->f_clumping);
    simulationShader->setFloat("f_l_drag", hair->f_l_drag);
    simulationShader->setFloat("f_a_drag", hair->f_a_drag);
    simulationShader->setVec3("fv_gravity", fv_gravity);
    simulationShader->setVec3("fv_torque", hair->torque);
    simulationShader->setMat3("inertia", hair->inertia);
    simulationShader->setVec3("up", Util::UP);
    simulationShader->setMat4("headTrans", hair->headTrans);
    simulationShader->setFloat("headRad", hair->renderHeadRadius);
    simulationShader->setInt("poreSamples", hair->poreSamples);
    simulationShader->setFloat("gridCellSize", grid->cellSize);
    simulationShader->setInt("simulationTick", simulationTick);
    simulationShader->setInt("clumpingRange", hair->clumpingRange);
    simulationShader->setFloat("fluidMassDiffusionFactor", fluid->fluidMassDiffusionFactor);

    for (int i = 0; i < simulationSubsteps; ++i) {
        for (int stage = 0; stage < N_SIM_STAGES; ++stage) {
            simulationShader->setInt("stage", stage);
            simulationShader->setInt("rbgs", -1);
            switch (stage) {
                case APPLY_EXTERNAL_FORCES:
                    glDispatchCompute(ceil((hairParticleCount + fluidParticleCount) / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case REP_VOLUME:
                    glDispatchCompute(ceil(porousParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case COMPUTE_DENSITIES:
                    glDispatchCompute(ceil(((fluidParticleCount + porousParticleCount)) / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case COMPUTE_VISCOSITES:
                    glDispatchCompute(ceil(fluidParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case COMPUTE_FLUID_AUX:
                    glDispatchCompute(ceil(fluidParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case PREDICT:
                    glDispatchCompute(ceil((hairParticleCount + fluidParticleCount) / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case PREDICT_POROUS:
                    glDispatchCompute(ceil(porousParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case RESOLVE_COLLISIONS:
                    glDispatchCompute(ceil(totalParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case STRETCH_SHEAR_CONSTRAINT:
                case BEND_TWIST_CONSTRAINT:
                    for (int iter = 0; iter < simulationIterations; ++iter) {
                        simulationShader->setInt("rbgs", 0);
                        glDispatchCompute(ceil((hairParticleCount / 2) / DISPATCH_SIZE) + 1, 1, 1);
                        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                        simulationShader->setInt("rbgs", 1);
                        glDispatchCompute(ceil((hairParticleCount / 2) / DISPATCH_SIZE) + 1, 1, 1);
                        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    }
                    break;
                case DENSITY_CONSTRAINT:
                    glDispatchCompute(ceil(fluidParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case CLUMPING:
                    glDispatchCompute(ceil(porousParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case UPDATE_VELOCITIES:
                    glDispatchCompute(ceil((hairParticleCount + fluidParticleCount) / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                case UPDATE_POROUS:
                    glDispatchCompute(ceil(porousParticleCount / DISPATCH_SIZE) + 1, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    break;
                default:
                    break;
            }
        }
    }

    simulationShader->rmv();
    glBindVertexArray(0);
    simulationTick++;
}

void Simulation::update() {
    if (Input::isKeyJustPressed(Key::SPACE) && !SM::cfg.isCamMode) play = !play;
    if (play) {
        if ((ticking && simulationTick < nextTick) || !ticking) {
            simulate();
        }
    }
}

}  // namespace Sim
