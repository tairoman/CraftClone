
#include <fstream>
#include "Shader.h"

#include <vector>

namespace Engine {


Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {


    GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream vs_file(vertexPath);
    std::string vs_src((std::istreambuf_iterator<char>(vs_file)), std::istreambuf_iterator<char>());

    std::ifstream fs_file(fragmentPath);
    std::string fs_src((std::istreambuf_iterator<char>(fs_file)), std::istreambuf_iterator<char>());

    const char *vs = vs_src.c_str();
    const char *fs = fs_src.c_str();

    glShaderSource(vertexShader, 1, &vs, nullptr);
    glShaderSource(fragmentShader, 1, &fs, nullptr);

    glCompileShader(vertexShader);
    int ok;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ok);
    if(!ok) {
	    GLint maxLength = 0;
	    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
	    glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);

        std::cerr << "Failed to compile vertex shader:\n\t";
        for (auto ch : errorLog) {
            std::cerr << ch;
        }
        std::cerr << "\n";
        
        exit(0);
    }

    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        fprintf(stderr, "%s\n", "Failed to compile fragment shader");
        exit(0);
    }


    this->id = glCreateProgram();

    glAttachShader(this->id, fragmentShader);
    glAttachShader(this->id, vertexShader);

    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );

    glLinkProgram(this->id);
}

void Shader::use() {
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