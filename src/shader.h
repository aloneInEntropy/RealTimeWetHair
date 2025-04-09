#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>

#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "util.h"

class Shader {
   public:
    GLuint ID = 0;
    std::string name;
    Shader() {}
    // Create a vertex and fragment shader
    Shader(std::string shader_name, std::string vertex_shader_path, std::string fragment_shader_path) {
        name = shader_name;
        ID = CompileShaders(vertex_shader_path.c_str(), fragment_shader_path.c_str());
    }

    // Create a typed shader
    Shader(std::string shader_name, std::string shader_path, GLenum shader_type) {
        name = shader_name;
        ID = CompileTypedShader(shader_path.c_str(), shader_type);
    }

    // Create a set of attached shaders. Note that you may not combine compute and non-compute shaders
    Shader(std::string shader_name, std::vector<std::pair<std::string, GLenum>> shader_pairs) {
        name = shader_name;
        ID = CompileShaderGroup(shader_pairs);
    }

    void AddShader(GLuint ShaderProgram, const char* pShaderText,
                   GLenum ShaderType);
    GLuint CompileShaders(const char* pVS, const char* pFS);
    GLuint CompileTypedShader(const char* pS, GLenum type);
    GLuint CompileShaderGroup(std::vector<std::pair<std::string, GLenum>> pairs);

    // activate the shader
    void use() { glUseProgram(ID); }
    // deactivate the shader
    void rmv() { glUseProgram(0); }

    /* utility uniform functions */
    // set a boolean value
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // set a float value
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // set a vec2 value
    void setVec2(const std::string& name, vec2 value) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
    }
    // set a vec3 value
    void setVec3(const std::string& name, vec3 v) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
    }
    // set an ivec3 value
    void setIVec3(const std::string& name, ivec3 v) const {
        glUniform3i(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
    }
    // set a vec4 value
    void setVec4(const std::string& name, vec4 v) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z, v.w);
    }
    // set a mat3 value
    void setMat3(const std::string& name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // set a mat4 value
    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                           &mat[0][0]);
    }
};

#endif /* SHADER_H */
