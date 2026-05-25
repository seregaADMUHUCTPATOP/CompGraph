#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>

class GpuProgram {
public:
    GpuProgram(const std::string& vertexPath, const std::string& fragmentPath);
    ~GpuProgram();

    void Use() const;

    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, float v1, float v2) const;
    void SetUniform(const std::string& name, float v1, float v2, float v3) const;
    void SetUniform(const std::string& name, float v1, float v2, float v3, float v4) const;
    void SetUniform(const std::string& name, const float* mat4, bool transpose = false) const;

private:
    GLuint programID_;
    mutable std::unordered_map<std::string, GLint> uniformLocationCache_;

    static std::string LoadFile(const std::string& path);
    static GLuint CompileShader(GLenum type, const std::string& source);
    static void CheckCompileErrors(GLuint shader, const std::string& type);
    static void CheckLinkErrors(GLuint program);

    GLint GetUniformLocation(const std::string& name) const;
};