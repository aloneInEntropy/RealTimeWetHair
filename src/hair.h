#ifndef HAIR_H
#define HAIR_H

#include <execution>
#include <deque>
#include <queue>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "camera.h"
#include "sm.h"
#include "util.h"
#include "input.h"
#include "shader.h"
#include "common_sim.h"
#include "spatialgrid.h"

#define USE_GPU

using namespace CommonSim;
namespace Sim {
namespace Rods {

enum HairDispatchPhase {
    APPLY_EXTERNAL_FORCES,
    PREDICT,
    RESOLVE_COLLISIONS,
    STRETCH_SHEAR_CONSTRAINT,
    BEND_TWIST_CONSTRAINT,
    UPDATE_VELOCITIES,
    N_STAGES = 6
};

struct PoreData {
    PoreData(int start, float sstrength, int end, float estrength) : startIndex(start), startStrength(sstrength), endIndex(end), endStrength(estrength) {}
    int startIndex;
    float startStrength;
    int endIndex;
    float endStrength;
    float volume = 0;
    float density = 0;
    int pd1, pd2; // padding
};

struct Rod {
    Rod(quat ori, vec3 a_vel, float invmass) {
        q = ori;
        v = vec4(a_vel, 0);
        w = invmass;
    }
    quat q;   // rod orientation
    vec4 v;   // rod velocity
    float w;  // rod inverse mass
    int pd1, pd2, pd3; // padding
};

struct HairStrand {
    HairStrand(int nv, int vidxs, int vidxe, int nr, int ridxs, int ridxe, float rest_length, vec4 rt) {
        root = rt;
        nVertices = nv;
        startVertexIdx = vidxs;
        endVertexIdx = vidxe;
        nRods = nr;
        startRodIdx = ridxs;
        endRodIdx = ridxe;
        l0 = rest_length;
    }

    vec4 root;
    int nVertices = 0;
    int startVertexIdx = 0;
    int endVertexIdx = 0;
    int nRods = 0;
    int startRodIdx = 0;
    int endRodIdx = 0;
    float l0 = 0;
    int pd = 0;  // padding
};

/* ----- Global variables ----- */

class Hair {
   public:
    Hair(std::vector<HairConfig> configs) {
        shader = new Shader("hair",
                            {
                                {DIR("Shaders/sim/hair/render/hair.vert"), GL_VERTEX_SHADER},
                                {DIR("Shaders/sim/hair/render/hair.frag"), GL_FRAGMENT_SHADER},
                                {DIR("Shaders/sim/hair/render/hair.tesc"), GL_TESS_CONTROL_SHADER},
                                {DIR("Shaders/sim/hair/render/hair.tese"), GL_TESS_EVALUATION_SHADER},
                            });
        // simulationShader = new Shader("hair compute", DIR("Shaders/hair/simulate.comp"), GL_COMPUTE_SHADER);

        numStrands = configs.size();
        int vStart = 0;
        int rStart = 0;
        for (int i = 0; i < numStrands; ++i) {
            auto& [n, root, rotMat] = configs[i];
            int nV = n;
            int nR = n - 1;
            HairStrand hs = HairStrand(
                nV, vStart, vStart + nV - 1,
                nR, rStart, rStart + nR - 1,
                0.05f, root);
            hairStrands.push_back(hs);
            vStart += nV;
            rStart += nR;
            for (int j = 0; j < nV; ++j) {
                vertexStrandMap.push_back(i);
            }
            for (int j = 0; j < nR; ++j) {
                rodStrandMap.push_back(i);
            }
            int prevIndexCount = indices.size();
            initialiseVertices(i, rotMat);
            initialiseQuaternions(i, rotMat);
            indexCounts.push_back(indices.size() - prevIndexCount);
        }
        nTotalVertices = vStart;
        nTotalRods = rStart;
    }
    ~Hair() {
        deleteBuffers();
    }

    void deleteBuffers() {
        glDeleteBuffers(1, &particleBuffer);
        glDeleteBuffers(1, &rodBuffer);
        glDeleteBuffers(1, &hairStrandBuffer);
        glDeleteBuffers(1, &predictedPositionBuffer);
        glDeleteBuffers(1, &predictedRotationBuffer);
        glDeleteBuffers(1, &restDarbouxBuffer);
        glDeleteBuffers(1, &vertexStrandMapBuffer);
        glDeleteBuffers(1, &rodStrandMapBuffer);
        glDeleteBuffers(1, &commandBuffer);
        // glDeleteBuffers(1, &commandBufferA);
        glDeleteBuffers(1, &VAO);
    }

    void populateBuffers() {
        if (buffersSet) {
            deleteBuffers();
            buffersSet = false;
        }
        glCreateVertexArrays(1, &VAO);
        glCreateBuffers(1, &EBO);
        glNamedBufferStorage(EBO, sizeof(indices[0]) * indices.size(), &indices[0], GL_MAP_READ_BIT);
        glVertexArrayElementBuffer(VAO, EBO);
        auto bf = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;

        glCreateBuffers(1, &rodBuffer);
        glNamedBufferStorage(rodBuffer, sizeof(Rod) * rods.size(), rods.data(), bf);

        glCreateBuffers(1, &hairStrandBuffer);
        glNamedBufferStorage(hairStrandBuffer, sizeof(HairStrand) * hairStrands.size(), hairStrands.data(), bf);

        glCreateBuffers(1, &predictedRotationBuffer);
        glNamedBufferStorage(predictedRotationBuffer, sizeof(quat) * us.size(), us.data(), bf);

        glCreateBuffers(1, &restDarbouxBuffer);
        glNamedBufferStorage(restDarbouxBuffer, sizeof(vec4) * d0s.size(), d0s.data(), GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

        glCreateBuffers(1, &vertexStrandMapBuffer);
        glNamedBufferStorage(vertexStrandMapBuffer, sizeof(int) * vertexStrandMap.size(), vertexStrandMap.data(), bf);

        glCreateBuffers(1, &rodStrandMapBuffer);
        glNamedBufferStorage(rodStrandMapBuffer, sizeof(int) * rodStrandMap.size(), rodStrandMap.data(), bf);

        glCreateBuffers(1, &poreDataBuffer);
        glNamedBufferStorage(poreDataBuffer, sizeof(PoreData) * poreData.size(), poreData.data(), bf);

        // load command buffer
        // IndirectArrayDrawCommand* cmdsA = new IndirectArrayDrawCommand[numStrands];
        // unsigned int baseInstance = 0;
        // for (int i = 0; i < numStrands; ++i) {
        //     const auto& s = hairStrands[i];
        //     int vCount = s.nVertices;  // vertices in this strand

        //     cmdsA[i].vertexCount = vCount;
        //     cmdsA[i].instanceCount = 1;  // 1 strand instance
        //     cmdsA[i].baseVertex = s.startVertexIdx;
        //     cmdsA[i].baseInstance = baseInstance;

        //     baseInstance++;
        // }
        // // send command buffers to gpu
        // glCreateBuffers(1, &commandBufferA);
        // glNamedBufferStorage(commandBufferA, sizeof(IndirectArrayDrawCommand) * numStrands, &cmdsA[0], bf);

        // load command buffer
        IndirectElementDrawCommand* cmds = new IndirectElementDrawCommand[numStrands];
        for (int i = 0; i < numStrands; ++i) {
            const auto& s = hairStrands[i];
            cmds[i].indexCount = indexCounts[i];
            cmds[i].instanceCount = 1;  // 1 strand instance
            cmds[i].baseIndex = 0;
            cmds[i].baseVertex = s.startVertexIdx;
            cmds[i].baseInstance = i;
        }
        // send command buffers to gpu
        glCreateBuffers(1, &commandBuffer);
        glNamedBufferStorage(commandBuffer, sizeof(IndirectElementDrawCommand) * numStrands, &cmds[0], bf);

        buffersSet = true;
    }

    // Hair vertices
    void initialiseVertices(int strandIdx, vec3 rot) {
        int vStart = hairStrands[strandIdx].startVertexIdx;
        int nV = hairStrands[strandIdx].nVertices;
        for (int i = 0; i < nV; ++i) {
            /* Generate hair strand vertex */
            float ang = (nCurls * 2 * PI) / nV * i;
            vec3 p = vec3(rad * cos(ang), strandLength - (strandLength / (float)nV) * i, rad * sin(ang));
            // vec3 p = vec3(0, rad * i, 0); // straight hair

            /* Rotate vertex to point in a given direction away from the head */
            vec3 from = Util::UP;        // hairs are generated upwards
            vec3 to = -rot;              // coils grow downwards
            vec3 axs = cross(from, to);  // normal axis from `from` and `to`
            // Deal with rotation direction pointing parallel to default generation order (i.e., up/down)
            float cosTheta = dot(from, to);
            if (cosTheta < -1 + 0.001f || length2(axs) < MIN_FLOAT_DIFF) {
                axs = cross(vec3(1.0f, 0.0f, 0.0f), from);
                if (length2(axs) < 0.01)
                    axs = cross(vec3(0.0f, 0.0f, 1.0f), from);
            }
            axs = normalize(axs);
            float rotAngle = acos(cosTheta);    // angle between `from` and `to`
            quat q = angleAxis(rotAngle, axs);  // rotation quaternion
            p = rotate(q, p);                   // rotate `from` about `axs` with `rotAngle` radians to point it towards `to`

            /* Add vertex/particle to this hair strand */
            Particle part = Particle(p, vec3(0), i == 0 ? 0 : 1, HAIR);
            particles.push_back(part);
            ps.push_back(vec4(p, 0));

            /* Add vertex indices */
            if (i <= 3) {
                indices.push_back(i);
            } else {
                indices.push_back(i - 3);
                indices.push_back(i - 2);
                indices.push_back(i - 1);
                indices.push_back(i);
            }
        }

        float l0 = distance(particles[vStart].x, particles[vStart + 1].x);
        /* Debug */
        for (int i = vStart; i < vStart + nV - 1; ++i) {
            float dd = distance(particles[i].x, particles[i + 1].x);
            // printf("%.10f, %.10f, %.10f\n", dd, l0, dd - l0); // debug
            assert(abs(dd - l0) <= 1e-4f && "vertex distances don't match up!");
        }
        hairStrands[strandIdx].l0 = l0;

        // move root to start position and offset all vertices by that distance (including the head's transform)
        vec4 offs = hairStrands[strandIdx].root - particles[vStart].x + headTrans[3];
        for (int i = vStart; i < vStart + nV; ++i) {
            particles[i].x += offs;
            ps[i] += offs;
        }
    }

    // Hair quaternions
    void initialiseQuaternions(int strandIdx, vec3 rot) {
        int vStart = hairStrands[strandIdx].startVertexIdx;
        int rStart = hairStrands[strandIdx].startRodIdx;
        int nR = hairStrands[strandIdx].nRods;
        vec3 from = e3;
        for (int j = 0; j < nR; ++j) {
            vec3 to = vec3(particles[vStart + j + 1].x - particles[vStart + j].x);
            quat q = quatFromVectors(from, to);
            if (j != 0) q *= rods[rStart + j - 1].q;  // each quaternion rotates depending on the previous one
            Rod rod = Rod(q, vec3(0), j == 0 ? 0 : 1);
            rods.push_back(rod);
            us.push_back(q);
            from = to;
        }
        for (int i = 0; i < nR - 1; ++i) {
            d0s.push_back(vec4(darboux(rStart + i), 0));
        }

        // todo: make per strand (only if strands can have different lengths. otherwise, make global)
        float factor = (1 / 12.f) * (hairStrands[0].l0 * hairStrands[0].l0);
        vec3 tensor = vec3(factor);
        inertia = diagonal3x3(tensor);
    }

    void samplePorousParticles() {
        assert(fluidLoaded && "fluid not loaded");
        for (int s = 0; s < numStrands; ++s) {
            for (int i = hairStrands[s].startVertexIdx; i < hairStrands[s].startVertexIdx + hairStrands[s].nVertices - 1; ++i) {
                vec3 a = vec3(particles[i].x);
                vec3 b = vec3(particles[i + 1].x);
                vec3 d = b - a;
                for (int j = 0; j < poreSamples; ++j) {
                    float strength = (j + 1.f) / (poreSamples + 1.f);
                    vec3 smpl = a + d * strength;
                    Particle p = Particle(smpl, vec3(0), 1.f, PORE);
                    particles.push_back(p);
                    ps.push_back(vec4(smpl, 0));
                    PoreData pData = PoreData(i, 1 - strength, i + 1, strength);
                    poreData.push_back(pData);
                }
            }
        }
    }

    /* Get the quaternion representing a rotation from `start` to `dest`.
     * From OpenGL Tutorial
     * https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp#L11
     */
    quat quatFromVectors(vec3 start, vec3 dest) {
        start = normalize(start);
        dest = normalize(dest);
        float cosTheta = dot(start, dest);
        vec3 rotationAxis;
        if (cosTheta < -1 + 0.001f) {
            // special case when vectors in opposite directions:
            // there is no "ideal" rotation axis
            // So guess one; any will do as long as it's perpendicular to start
            rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);
            if (length2(rotationAxis) < 0.01)  // bad luck, they were parallel, try again!
                rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);

            rotationAxis = normalize(rotationAxis);
            return angleAxis(radians(180.0f), rotationAxis);
        }

        rotationAxis = cross(start, dest);
        float s = sqrt((1 + cosTheta) * 2);
        float invs = 1 / s;
        return quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs);
    }

    // Render the simulation
    void render() {
        glBindVertexArray(VAO);
        shader->use();

        bindKernels();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);  // rebind command buffer

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        float heightOfNearPlane = (float)abs(viewport[3] - viewport[1]) / (2 * tan(Util::rad(0.5 * SM::camera->FOV)));

        shader->setMat4("view", SM::camera->getViewMatrix());
        shader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        shader->setVec3("viewPos", SM::camera->pos);
        shader->setFloat("particleRadius", particleRadius);
        shader->setFloat("nearPlaneHeight", heightOfNearPlane);
        shader->setVec4("colour", hairColour);
        shader->setVec3("pbrMaterial.albedo", vec3(hairColour));
        shader->setFloat("pbrMaterial.metalness", metalness);
        shader->setFloat("pbrMaterial.roughness", roughness);
        shader->setFloat("fresnelPower", fresnelExponent);
        shader->setVec3("headPos", vec3(headTrans[3]));
        glPatchParameteri(GL_PATCH_VERTICES, 4); // catmull-rom splines use at 4 points: 2 controls, 2 line points
        glMultiDrawElementsIndirect(GL_PATCHES, GL_UNSIGNED_INT, 0, numStrands, 0);

        glBindVertexArray(0);
    }

    void bindKernels() {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, rodBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, hairStrandBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, predictedPositionBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, predictedRotationBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, restDarbouxBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, vertexStrandMapBuffer);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, rodStrandMapBuffer);
    }

    /* --- Maths functions --- */
    // discrete darboux sign factor
    float s(int j) {
        float pos = length2(darboux(j) + vec3(d0s[j - rodStrandMap[j]]));
        float neg = length2(darboux(j) - vec3(d0s[j - rodStrandMap[j]]));
        if (neg < pos) return 1;
        return -1;
    }
    vec3 darboux(int j) { return Im(qmul(conjugate((us[j])), (us[j + 1]))); }
    vec3 Im(quat q) { return vec3(q.x, q.y, q.z); }
    // Quaternion multiplication
    quat qmul(quat p, quat q) {
        quat qo;
        qo.w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
        qo.x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
        qo.y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
        qo.z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
        return qo;
    }

    int numStrands = 0;        // number of total strands
    int numGuideStrands = 0;   // number of guide strands
    int numRenderStrands = 0;  // number of render strands
    std::vector<HairStrand> hairStrands;
    std::vector<int> vertexStrandMap;  // mapping of vertex index to hairStrand index. e.g., [0, 0, 0, 0, 1, 1, 1, 1, ...]
    std::vector<int> rodStrandMap;     // mapping of rod index to hairStrand index. e.g., [0, 0, 0, 1, 1, 1, ...]
    Shader* shader;
    Shader* simulationShader;

    /* --- Other --- */
    float renderHeadRadius = 25;

    /* --- Physics buffers --- */
    vec3 torque{0};      // hair torque
    mat3 inertia;        // inertia matrix
    vec3 e3 = Util::UP;  // global up (0, 1, 0)

    /* Vertices */
    int nTotalVertices;  // number of vertices
    std::vector<int> indices;      // indices for each vertex
    std::vector<int> indexCounts;  // index counts for each strand
    
    /* Pores */
    std::vector<PoreData> poreData;
    
    /* Rods */
    std::vector<Rod> rods;
    std::vector<quat> us;   // predicted rod orientations
    std::vector<vec4> d0s;  // rest darboux vectors
    int nTotalRods;         // number of quaternions/rods
    
    /* --- Rendering buffers --- */
    unsigned VAO = 0;
    unsigned EBO = 0;
    unsigned rodBuffer = 0;  // holds Rod struct
    unsigned predictedRotationBuffer = 0;
    unsigned restDarbouxBuffer = 0;
    unsigned vertexStrandMapBuffer = 0;
    unsigned rodStrandMapBuffer = 0;
    unsigned hairStrandBuffer = 0;
    unsigned poreDataBuffer = 0;
    // unsigned commandBufferA = 0;
    unsigned commandBuffer = 0;
    bool buffersSet = false;

    /* Constraints and Stiffnesses */
    float ss_k = 1;        // stretch and shear stiffness coefficient
    float bt_k = 1;        // bend and twist stiffness coefficient
    float ss_SOR = 1.36f;  // see [UPP14]
    float bt_SOR = 3.f;    // see [UPP14]

    /* Physics */
    float f_l_drag = 0.98f;  // vertex air resistance
    float f_a_drag = 0.6f;   // rod rotation resistance
    float f_porosity = .25;
    float f_clumping = -0.0002;
    int clumpingRange = 1;

    /* Hair */
    mat4 headTrans = translate(mat4(1), vec3(150, 6, 150));
    vec4 hairColour = vec4(42, 25, 5, 255) / 255.f;
    float metalness = 0.1;
    float roughness = 0.9;
    float fresnelExponent = 5;
    float sRad = 0.05;       // rod thickness
    float rad = .5f;         // strand coil radius
    float strandLength = 5;  // length of a strand
    int nCurls = 3;          // number of curls in a strand
    int poreSamples = 1;

    /* ----- Settings ----- */
    bool ssC = true;
    bool btC = true;
    bool showLines = true;
    bool showPoints = true;
};
}  // namespace Rods
}  // namespace Sim

#endif /* HAIR_H */
