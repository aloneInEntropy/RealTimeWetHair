#ifndef LIGHTING_H
#define LIGHTING_H

#include "shader.h"
#include "util.h"

#define MAX_LIGHT_COUNT 100

enum SpecularPreset {
    /// <summary>
    /// A heavily non-specular material. shininess = 1.0f
    /// </summary>
    MATERIAL_RUBBER,

    /// <summary>
    /// A rough non-specular material. shininess = 5.0f
    /// </summary>
    MATERIAL_STONE,

    /// <summary>
    /// A flat, lower-specular texture. shininess = 20.0f
    /// </summary>
    MATERIAL_WOOD,

    /// <summary>
    /// A moderately specular material. shininess = 50.0f
    /// </summary>
    MATERIAL_PLASTIC,

    /// <summary>
    /// A metallic, highly specular material. shininess = 100.0f
    /// </summary>
    MATERIAL_METAL,

    /// <summary>
    /// An extremely reflective material. shininess = 850.0f
    /// </summary>
    MATERIAL_SHINY,
    
    // A custom material. Not preset
    MATERIAL_CUSTOM,
};

class Lighting {
   public:
    struct Material {
        int diffuse;
        int specular;
        float shininess;
    };

    struct DirLight {
        vec3 direction;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    struct PointLight {
        vec3 position;

        float constant = 1.f;
        float linear = 0.09f;
        float quadratic = 0.032f;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    struct SpotLight {
        vec3 position;
        vec3 direction;
        float cutOff = cos(Util::rad(12.5f));
        float outerCutOff = cos(Util::rad(15.5f));

        float constant = 1.f;
        float linear = 0.09f;
        float quadratic = 0.032f;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    std::string name;
    Shader* shader;
    SpecularPreset material;
    int nPointLights = 0;
    int nSpotLights = 0;
    int nDirLights = 0;
    float materialShininess = 15;

    DirLight dirLights[MAX_LIGHT_COUNT];
    PointLight pointLights[MAX_LIGHT_COUNT];
    SpotLight spotLights[MAX_LIGHT_COUNT];

    Lighting() {}
    Lighting(std::string nm, Shader* s, SpecularPreset mp) {
        name = nm;
        shader = s;
        material = mp;
    }

    /// <summary>
    /// Set the view matrix, projection matrix, and view position of the light
    /// </summary>
    /// <param name="view">View matrix</param>
    /// <param name="projection">Projection matrix</param>
    /// <param name="vPos">View position</param>
    void setLightAtt(mat4 view, mat4 projection, vec3 vPos);

    /// <summary>
    /// Add a single spot light
    /// </summary>
    /// <param name="pos">The spot light position</param>
    /// <param name="dir">The spot light direction</param>
    /// <param name="amb">The ambient light colour</param>
    /// <param name="dif">The diffuse light colour</param>
    /// <param name="spe">The specular light colour</param>
    void addSpotLightAtt(vec3 pos, vec3 dir, vec3 amb, vec3 dif, vec3 spec);
    void setSpotLightAtt(int idx, vec3 pos, vec3 dir, vec3 amb, vec3 dif, vec3 spec);
    void setSpotLightAtt(int idx, vec3 pos, vec3 dir);
    void addSpotLightAtt(SpotLight sl) {
        if (nSpotLights < MAX_LIGHT_COUNT) spotLights[nSpotLights++] = sl;
    }
    void setSpotLightAtt(int idx, SpotLight sl) {
        if (idx < nSpotLights) spotLights[idx] = sl;
    }

    /// <summary>
    /// Add a single point light
    /// </summary>
    /// <param name="pos">The point light position</param>
    /// <param name="amb">The ambient light colour</param>
    /// <param name="dif">The diffuse light colour</param>
    /// <param name="spe">The specular light colour</param>
    void addPointLightAtt(vec3 pos, vec3 amb, vec3 dif, vec3 spec);
    void addPointLightAtt(PointLight pl) {
        if (nPointLights < MAX_LIGHT_COUNT) pointLights[nPointLights++] = pl;
    }
    void setPointLightAtt(int idx, vec3 pos, vec3 amb, vec3 dif, vec3 spec);
    void setPointLightAtt(int idx, vec3 pos);
    void setPointLightAtt(int idx, PointLight pl) {
        if (idx < nPointLights) pointLights[idx] = pl;
    }

    /// <summary>
    /// Set the directions of the directional lights
    /// </summary>
    /// <param name="dir">The directional light directions</param>
    /// <param name="amb">The ambient light colour</param>
    /// <param name="dif">The diffuse light colour</param>
    /// <param name="spe">The specular light colour</param>
    void addDirLightAtt(vec3 dir, vec3 amb, vec3 dif, vec3 spec);
    void setDirLightAtt(int idx, vec3 dir, vec3 amb, vec3 dif, vec3 spec);
    void setDirLightAtt(int idx, vec3 dir);
    void addDirLightAtt(DirLight dl) {
        if (nDirLights < MAX_LIGHT_COUNT) dirLights[nDirLights++] = dl;
    }
    void setDirLightAtt(int idx, DirLight dl) {
        if (idx < nDirLights) dirLights[idx] = dl;
    }

    void use();
    void loadMaterial();
    void loadStoneMaterial();
    void loadPlasticMaterial();
    void loadWoodMaterial();
    void loadRubberMaterial();
    void loadMetalMaterial();
    void loadShinyMaterial();
    void loadCustomMaterial(float s);
};

#endif /* LIGHTING_H */
