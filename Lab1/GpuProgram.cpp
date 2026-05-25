#include "GpuProgram.h"
#include <fstream>
#include <sstream>
#include <iostream>

GpuProgram::GpuProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = LoadFile(vertexPath);
    std::string fragmentCode = LoadFile(fragmentPath);

    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);

    programID_ = glCreateProgram();
    glAttachShader(programID_, vertex);
    glAttachShader(programID_, fragment);
    glLinkProgram(programID_);
    CheckLinkErrors(programID_);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

GpuProgram::~GpuProgram() {
    glDeleteProgram(programID_);
}

void GpuProgram::Use() const {
    glUseProgram(programID_);
}

std::string GpuProgram::LoadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open file " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint GpuProgram::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    CheckCompileErrors(shader, shaderType);
    return shader;
}

void GpuProgram::CheckCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n";
    }
}

void GpuProgram::CheckLinkErrors(GLuint program) {
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << "\n";
    }
}

GLint GpuProgram::GetUniformLocation(const std::string& name) const {
    if (uniformLocationCache_.find(name) != uniformLocationCache_.end())
        return uniformLocationCache_[name];

    GLint location = glGetUniformLocation(programID_, name.c_str());
    if (location == -1)
        std::cerr << "Warning: uniform '" << name << "' not found or not active.\n";

    uniformLocationCache_[name] = location;
    return location;
}

void GpuProgram::SetUniform(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

void GpuProgram::SetUniform(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void GpuProgram::SetUniform(const std::string& name, float v1, float v2) const {
    glUniform2f(GetUniformLocation(name), v1, v2);
}

void GpuProgram::SetUniform(const std::string& name, float v1, float v2, float v3) const {
    glUniform3f(GetUniformLocation(name), v1, v2, v3);
}

void GpuProgram::SetUniform(const std::string& name, float v1, float v2, float v3, float v4) const {
    glUniform4f(GetUniformLocation(name), v1, v2, v3, v4);
}

void GpuProgram::SetUniform(const std::string& name, const float* mat4, bool transpose) const {
    glUniformMatrix4fv(GetUniformLocation(name), 1, transpose ? GL_TRUE : GL_FALSE, mat4);
}