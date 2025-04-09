#include "lighting.h"

void Lighting::use() {
    switch (material) {
        case MATERIAL_STONE:
            loadStoneMaterial();
            break;
        case MATERIAL_PLASTIC:
            loadPlasticMaterial();
            break;
        case MATERIAL_WOOD:
            loadWoodMaterial();
            break;
        case MATERIAL_RUBBER:
            loadRubberMaterial();
            break;
        case MATERIAL_METAL:
            loadMetalMaterial();
            break;
        case MATERIAL_SHINY:
            loadShinyMaterial();
            break;
        case MATERIAL_CUSTOM:
            loadCustomMaterial(materialShininess);
            break;
        default:
            printf("invalid material for light");
            break;
    }
}

void Lighting::loadMaterial() {
    shader->use();
    shader->setFloat("material.shininess", materialShininess);
    // shader->setInt("material.diffuse", 0);
    // shader->setInt("material.specular", 1);
    shader->setInt("nDirLights", nDirLights);
    shader->setInt("nSpotLights", nSpotLights);
    shader->setInt("nPointLights", nPointLights);

    for (int i = 0; i < nDirLights; i++) {
        shader->setVec3("dirLights[" + std::to_string(i) + "].direction", dirLights[i].direction);
        shader->setVec3("dirLights[" + std::to_string(i) + "].ambient", dirLights[i].ambient);
        shader->setVec3("dirLights[" + std::to_string(i) + "].diffuse", dirLights[i].diffuse);
        shader->setVec3("dirLights[" + std::to_string(i) + "].specular", dirLights[i].specular);
    }

    for (int i = 0; i < nPointLights; i++) {
        shader->setVec3("pointLights[" + std::to_string(i) + "].position", pointLights[i].position);
        shader->setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        shader->setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        shader->setVec3("pointLights[" + std::to_string(i) + "].specular", pointLights[i].specular);
        shader->setFloat("pointLights[" + std::to_string(i) + "].constant", pointLights[i].constant);
        shader->setFloat("pointLights[" + std::to_string(i) + "].linear", pointLights[i].linear);
        shader->setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLights[i].quadratic);
    }

    for (int i = 0; i < nSpotLights; i++) {
        shader->setVec3("spotLights[" + std::to_string(i) + "].position", spotLights[i].position);
        shader->setVec3("spotLights[" + std::to_string(i) + "].direction", spotLights[i].direction);
        shader->setVec3("spotLights[" + std::to_string(i) + "].ambient", spotLights[i].ambient);
        shader->setVec3("spotLights[" + std::to_string(i) + "].diffuse", spotLights[i].diffuse);
        shader->setVec3("spotLights[" + std::to_string(i) + "].specular", spotLights[i].specular);
        shader->setFloat("spotLights[" + std::to_string(i) + "].constant", spotLights[i].constant);
        shader->setFloat("spotLights[" + std::to_string(i) + "].linear", spotLights[i].linear);
        shader->setFloat("spotLights[" + std::to_string(i) + "].quadratic", spotLights[i].quadratic);
        shader->setFloat("spotLights[" + std::to_string(i) + "].cutOff", spotLights[i].cutOff);
        shader->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", spotLights[i].outerCutOff);
    }
}

void Lighting::loadStoneMaterial() {
    material = MATERIAL_STONE;
    materialShininess = 20.f;
    loadMaterial();
}

void Lighting::loadMetalMaterial() {
    material = MATERIAL_METAL;
    materialShininess = 400.f;
    loadMaterial();
}

void Lighting::loadWoodMaterial() {
    material = MATERIAL_WOOD;
    materialShininess = 80.f;
    loadMaterial();
}

void Lighting::loadRubberMaterial() {
    material = MATERIAL_RUBBER;
    materialShininess = 4.f;
    loadMaterial();
}

void Lighting::loadPlasticMaterial() {
    material = MATERIAL_PLASTIC;
    materialShininess = 200.f;
    loadMaterial();
}

void Lighting::loadShinyMaterial() {
    material = MATERIAL_SHINY;
    materialShininess = 850.f;
    loadMaterial();
}

void Lighting::loadCustomMaterial(float s) {
    material = MATERIAL_CUSTOM;
    materialShininess = s;
    loadMaterial();
}

void Lighting::setLightAtt(mat4 view, mat4 projection, vec3 vPos) {
    shader->use();
    shader->setVec3("viewPos", vPos);
    shader->setMat4("view", view);
    shader->setMat4("proj", projection);
}

void Lighting::addSpotLightAtt(vec3 pos, vec3 dir, vec3 amb, vec3 dif, vec3 spec) {
    if (nSpotLights < MAX_LIGHT_COUNT) {
        spotLights[nSpotLights].position = pos;
        spotLights[nSpotLights].direction = dir;
        spotLights[nSpotLights].ambient = amb;
        spotLights[nSpotLights].diffuse = dif;
        spotLights[nSpotLights].specular = spec;
        nSpotLights++;
    }
}

void Lighting::setSpotLightAtt(int idx, vec3 pos, vec3 dir, vec3 amb, vec3 dif, vec3 spec) {
    if (idx < nSpotLights) {
        spotLights[idx].position = pos;
        spotLights[idx].direction = dir;
        spotLights[idx].ambient = amb;
        spotLights[idx].diffuse = dif;
        spotLights[idx].specular = spec;
    }
}

void Lighting::setSpotLightAtt(int idx, vec3 pos, vec3 dir) {
    if (idx < nSpotLights) {
        spotLights[idx].position = pos;
        spotLights[idx].direction = dir;
    }
}

void Lighting::addPointLightAtt(vec3 pos, vec3 amb, vec3 dif, vec3 spec) {
    if (nPointLights < MAX_LIGHT_COUNT) {
        pointLights[nPointLights].position = pos;
        pointLights[nPointLights].ambient = amb;
        pointLights[nPointLights].diffuse = dif;
        pointLights[nPointLights].specular = spec;
        nPointLights++;
    }
}

void Lighting::setPointLightAtt(int idx, vec3 pos, vec3 amb, vec3 dif, vec3 spec) {
    if (idx < nPointLights) {
        pointLights[idx].position = pos;
        pointLights[idx].ambient = amb;
        pointLights[idx].diffuse = dif;
        pointLights[idx].specular = spec;
    }
}

void Lighting::setPointLightAtt(int idx, vec3 pos) {
    if (idx < nPointLights) {
        pointLights[idx].position = pos;
    }
}

void Lighting::addDirLightAtt(vec3 dir, vec3 amb, vec3 dif, vec3 spec) {
    if (nDirLights < MAX_LIGHT_COUNT) {
        dirLights[nDirLights].direction = dir;
        dirLights[nDirLights].ambient = amb;
        dirLights[nDirLights].diffuse = dif;
        dirLights[nDirLights].specular = spec;
        nDirLights++;
    }
}

void Lighting::setDirLightAtt(int idx, vec3 dir, vec3 amb, vec3 dif, vec3 spec) {
    if (idx < nDirLights) {
        dirLights[idx].direction = dir;
        dirLights[idx].ambient = amb;
        dirLights[idx].diffuse = dif;
        dirLights[idx].specular = spec;
    }
}

void Lighting::setDirLightAtt(int idx, vec3 dir) {
    if (idx < nDirLights) {
        dirLights[idx].direction = dir;
    }
}
