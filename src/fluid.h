#ifndef FLUID_H
#define FLUID_H

#include "camera.h"
#include "cubemap.h"
#include "input.h"
// #include "kernels.h"
#include "shader.h"
#include "framebuffer.h"
#include "sm.h"
#include "staticmesh.h"
#include "util.h"
#include "common_sim.h"
#include "spatialgrid.h"
// #include "simulation.h"

// class Simulation;

#define USING_GPU

using namespace CommonSim;
namespace Sim {
namespace PBF {

enum FluidRenderStage {
    SPRITE,
    VELOCITY,
    DEPTH,
    THICKNESS,
    SMOOTH_DEPTH,
    SMOOTH_THICKNESS,
    NORMAL,
    COMPOSITION
};

class Fluid {
   public:
    Fluid(FluidConfig cfg) {
        auto& [n, pd, offs] = cfg;
        // smoothingRadius = grid->cellSize;
        createParticles(n, pd, offs);
        densities.resize(nTotalParticles);
        lambdas.resize(nTotalParticles);
        deltas.resize(nTotalParticles);
        omegas.resize(nTotalParticles);
        curvatureNormals.resize(nTotalParticles);
        transforms.resize(nTotalParticles);

        spriteShader = new Shader("fluid", DIR("Shaders/sim/fluid/render/fluid.vert"), DIR("Shaders/sim/fluid/render/fluid.frag"));
        // simulationShader = new Shader("fluid compute",
        //                               {
        //                                   {DIR("Shaders/fluid/compute/kernels.comp"), GL_COMPUTE_SHADER},
        //                                   {DIR("Shaders/fluid/compute/fluid.comp"), GL_COMPUTE_SHADER},
        //                                   {DIR("Shaders/fluid/compute/grid.comp"), GL_COMPUTE_SHADER},
        //                               });
        depthPassShader = new Shader("depth pass", DIR("Shaders/sim/fluid/render/fluid.vert"), DIR("Shaders/sim/fluid/render/depth_pass.frag"));
        thicknessPassShader = new Shader("depth pass", DIR("Shaders/sim/fluid/render/fluid.vert"), DIR("Shaders/sim/fluid/render/thickness_pass.frag"));

        // grid->init();
        // populateBuffers();
        // render();
    }

    void createParticles(int n, PD pd, vec3 offset = vec3(0)) {
        int cubeRoot = ceil(pow(n, 1.f / 3.f) - 1e-5);  // ceil(n) = n + 1 if n is a whole number, or some other floating point bullshit
        float spacing = smoothingRadius * .75f;
        nFluidParticles = n;
        vec3 halfBounds = (bounds - vec3(particleRadius)) / 2.f;

        /* Add fluid particles */
        if (pd == PD::DAM_BREAK) {
            for (int i = 0; i < n; ++i) {
                vec3 p = Util::to3D(i, vec3(cubeRoot));
                vec3 gridPos = (p - vec3(cubeRoot / 2.f + .5f)) * spacing;
                gridPos += centre + offset;
                Particle part = Particle(gridPos, vec3(0), 1, FLUID);
                particles.push_back(part);
                ps.push_back(vec4(gridPos, 0));
            }
        } else if (pd == PD::DOUBLE_DAM_BREAK) {
            // todo
            // vec2 centre = {SM::width / 2, SM::height * 0.5f};
            // for (int i = 0; i < n; i += 2) {
            //     float x = (i % ppr - ppr / 2.f + .5f) * spacing / 2;
            //     float y = (i / ppr - ppc / 2.f + .5f) * spacing / 2;
            //     vec2 pos = centre + halfBounds / 2.f + vec2(x, y);
            //     particles.emplace_back(pos, vec2(0, 0), 1, FLUID);
            //     ps.push_back(pos);
            // }
            // for (int i = 1; i < n; i += 2) {
            //     float x = (i % ppr - ppr / 2.f + .5f) * spacing / 2;
            //     float y = (i / ppr - ppc / 2.f + .5f) * spacing / 2;
            //     vec2 pos = centre - vec2(halfBounds.x, -halfBounds.y) / 2.f + vec2(x, y);
            //     particles.emplace_back(pos, vec2(0, 0), 1, FLUID);
            //     ps.push_back(pos);
            // }
        }

        /* Add boundary particles */
        // ! way too many particles are added
        // float diam = (smoothingRadius * .25f);  // tighten gap
        // int width_in_particles = (bounds.x / diam) + 1;
        // int height_in_particles = (bounds.y / diam) + 1;
        // int depth_in_particles = (bounds.z / diam) + 1;

        // float bmass = 100;
        // for (int y = 0; y < height_in_particles; ++y) {
        //     for (int z = 0; z < depth_in_particles; ++z) {
        //         vec4 p1 = vec4(centre + vec3(0, y * diam, z * diam) - halfBounds, 0);
        //         vec4 p2 = vec4(centre + vec3(bounds.x, y * diam, z * diam) - halfBounds, 0);
        //         particles.emplace_back(p1, vec4(0), bmass, SOLID);
        //         particles.emplace_back(p2, vec4(0), bmass, SOLID);
        //         ps.push_back(p1);
        //         ps.push_back(p2);
        //         nBoundaryParticles += 2;
        //     }
        // }
        // for (int x = 0; x < width_in_particles; ++x) {
        //     for (int z = 0; z < depth_in_particles; ++z) {
        //         vec4 p1 = vec4(centre + vec3(x * diam, 0, z * diam) - halfBounds, 0);
        //         vec4 p2 = vec4(centre + vec3(x * diam, bounds.y, z * diam) - halfBounds, 0);
        //         particles.emplace_back(p1, vec4(0), bmass, SOLID);
        //         particles.emplace_back(p2, vec4(0), bmass, SOLID);
        //         ps.push_back(p1);
        //         ps.push_back(p2);
        //         nBoundaryParticles += 2;
        //     }
        // }
        // for (int x = 0; x < width_in_particles; ++x) {
        //     for (int y = 0; y < height_in_particles; ++y) {
        //         vec4 p1 = vec4(centre + vec3(x * diam, y * diam, 0) - halfBounds, 0);
        //         vec4 p2 = vec4(centre + vec3(x * diam, y * diam, bounds.z) - halfBounds, 0);
        //         particles.emplace_back(p1, vec4(0), bmass, SOLID);
        //         particles.emplace_back(p2, vec4(0), bmass, SOLID);
        //         ps.push_back(p1);
        //         ps.push_back(p2);
        //         nBoundaryParticles += 2;
        //     }
        // }

        nTotalParticles = nFluidParticles + nBoundaryParticles;
    }

    void populateBuffers() {
        glCreateVertexArrays(1, &VAO);
        auto bf = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;

        glCreateBuffers(1, &densitiesBuffer);
        glNamedBufferStorage(densitiesBuffer, sizeof(float) * densities.size(), densities.data(), bf);

        glCreateBuffers(1, &lambdasBuffer);
        glNamedBufferStorage(lambdasBuffer, sizeof(float) * lambdas.size(), lambdas.data(), bf);

        glCreateBuffers(1, &curvatureNormalsBuffer);
        glNamedBufferStorage(curvatureNormalsBuffer, sizeof(vec4) * curvatureNormals.size(), curvatureNormals.data(), bf);

        glCreateBuffers(1, &omegasBuffer);
        glNamedBufferStorage(omegasBuffer, sizeof(vec4) * omegas.size(), omegas.data(), bf);

        // load command buffer
        IndirectArrayDrawCommand* cmds = new IndirectArrayDrawCommand[1];
        unsigned int baseInstance = 0, baseVertex = 0;
        for (int i = 0; i < 1; ++i) {
            int vCount = 1;

            cmds[i].vertexCount = 1;
            cmds[i].instanceCount = nTotalParticles;
            cmds[i].baseVertex = 0;
            cmds[i].baseInstance = 0;

            baseVertex++;
            baseInstance++;
        }

        // send command buffers to gpu
        glCreateBuffers(1, &commandBuffer);
        glNamedBufferStorage(commandBuffer, sizeof(IndirectArrayDrawCommand) * 1, &cmds[0], bf);
    }

    // todo: either add fbo to all render stages, or create new stage with depth + env
    void createFramebuffers() {
        // create framebuffers
        
        depthFBO = new Framebuffer("fluid depth", DIR("Shaders/quad.vert"), DIR("Shaders/quad.frag"), SM::width, SM::height);
        depthFBO->addTexture("depth colour texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        depthFBO->addRenderbuffer("depth render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);

        thicknessFBO1 = new Framebuffer("fluid thickness", DIR("Shaders/quad.vert"), DIR("Shaders/quad.frag"),
                                        SM::width / thicknessInvScale, SM::height / thicknessInvScale);
        thicknessFBO1->addTexture("depth colour texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        thicknessFBO1->addRenderbuffer("depth render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        thicknessFBO2 = new Framebuffer("fluid thickness", DIR("Shaders/quad.vert"), DIR("Shaders/quad.frag"), SM::width, SM::height);
        thicknessFBO2->addTexture("depth colour texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        thicknessFBO2->addRenderbuffer("depth render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        
        blurFBO = new Framebuffer("narrow-range", DIR("Shaders/quad.vert"), DIR("Shaders/sim/fluid/render/narrow_range_filter.frag"), SM::width, SM::height);
        blurFBO->addTexture("blur pass texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        blurFBO->addRenderbuffer("depth render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        
        tempBlur1FBO = new Framebuffer("temp narrow-range 1", DIR("Shaders/quad.vert"), DIR("Shaders/sim/fluid/render/narrow_range_filter.frag"), SM::width, SM::height);
        tempBlur1FBO->addTexture("temp blur pass texture 1", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        tempBlur1FBO->addRenderbuffer("temp blur 1 render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        tempBlur2FBO = new Framebuffer("temp narrow-range 2", DIR("Shaders/quad.vert"), DIR("Shaders/sim/fluid/render/narrow_range_filter.frag"), SM::width, SM::height);
        tempBlur2FBO->addTexture("temp blur pass 2 texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        tempBlur2FBO->addRenderbuffer("temp blur 2 render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);

        normalsFBO = new Framebuffer("smoothed normal", DIR("Shaders/quad.vert"), DIR("Shaders/sim/fluid/render/normal_pass.frag"), SM::width, SM::height);
        normalsFBO->addTexture("normal texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        normalsFBO->addRenderbuffer("normal render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        
        envFBO = new Framebuffer("environment", DIR("Shaders/quad.vert"), DIR("Shaders/quad.frag"), SM::width, SM::height);
        envFBO->addTexture("env colour texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT0);
        envFBO->addTexture("env depth texture", GL_TEXTURE_2D, GL_RGBA32F, GL_COLOR_ATTACHMENT1);
        envFBO->addRenderbuffer("env render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
        envFBO->addDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
        
        compositionFBO = new Framebuffer("composition texture", DIR("Shaders/quad.vert"), DIR("Shaders/sim/fluid/render/composition_pass.frag"), SM::width, SM::height);
        compositionFBO->addTexture("composited blurred depth texture", blurFBO->textures[0]->tex, GL_TEXTURE_2D);
        compositionFBO->addTexture("composited normal texture", normalsFBO->textures[0]->tex, GL_TEXTURE_2D);
        compositionFBO->addTexture("composited thickness texture", thicknessFBO2->textures[0]->tex, GL_TEXTURE_2D);
        compositionFBO->addTexture("cubemap texture", SM::skybox->tex->texture, GL_TEXTURE_CUBE_MAP);
        compositionFBO->addTexture("environment", envFBO->textures[0]->tex, GL_TEXTURE_2D);
        compositionFBO->addTexture("environment dpth", envFBO->textures[1]->tex, GL_TEXTURE_2D);
        compositionFBO->addRenderbuffer("normal render buffer", GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT);
    }

    void bindKernels() {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, predictedPositionBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, densitiesBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lambdasBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, curvatureNormalsBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, grid->startIndicesBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, grid->cellEntriesBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, omegasBuffer);
    }

    // dispatch compute shader kernels
    // todo
    void simulateGPU() {
        // /* Dispatch grid reconstruction outside substeps */
        // grid->dispatchKernels();

        // glBindVertexArray(VAO);
        // bindKernels();
        // // todo: uniform block objects

        // sdt = dt / simulationSubsteps;
        // restDensityInv = 1.f / restDensity;
        // simulationShader->use();
        // simulationShader->setVec3("gravity", gravityDir);
        // simulationShader->setFloat("dt", sdt);
        // simulationShader->setFloat("smoothingRadius", smoothingRadius);
        // simulationShader->setFloat("collisionDamping", collisionDamping);
        // simulationShader->setFloat("restDensity", restDensity);
        // simulationShader->setFloat("restDensityInv", restDensityInv);
        // simulationShader->setFloat("relaxationEpsilon", relaxationEpsilon);
        // simulationShader->setFloat("SOR", SOR);
        // simulationShader->setFloat("f_cohesion", f_cohesion);
        // simulationShader->setFloat("f_curvature", f_curvature);
        // simulationShader->setFloat("f_viscosity", f_viscosity);
        // simulationShader->setFloat("f_adhesion", f_adhesion);
        // simulationShader->setFloat("boundaryDensityCoeff", boundaryDensityCoeff);
        // simulationShader->setFloat("gridCellSize", grid->cellSize);
        // simulationShader->setInt("nFluidParticles", nFluidParticles);
        // simulationShader->setInt("nTotalParticles", nTotalParticles);
        // simulationShader->setFloat("particleRadius", particleRadius);
        // simulationShader->setVec3("bounds", bounds);
        // simulationShader->setVec3("centre", centre);
        // simulationShader->setVec3("gravityDir", gravityDir);
        // simulationShader->setInt("substepCount", simulationSubsteps);

        // for (int s = 0; s < simulationSubsteps; ++s) {
        //     for (int stage = 0; stage < FluidSimulationStep::N_STAGES; ++stage) {
        //         simulationShader->setInt("stage", stage);
        //         if (stage == DENSITY_CONSTRAINT) {
        //             glDispatchCompute(ceil((nFluidParticles) / DISPATCH_SIZE) + 1, 1, 1);
        //             glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //         } else {
        //             glDispatchCompute(ceil(nFluidParticles / DISPATCH_SIZE) + 1, 1, 1);
        //             glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //         }
        //     }
        // }
        // simulationTick++;
    }

    void update() {
        if (Input::isKeyJustPressed(Key::SPACE) && !SM::cfg.isCamMode) play = !play;
        if (play) {
            if ((ticking && simulationTick < nextTick) || !ticking) {
                simulateGPU();
            }
        } else {
            if (Input::isKeyJustPressed('Q')) {
                tick();
            }
        }
    }

    void renderSprites(bool showVelocity) {
        // https://gamedev.stackexchange.com/questions/54391/scaling-point-sprites-with-distance
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float heightOfNearPlane = (float)abs(viewport[3] - viewport[1]) / (2 * tan(Util::rad(0.5 * SM::camera->FOV)));

        spriteShader->use();
        spriteShader->setMat4("view", SM::camera->getViewMatrix());
        spriteShader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        spriteShader->setVec3("viewPos", SM::camera->pos);
        spriteShader->setFloat("particleRadius", particleRadius);
        spriteShader->setBool("showOutline", showOutline);
        spriteShader->setBool("showLighting", !showVelocity);
        spriteShader->setBool("showBounds", showBounds);
        spriteShader->setFloat("nearPlaneHeight", heightOfNearPlane);
        spriteShader->setFloat("depthStrength", depthStrength);
        spriteShader->setInt("startIdx", fluidParticleStartIdx);

        glBindVertexArray(VAO);
        bindKernels();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);  // rebind command buffer
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glDrawArraysIndirect(GL_POINTS, 0);
    }

    void renderDepth() {
        depthFBO->bind();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);  // enable colour blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float heightOfNearPlane = (float)abs(viewport[3] - viewport[1]) / (2 * tan(Util::rad(0.5 * SM::camera->FOV)));

        depthPassShader->use();
        depthPassShader->setMat4("view", SM::camera->getViewMatrix());
        depthPassShader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        depthPassShader->setVec3("viewPos", SM::camera->pos);
        depthPassShader->setFloat("particleRadius", particleRadius);
        depthPassShader->setBool("showOutline", showOutline);
        depthPassShader->setBool("showLighting", showLighting);
        depthPassShader->setBool("showBounds", showBounds);
        depthPassShader->setFloat("nearPlaneHeight", heightOfNearPlane);
        depthPassShader->setFloat("depthStrength", depthStrength);
        depthPassShader->setInt("startIdx", fluidParticleStartIdx);

        glBindVertexArray(VAO);
        bindKernels();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);  // rebind command buffer
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glDrawArraysIndirect(GL_POINTS, 0);

        glDisable(GL_DEPTH_TEST);
        depthFBO->unbind();
    }

    void renderThickness() {
        vec2 sz = {SM::width, SM::height};

        /* render into smaller quad */
        thicknessFBO1->bind();
        glViewport(0, 0, SM::width / thicknessInvScale, SM::height / thicknessInvScale);
        SM::updateWindow(SM::width / thicknessInvScale, SM::height / thicknessInvScale);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float heightOfNearPlane = (float)abs(viewport[3] - viewport[1]) / (2 * tan(Util::rad(0.5 * SM::camera->FOV)));

        thicknessPassShader->use();
        thicknessPassShader->setMat4("view", SM::camera->getViewMatrix());
        thicknessPassShader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        thicknessPassShader->setVec3("viewPos", SM::camera->pos);
        thicknessPassShader->setFloat("particleRadius", particleRadius);
        thicknessPassShader->setBool("showOutline", showOutline);
        thicknessPassShader->setBool("showLighting", showLighting);
        thicknessPassShader->setBool("showBounds", showBounds);
        thicknessPassShader->setFloat("nearPlaneHeight", heightOfNearPlane);
        thicknessPassShader->setFloat("thicknessStrength", thicknessStrength);
        thicknessPassShader->setInt("startIdx", fluidParticleStartIdx);

        glBindVertexArray(VAO);
        bindKernels();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);  // rebind command buffer
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glDrawArraysIndirect(GL_POINTS, 0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_TRUE);
        glViewport(0, 0, sz.x, sz.y);
        SM::updateWindow(sz.x, sz.y);
        thicknessFBO1->unbind();

        /* render into fullscreen quad */
        thicknessFBO2->bind();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        thicknessFBO1->draw();
        thicknessFBO2->unbind();
    }

    void smoothTextures() {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float heightOfNearPlane = (float)abs(viewport[3] - viewport[1]) / (2 * tan(Util::rad(0.5 * SM::camera->FOV)));
        auto blur = [&](vec2 stride, int dimension, Framebuffer* src, Framebuffer* dst, float thresholdMult = 1, bool useOwnShader = false) {
            if (!useOwnShader) {
                dst->quadShader->use();
                dst->quadShader->setFloat("particleRadius", particleRadius);
                dst->quadShader->setVec2("stride", stride);
                dst->quadShader->setInt("filterDim", dimension);
                dst->quadShader->setInt("filterRadius", kernelHalfWidth);
                dst->quadShader->setInt("maxFilterSize", maxKernelHalfWidth);
                dst->quadShader->setFloat("nearPlaneHeight", heightOfNearPlane);
                dst->quadShader->setFloat("thresholdRatio", thresholdRatio * thresholdMult);
                dst->quadShader->setFloat("clampRatio", clampRatio);
            }

            dst->bind();
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);  // enable colour blending
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            src->draw(false, 0, useOwnShader);  // draw the src texture without binding its shader
            glDisable(GL_DEPTH_TEST);
            dst->unbind();
        };

        /* ping pong smoothed texture between tempBlur1 and tempBlur2 */
        blur({1.f / SM::width, 0}, 1, depthFBO, tempBlur1FBO);
        blur({0, 1.f / SM::height}, 1, tempBlur1FBO, tempBlur2FBO);
        blur({1.f / SM::width, 0}, 1, tempBlur2FBO, tempBlur1FBO);
        blur({0, 1.f / SM::height}, 1, tempBlur1FBO, tempBlur2FBO);
        blur({1.f / SM::width, 1.f / SM::height}, 2, tempBlur2FBO, blurFBO);

        /* 2D blur. the second pass will use the thicknessFBO's shader, which merely transfers the texture across */
        blur({1.f / SM::width, 1.f / SM::height}, 2, thicknessFBO2, tempBlur1FBO, 10);
        blur({0, 0}, 0, tempBlur1FBO, thicknessFBO2, 10, true);
    }

    void calcNormals() {
        normalsFBO->bind();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);
        // glEnable(GL_BLEND);  // enable colour blending
        // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        normalsFBO->quadShader->use();
        normalsFBO->quadShader->setFloat("particleRadius", particleRadius);
        normalsFBO->quadShader->setFloat("nearPlane", SM::camera->nearClipDist);
        normalsFBO->quadShader->setFloat("farPlane", SM::camera->farClipDist);
        normalsFBO->quadShader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        normalsFBO->quadShader->setVec2("stride", vec2(1.f / SM::width, 1.f / SM::height));
        blurFBO->draw(false, 0, false);
        normalsFBO->unbind();
    }

    void compositeTextures() {
        compositionFBO->quadShader->use();
        compositionFBO->quadShader->setFloat("particleRadius", particleRadius);
        compositionFBO->quadShader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        compositionFBO->quadShader->setFloat("nearPlane", SM::camera->nearClipDist);
        compositionFBO->quadShader->setFloat("farPlane", SM::camera->farClipDist);
        compositionFBO->quadShader->setBool("showDiffuse", showDiffuseOnly);
        compositionFBO->quadShader->setVec3("waterCol", waterColour);
        compositionFBO->quadShader->setVec3("attenuationCol", attenuationColour);
        compositionFBO->quadShader->rmv();
    }

    void render() {
        if (ticking && !renderWhileTicking && simulationTick < nextTick) return;

        /* 0: render either basic point sprites or particle velocities */
        if (renderAtStage(SPRITE)) return;
        if (renderAtStage(VELOCITY)) return;

        /* 1: capture depth */
        renderDepth();
        if (renderAtStage(DEPTH)) return;

        /* 2: capture thickness */
        renderThickness();
        if (renderAtStage(THICKNESS)) return;

        /* 3: smoothen depth and thickness textures */
        smoothTextures();
        if (renderAtStage(SMOOTH_DEPTH)) return;
        if (renderAtStage(SMOOTH_THICKNESS)) return;

        /* 4: calculated normals from smoothed depth texture */
        calcNormals();
        if (renderAtStage(NORMAL)) return;

        /* 5: apply diffuse lighting and volume */
        compositeTextures();
        renderAtStage(COMPOSITION);
    }

    bool renderAtStage(FluidRenderStage stage) {
        if (stage != renderStage) return false;
        glClearColor(SM::cfg.bgColour.r, SM::cfg.bgColour.g, SM::cfg.bgColour.b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);  // enable colour blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* draw skybox background */
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        SM::skybox->render();
        glDepthFunc(GL_LESS);

        switch (renderStage) {
            case SPRITE:
                renderSprites(false);
                break;
            case VELOCITY:
                renderSprites(true);
                break;
            case DEPTH:
                depthFBO->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            case THICKNESS:
                thicknessFBO1->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            case SMOOTH_DEPTH:
                blurFBO->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            case SMOOTH_THICKNESS:
                thicknessFBO2->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            case NORMAL:
                normalsFBO->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            case COMPOSITION:
                compositionFBO->draw(true, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                break;
            default:
                return false;
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return true;
    }

    void tick() {
        simulateGPU();
        render();
    }

    int nFluidParticles = 0;
    int nBoundaryParticles = 0;
    int nTotalParticles = 0;
    std::vector<float> densities;
    std::vector<float> lambdas;
    std::vector<vec4> deltas;
    std::vector<vec4> omegas;
    std::vector<vec4> curvatureNormals;
    std::vector<mat4> transforms;

    Shader *spriteShader, *simulationShader, *depthPassShader, *thicknessPassShader;
    Framebuffer *depthFBO, *thicknessFBO1, *thicknessFBO2, *blurFBO, *normalsFBO, *tempBlur1FBO, *tempBlur2FBO, *compositionFBO, *envFBO;
    StaticMesh* mesh;
    unsigned VAO = 0;
    unsigned densitiesBuffer = 0;
    unsigned lambdasBuffer = 0;
    unsigned curvatureNormalsBuffer = 0;
    unsigned deltasBuffer = 0;
    unsigned omegasBuffer = 0;
    unsigned commandBuffer = 0;
    const float thicknessInvScale = 8;  // how much to scale the thickness map down by

    /* Settings */
    // vec3 centre{50, 50, 50};
    // vec3 bounds{35, 35, 35};
    float smoothingRadius = 1.4f;
    float collisionDamping = 1;
    float restDensity = -5e3f;
    float restDensityInv = 1 / restDensity;
    float relaxationEpsilon = 1e-2f;
    float SOR = 1.7;
    float k = 1;  // stiffness
    float f_cohesion = 80.f;
    float f_curvature = 1e-3f;
    float f_viscosity = 1.3f;
    float f_adhesion = 750.f;

    /* Rendering */
    bool showOutline = true;
    bool showLighting = true;
    bool showBounds = true;
    float depthStrength = 150;
    float thicknessStrength = 0.5;
    int renderStage = COMPOSITION;
    int kernelHalfWidth = 3;
    int maxKernelHalfWidth = 11;
    bool showDiffuseOnly = false;
    vec3 waterColour = vec3(0, 0.2, 0.8) / 10.f;
    vec3 attenuationColour = vec3(0.5, 0.2, 0.05);
    float thresholdRatio = .125;
    float clampRatio = .001;
};

}  // namespace PBF
}  // namespace Sim

#endif /* FLUID_H */
