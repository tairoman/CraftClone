
#include <fstream>
#include "Shader.h"

#include <vector>

namespace Engine {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {

    this->id = glCreateProgram();

    loadShader(vertexPath, Type::VERTEX);
    loadShader(fragmentPath, Type::FRAGMENT);

    glLinkProgram(this->id);
}

void Shader::loadShader(const std::string& path, Type shaderType) {

    GLuint shader = glCreateShader(shaderType == Type::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

    std::ifstream shader_file(path);

    if (shader_file.fail()) {
        std::cerr << "Something went wrong when a shader was loaded: " << path << "\n";
        exit(0);
    }

    std::string shader_src((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());

    const char* shaderStr = shader_src.c_str();

    glShaderSource(shader, 1, &shaderStr, nullptr);

    glCompileShader(shader);
    
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    
    if(!ok) {
        
        // Get the error message

	    GLint maxLength = 0;
	    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
	    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        std::cerr << "Failed to compile vertex shader:\n\t";
        for (auto ch : errorLog) {
            std::cerr << ch;
        }
        std::cerr << "\n";
        
        exit(0);
    }

    glAttachShader(this->id, shader);

    glDeleteShader(shader);

}

void Shader::use() const {
    glUseProgram( this->id );
}

void Shader::setUniform(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(this->id, name.c_str()), (int)value);
}

void Shader::setUniform(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(this->id, name.c_str()), value);
}

void Shader::setUniform(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(this->id, name.c_str()), value);
}

void Shader::setUniform(const std::string &name, glm::mat4 value) const {
    glUniformMatrix4fv(glGetUniformLocation(this->id, name.c_str()), 1, false, &value[0].x);
}

void Shader::setUniform(const std::string &name, float* value, std::size_t length, std::size_t stride) const {
    glUniform2fv(glGetUniformLocation(this->id, name.c_str()), length / stride, value);
}

}