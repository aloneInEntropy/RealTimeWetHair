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

    // Initialise the grid
    void initGrid() {
        nTotalCells = ps.size() * 3;
        startIndices.clear();
        startIndices.resize(nTotalCells + 1, 0);
        cellEntries.clear();
        cellEntries.resize(ps.size(), 0);
        loadGrid();
    }

    // Load the grid
    void loadGrid() {
        // add to startIndices
        for (int i = 0; i < ps.size(); ++i) {
            vec3 p = ps[i];
            ivec3 cell = posToCell(p);
            unsigned key = flatten(cell);
            startIndices[key]++;
        }

        // compute inclusive scan (partial/prefix sum)
        for (int i = 1; i < nTotalCells; ++i) {
            startIndices[i] += startIndices[i - 1];
        }

        // compute cell entries
        for (int i = 0; i < ps.size(); ++i) {
            vec3 p = ps[i];
            ivec3 cell = posToCell(p);
            unsigned key = flatten(cell);
            startIndices[key]--;
            cellEntries[startIndices[key]] = i;
        }
    }

    // Get the grid cell containing point `p`. Will crash if `p` is outside the grid.
    ivec3 posToCell(vec3 p) {
        return ivec3(floor(p.x / cellSize), floor(p.y / cellSize), floor(p.z / cellSize));
    }

    // Convert a grid cell `cell` to a flattened version that can be used to access `startIndices`.
    unsigned flatten(ivec3 cell) {
        unsigned tmpx = (cell.x * 78455519);
        unsigned tmpy = (cell.y * 41397959);
        unsigned tmpz = (cell.z * 27614441);
        return abs(tmpx ^ tmpy ^ tmpz) % nTotalCells;
    }

    // Is the grid cell `cell` empty?
    bool isCellEmpty(ivec3 cell) {
        unsigned key = flatten(cell);
        int startIdx = startIndices[key];
        int endIdx = startIndices[key + 1];
        return startIdx == endIdx;
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
    /*
        A map of the start indices of the list of particles in a given grid cell.
```
key = flatten(posToCell(position));
start = startIndices[key]; // start index
end = startIndices[key + 1]; // end index
for (int i = start; i < end; ++i) {
    int idx = cellEntries[i];
    vec3 pos = particles[idx].x;
    // and so on...
}
```
    **/
    std::vector<int> startIndices;
    /*
        A map of flattened grid indices to particle indices. It is accessed like so:
```
key = flatten(posToCell(position));
start = startIndices[key]; // start index
end = startIndices[key + 1]; // end index
for (int i = start; i < end; ++i) {
    int idx = cellEntries[i];
    vec3 pos = particles[idx].x;
    // and so on...
}
```
    **/
    std::vector<int> cellEntries;
};

#endif /* SPATIALGRID_H */
