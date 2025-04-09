#ifndef GRID_H
#define GRID_H

#include "util.h"
#include "sm.h"
#include "camera.h"
#include "shader.h"

class Camera;

class Grid {
   public:
    Grid() {
        setupData();
        populateBuffers();
        shader = new Shader("infinite grid", PROJDIR "Shaders/grid.vert", PROJDIR "Shaders/grid.frag");
    }
    ~Grid() {}

    // https://www.youtube.com/watch?v=RqrkVmj-ntM - OGLDEV video for grid
    void setupData() {
        int index = 0;
        indices.resize((width - 1) * (depth - 1) * 6, 0); // number of quads
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                vertices.emplace_back(vec3(x, 0, z), vec2(x, z));
                if (x < width - 1 && z < depth - 1) {
                    int idxBL = z * width + x;
                    int idxBR = z * width + x + 1;
                    int idxTL = (z + 1) * width + x;
                    int idxTR = (z + 1) * width + x + 1;
                    
                    // top left triangle
                    indices[index++] = idxBL;
                    indices[index++] = idxTL;
                    indices[index++] = idxTR;
                    // bottom right triangle
                    indices[index++] = idxBL;
                    indices[index++] = idxTR;
                    indices[index++] = idxBR;
                }
            }
        }
        assert(index == indices.size());
    }

    void populateBuffers() {
        glCreateVertexArrays(1, &VAO);
    }

    // show the grid
    void render() {
        glDisable(GL_CULL_FACE); // view grid from below
        shader->use();
        shader->setVec3("viewPos", SM::camera->pos);
        shader->setMat4("view", SM::camera->getViewMatrix());
        shader->setMat4("proj", SM::camera->getPerspectiveMatrix());
        shader->setVec4("gridColorThin", vec4(SM::cfg.gridThickColour, 1));
        shader->setVec4("gridColorThick", vec4(SM::cfg.gridThinColour, 1));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    struct Vertex {
        vec3 pos;
        vec2 uv;
    };
    Shader* shader;
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    unsigned int VAO = 0;
    unsigned int VBO = 0; // vertices
    unsigned int IBO = 0; // indices
    int width = 20, depth = 20;
    static constexpr int grid_vbo_pos = 0;
    static constexpr int grid_tbo_pos = 1;
};

#endif /* GRID_H */
