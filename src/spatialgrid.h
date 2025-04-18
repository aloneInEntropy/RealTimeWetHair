#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include "sm.h"
#include "util.h"
#include "shader.h"
#include "common_sim.h"
using namespace CommonSim;

#define GRID_DISPATCH_SIZE 8

// See [Grid]
// Dense uniform grid
class SpatialGrid {
   public:
    struct BucketData {
        int startIndex = 0;
        int particlesInBucket = 0;
        int nextParticleSlot = 0;
        int pd;  // unsure why this needs to be included in the shaders too. usually padding doesn't need to be added
    };

    SpatialGrid() {}
    ~SpatialGrid() {}

    void init() {
        resetGridKernel = new Shader("grid reset", {{DIR("Shaders/sim/grid/resetGrid.comp"), GL_COMPUTE_SHADER},
                                                    {DIR("Shaders/sim/grid/helper.comp"), GL_COMPUTE_SHADER}});
        countKernel = new Shader("grid count", {{DIR("Shaders/sim/grid/count.comp"), GL_COMPUTE_SHADER},
                                                {DIR("Shaders/sim/grid/helper.comp"), GL_COMPUTE_SHADER}});
        allocateKernel = new Shader("grid allocate", {{DIR("Shaders/sim/grid/allocate.comp"), GL_COMPUTE_SHADER},
                                                      {DIR("Shaders/sim/grid/helper.comp"), GL_COMPUTE_SHADER}});
        insertKernel = new Shader("grid insert", {{DIR("Shaders/sim/grid/insert.comp"), GL_COMPUTE_SHADER},
                                                  {DIR("Shaders/sim/grid/helper.comp"), GL_COMPUTE_SHADER}});
        nTotalCells = ps.size() * 3;
        particleStartIndices.resize(nTotalCells);
        cellEntries.resize(ps.size(), 0);
    }

    void populateBuffers() {
        glCreateVertexArrays(1, &VAO);
        auto bf = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;

        glCreateBuffers(1, &particleBuffer);
        glNamedBufferStorage(particleBuffer, sizeof(Particle) * particles.size(), particles.data(), bf);

        glCreateBuffers(1, &predictedPositionBuffer);
        glNamedBufferStorage(predictedPositionBuffer, sizeof(vec4) * ps.size(), ps.data(), bf);

        glCreateBuffers(1, &startIndicesBuffer);
        glNamedBufferStorage(startIndicesBuffer, sizeof(BucketData) * particleStartIndices.size(), particleStartIndices.data(), bf);

        glCreateBuffers(1, &cellEntriesBuffer);
        glNamedBufferStorage(cellEntriesBuffer, sizeof(int) * cellEntries.size(), cellEntries.data(), bf);

        glCreateBuffers(1, &totalParticleCountIndexBuffer);
        glNamedBufferStorage(totalParticleCountIndexBuffer, sizeof(int) * totalParticleCountIndex.size(), totalParticleCountIndex.data(), bf);
    }

    // Organise the grid using compute shaders
    void dispatchKernels() {
        glBindVertexArray(VAO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, predictedPositionBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, startIndicesBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellEntriesBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, totalParticleCountIndexBuffer);

        resetGridKernel->use();
        resetGridKernel->setFloat("gridCellSize", cellSize);
        glDispatchCompute(ceil(particleStartIndices.size() / GRID_DISPATCH_SIZE) + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        countKernel->use();
        countKernel->setFloat("gridCellSize", cellSize);
        glDispatchCompute(ceil(ps.size() / GRID_DISPATCH_SIZE) + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        allocateKernel->use();
        allocateKernel->setFloat("gridCellSize", cellSize);
        glDispatchCompute(ceil(particleStartIndices.size() / GRID_DISPATCH_SIZE) + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        insertKernel->use();
        insertKernel->setFloat("gridCellSize", cellSize);
        glDispatchCompute(ceil(ps.size() / GRID_DISPATCH_SIZE) + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindVertexArray(0);
    }

    int nTotalCells = 0;
    float cellSize = 1;
    std::vector<int> totalParticleCountIndex = {0};
    std::vector<BucketData> particleStartIndices;
    Shader *resetGridKernel;
    Shader *countKernel;
    Shader *allocateKernel;
    Shader *insertKernel;
    unsigned VAO = 0;
    unsigned startIndicesBuffer = 0;
    unsigned cellEntriesBuffer = 0;
    unsigned totalParticleCountIndexBuffer = 0;
    std::vector<int> startIndices;
    std::vector<int> cellEntries;
};

#endif /* SPATIALGRID_H */
