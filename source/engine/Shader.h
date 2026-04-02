#pragma once
#include "Common.h"
#include <string>
#include "glm/fwd.hpp"

std::string get_file_contents(const char* filename);

class Shader
{
public:
    GLuint ID{};

    void InitializeShader(const char* vertexFile, const char* fragmentFile);
    void InitializeShader(const char* vertexFile, const char* fragmentFile, const char* geometryFile);
    void InitializeCompute(const char* computeFile);
    
    const char* shaderLocation{};
    void ReloadCompute();


    Shader() {}
    Shader(const char* vertexFile, const char* fragmentFile);
    Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile);


    void Activate();
    void Delete();

    // setters
    void SetMat4(const char* location, const glm::mat4& value);
    void SetVec4(const char* location, const glm::vec4& value);
    void SetVec3(const char* location, const glm::vec3& value);
    void SetVec2(const char* location, const glm::vec2& value);
    void SetInt(const char* location, int value);
    void SetFloat(const char* location, float value);
    void SetBool(const char* location, bool value);
private:
    void CompileErrors(unsigned int shader, const char* type);

};

