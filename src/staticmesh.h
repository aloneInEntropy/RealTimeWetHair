#ifndef STATICMESH_H
#define STATICMESH_H

#pragma warning(disable : 26495)

#include "mesh.h"

#define AI_LOAD_FLAGS aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace

class StaticMesh : public Mesh {
   public:
    // Create a new Mesh object without a mesh
    StaticMesh() { name = "NewStaticMesh" + std::to_string(SM::unnamedStaticMeshCount++); }

    // Create a new unnamed Mesh object
    StaticMesh(std::string mesh_file_name) {
        name = "NewStaticMesh" + std::to_string(SM::unnamedStaticMeshCount++);
        if (!loadMesh(mesh_file_name)) std::cout << "\n\nfailed to load mesh \"" << name.c_str() << "\" :(\n";
    }

    // Create a new Mesh object and give it a name.
    StaticMesh(std::string nm, std::string mesh_file_name) {
        name = nm;
        if (!loadMesh(mesh_file_name)) std::cout << "\n\nfailed to load mesh \"" << nm.c_str() << "\" :(\n";
    }

    // Create a new Mesh object from a atlas texture.
    // The width of the texture must be `_atlasTileSize` and the length must be at least `_atlasTileSize` * `_atlasTilesUsed`
    // If `isVerticalTexture` is `true`, the texture is assumed to be one tile wide and `_atlasTilesUsed` tiles long.
    // If `isVerticalTexture` is `false`, it is assumed the texture only uses tiles of size `_atlasTileSize`.
    StaticMesh(std::string nm, std::string mesh_file_name, int _atlasTileSize, int _atlasTilesUsed, bool load = true) {
        name = nm;
        usingAtlas = true;
        atlasTileSize = _atlasTileSize;
        atlasTilesUsed = _atlasTilesUsed;
        if (load)
            if (!loadMesh(mesh_file_name)) std::cout << "\n\nfailed to load mesh \"" << nm.c_str() << "\" :(\n";
    }

    ~StaticMesh();

    bool loadMesh(std::string mesh_file_name) { return loadMesh(mesh_file_name, true); }
    bool loadMesh(std::string mesh_file_name, bool popBuffer);
    bool initScene(const aiScene*, std::string);
    void initSingleMesh(const aiMesh*);
    bool initMaterials(const aiScene*, std::string);
    void populateBuffers();
    void render(unsigned int, const mat4*);                // render an array of meshes using instancing
    void render(unsigned int, const mat4*, const float*);  // render an array of meshes using instancing and atlas depths
    void render(mat4, float);                              // single atlas depth
    void render(mat4);                                     // render a single mesh
    void render(std::vector<mat4>);                        // render a list of meshes

   private:
    std::vector<mat4> getUpdatedTransforms(Shader* skinnedShader, float animSpeed) { return {}; }  // not implemented
    std::vector<mat4> getUpdatedTransforms(float animSpeed) { return {}; }                         // not implemented
    void update() {}                                                                               // not implemented
    void update(Shader* shader) {}                                                                 // not implemented
    void update(float speed) {}                                                                    // not implemented
    void update(Shader* shader, float speed) {}                                                    // not implemented

    unsigned int tn_VBO = 0;
#define ST_POSITION_LOC 0  // p_vbo
#define ST_NORMAL_LOC 1    // n_vbo
#define ST_TEXTURE_LOC 2   // t_vbo
#define ST_TANGENT_LOC 3  // 
#define ST_INSTANCE_LOC 4
};

#endif /* STATICMESH_H */
