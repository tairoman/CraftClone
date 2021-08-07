#pragma once

#include <array>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

namespace Engine
{

class Shader
{

public:

    enum class Type {
        VERTEX, FRAGMENT
    };

    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    /* Activate the shader */
    void use() const;

    void setUniform(const std::string &name, bool value) const;
    void setUniform(const std::string &name, int value) const;
    void setUniform(const std::string &name, float value) const;
    void setUniform(const std::string &name, glm::mat4 value) const;
private:

    void loadShader(const std::string& path, Type shaderType);

    GLuint id; // Shader program id

};

}
