
#ifndef CRAFTBONE_SHADER_H
#define CRAFTBONE_SHADER_H

#include <iostream>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

namespace Engine {


class Shader {

public:

    /* Constructors */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    /* Activate the shader */
    void use();

    /* Set uniforms */
    void setUniform(const std::string &name, bool value) const;
    void setUniform(const std::string &name, int value) const;
    void setUniform(const std::string &name, float value) const;
    void setUniform(const std::string &name, glm::mat4 value) const;
    void setUniform(const std::string &name, glm::vec2* value, std::size_t length) const;

private:

    GLuint id; // Shader program id

};

}

#endif //CRAFTBONE_SHADER_H
