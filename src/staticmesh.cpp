#include "staticmesh.h"

StaticMesh::~StaticMesh() {}

/// <summary>
/// Load a mesh with a given name.
/// </summary>
/// <param name="file_name">The full name of the model to load.</param>
/// <returns>A boolean. True if loading succeeds, false otherwise.</returns>
bool StaticMesh::loadMesh(std::string mesh_file_name, bool popBuffers) {
    populateBuffer = popBuffers;

    std::string rpath = MODELPATH(mesh_file_name) + mesh_file_name;
    scene = aiImportFile(
        rpath.c_str(),
        AI_LOAD_FLAGS);

    bool valid_scene = false;

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n%s", rpath.c_str(), aiGetErrorString());
        valid_scene = false;
    } else {
        valid_scene = initScene(scene, mesh_file_name);
    }

    glBindVertexArray(0);  // avoid modifying VAO between loads

    if (valid_scene) printf("Successfully loaded %sstatic mesh \"%s\"\n", popBuffers ? "" : "variant ", name.c_str());
    return valid_scene;
}

bool StaticMesh::initScene(const aiScene* scene, std::string file_name) {
    meshes.resize(scene->mNumMeshes);
    materials.resize(scene->mNumMaterials);

    unsigned int nvertices = 0;
    unsigned int nindices = 0;

    // Count all vertices and indices
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;  // get current material index
        meshes[i].n_Indices = scene->mMeshes[i]->mNumFaces * 3;       // 3 times as many indices as there are faces
        meshes[i].baseVertex = nvertices;                             // index of first vertex in the current mesh
        meshes[i].baseIndex = nindices;                               // track number of indices

        // Move forward by the corresponding number of vertices/indices to find the base of the next vertex/index
        nvertices += scene->mMeshes[i]->mNumVertices;
        nindices += meshes[i].n_Indices;
    }

    // Reallocate space for structure of arrays (SOA) values
    vertexData.reserve(nvertices);
    indices.reserve(nindices);

    // Initialise meshes
    for (unsigned int i = 0; i < meshes.size(); i++) {
        const aiMesh* am = scene->mMeshes[i];
        initSingleMesh(am);
    }

    if (!initMaterials(scene, file_name)) {
        return false;
    }

    if (populateBuffer) populateBuffers();
    return glGetError() == GL_NO_ERROR;
}

// Initialise a single mesh object and add its values (vertices, indices, positions, normals, and texture coordinates) to the parent mesh.
void StaticMesh::initSingleMesh(const aiMesh* amesh) {
    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < amesh->mNumVertices; i++) {
        const aiVector3D& pPos = amesh->mVertices[i];
        const aiVector3D& pNormal = amesh->mNormals ? amesh->mNormals[i] : aiVector3D(0.0f, 1.0f, 0.0f);
        const aiVector3D& pTexCoord = amesh->HasTextureCoords(0) ? amesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
        const aiVector3D& pTangents = amesh->mTangents ? amesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);
        const aiVector3D& pBitangents = amesh->mBitangents ? amesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);

        vertexData.emplace_back(vec3(pPos.x, pPos.y, pPos.z),
                                vec2(pTexCoord.x, pTexCoord.y),
                                vec3(pNormal.x, pNormal.y, pNormal.z),
                                vec3(pTangents.x, pTangents.y, pTangents.z));
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < amesh->mNumFaces; i++) {
        const aiFace& Face = amesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }
}

/// Initialise the materials and textures used in the mesh.
bool StaticMesh::initMaterials(const aiScene* scene, std::string model_file_name) {
    std::string dir = MODELPATH(model_file_name);
    std::vector<std::string> paths;
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = scene->mMaterials[i];

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* embeddedTex = scene->GetEmbeddedTexture(Path.C_Str());
                if (embeddedTex) {
                    materials[i].diffTex = new Texture(GL_TEXTURE_2D);
                    unsigned int buffer = embeddedTex->mWidth;
                    materials[i].diffTex->load(buffer, embeddedTex->pcData);
                    printf("%s: embedded diffuse texture type %s\n", name.c_str(), embeddedTex->achFormatHint);
                } else {
                    std::string p(Path.data);
                    // std::cout << p << std::endl;
                    if (p.substr(0, 2) == ".\\") {
                        p = p.substr(2, p.size() - 2);
                    }
                    std::string fullPath = dir + p;
                    materials[i].diffTex = new Texture(GL_TEXTURE_2D);
                    if (materials[i].diffTex->load(fullPath)) {
                        // printf("%s: Loaded diffuse texture '%s'\n", name.c_str(), fullPath.c_str());
                    } else {
                        printf("Error loading diffuse texture '%s'\n", fullPath.c_str());
                        return false;
                    }
                }
            }
        }

        if (pMaterial->GetTextureCount(aiTextureType_METALNESS) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* embeddedTex = scene->GetEmbeddedTexture(Path.C_Str());
                if (embeddedTex) {
                    materials[i].mtlsTex = new Texture(GL_TEXTURE_2D);
                    unsigned int buffer = embeddedTex->mWidth;
                    materials[i].mtlsTex->load(buffer, embeddedTex->pcData);
                    printf("%s: embedded metalness texture type %s\n", name.c_str(), embeddedTex->achFormatHint);
                } else {
                    std::string p(Path.data);
                    // std::cout << p << std::endl;
                    if (p.substr(0, 2) == ".\\") {
                        p = p.substr(2, p.size() - 2);
                    }
                    std::string fullPath = dir + p;
                    materials[i].mtlsTex = new Texture(GL_TEXTURE_2D);
                    if (materials[i].mtlsTex->load(fullPath)) {
                        // printf("%s: Loaded metalness texture '%s'\n", name.c_str(), fullPath.c_str());
                    } else {
                        printf("Error loading metalness texture '%s'\n", fullPath.c_str());
                        return false;
                    }
                }
            }
        }

        if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* embeddedTex = scene->GetEmbeddedTexture(Path.C_Str());
                if (embeddedTex) {
                    materials[i].normTex = new Texture(GL_TEXTURE_2D);
                    unsigned int buffer = embeddedTex->mWidth;
                    materials[i].normTex->load(buffer, embeddedTex->pcData);
                    printf("%s: embedded normal texture type %s\n", name.c_str(), embeddedTex->achFormatHint);
                } else {
                    std::string p(Path.data);
                    // std::cout << p << std::endl;
                    if (p.substr(0, 2) == ".\\") {
                        p = p.substr(2, p.size() - 2);
                    }
                    std::string fullPath = dir + p;
                    materials[i].normTex = new Texture(GL_TEXTURE_2D);
                    if (materials[i].normTex->load(fullPath)) {
                        // printf("%s: Loaded normal texture '%s'\n", name.c_str(), fullPath.c_str());
                    } else {
                        printf("Error loading normal texture '%s'\n", fullPath.c_str());
                        return false;
                    }
                }
            }
        }
    }

    return glGetError() == GL_NO_ERROR;
}

/// <summary>
/// Bind and enable the VAO, VBOs, and EBO for usage.
/// </summary>
void StaticMesh::populateBuffers() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);
    glCreateBuffers(1, &IBO);

    glNamedBufferStorage(VBO, sizeof(vertexData[0]) * vertexData.size(), &vertexData[0], GL_MAP_READ_BIT);
    glVertexArrayVertexBuffer(VAO, ST_POSITION_LOC, VBO, 0, sizeof(Vertex));
    glNamedBufferStorage(EBO, sizeof(indices[0]) * indices.size(), &indices[0], GL_MAP_READ_BIT);
    glVertexArrayElementBuffer(VAO, EBO);

    glEnableVertexArrayAttrib(VAO, ST_POSITION_LOC);
    glVertexArrayAttribFormat(VAO, ST_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(VAO, ST_POSITION_LOC, 0);

    glEnableVertexArrayAttrib(VAO, ST_TEXTURE_LOC);
    glVertexArrayAttribFormat(VAO, ST_TEXTURE_LOC, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(VAO, ST_TEXTURE_LOC, 0);

    glEnableVertexArrayAttrib(VAO, ST_NORMAL_LOC);
    glVertexArrayAttribFormat(VAO, ST_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, norm));
    glVertexArrayAttribBinding(VAO, ST_NORMAL_LOC, 0);

    glEnableVertexArrayAttrib(VAO, ST_TANGENT_LOC);
    glVertexArrayAttribFormat(VAO, ST_TANGENT_LOC, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
    glVertexArrayAttribBinding(VAO, ST_TANGENT_LOC, 0);

    glNamedBufferStorage(IBO, sizeof(mat4) * SM::MAX_NUM_INSTANCES, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexArrayAttrib(VAO, ST_INSTANCE_LOC + i);
        glVertexArrayAttribFormat(VAO, ST_INSTANCE_LOC + i, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayVertexBuffer(VAO, ST_INSTANCE_LOC + i, IBO, sizeof(vec4) * i, sizeof(mat4));
        glVertexArrayBindingDivisor(VAO, ST_INSTANCE_LOC + i, 1);  // tell OpenGL this is an instanced vertex attribute.
    }
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This function supports instancing and atlas coordinates.
/// </summary>
/// <param name="nInstances">The number of instances you would like to draw.</param>
/// <param name="model_matrix">The matrices you would like to transform each instance with.</param>
void StaticMesh::render(unsigned int nInstances, const mat4* model_matrix, const float* atlasDepths) {
    if (nInstances == 0) return;
    glBindVertexArray(VAO);
    glNamedBufferSubData(IBO, 0, sizeof(mat4) * nInstances, &model_matrix[0]);
    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int mIndex = meshes[i].materialIndex;
        assert(mIndex < materials.size());

        // glBindBuffer(GL_ARRAY_BUFFER, d_VBO);
        // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * nInstances, &atlasDepths[0]);
        if (materials[mIndex].diffTex) materials[mIndex].diffTex->bind(GL_TEXTURE0);
        if (materials[mIndex].mtlsTex) materials[mIndex].mtlsTex->bind(GL_TEXTURE1);
        if (materials[mIndex].normTex) materials[mIndex].normTex->bind(GL_TEXTURE2);

        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            meshes[i].n_Indices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].baseIndex),
            nInstances,
            meshes[i].baseVertex);

        // unbind textures so they don't "spill over"
        if (materials[mIndex].diffTex) materials[mIndex].diffTex->unbind(GL_TEXTURE0);
        if (materials[mIndex].mtlsTex) materials[mIndex].mtlsTex->unbind(GL_TEXTURE1);
        if (materials[mIndex].normTex) materials[mIndex].normTex->unbind(GL_TEXTURE2);
    }
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This function supports instancing.
/// </summary>
/// <param name="nInstances">The number of instances you would like to draw.</param>
/// <param name="model_matrix">The matrices you would like to transform each instance with.</param>
void StaticMesh::render(unsigned int nInstances, const mat4* model_matrix) {
    std::vector<float> dpths(nInstances, 0);
    render(nInstances, model_matrix, dpths.data());
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This is a special case that will render exactly one instance of your mesh.
/// </summary>
/// <param name="mat">The transform you would like to apply to your instance.</param>
void StaticMesh::render(mat4 mat, float depth) {
    this->mat = mat;
    const float ds[1] = {depth};
    render(1, &mat, ds);
}

/// <summary>
/// Render the mesh by binding its VAO and drawing each index of every submesh. This is a special case that will render exactly one instance of your mesh.
/// </summary>
/// <param name="mat">The transform you would like to apply to your instance.</param>
void StaticMesh::render(mat4 mat) {
    render(mat, 0);
}

void StaticMesh::render(std::vector<mat4> ms) {
    render(ms.size(), ms.data(), nullptr);
}
