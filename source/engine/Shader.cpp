#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <fstream>
#include <iostream>

std::string get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (in.is_open()) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }
    std::cout << "COULD NOT OPEN FILE" << filename << '\n';
    throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    InitializeShader(vertexFile, fragmentFile);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile) {
    InitializeShader(vertexFile, fragmentFile,geometryFile);
}


void Shader::InitializeShader(const char* vertexFile, const char* fragmentFile) {

    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    CompileErrors(vertexShader, "VERTEX");

    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    CompileErrors(fragmentShader, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    CompileErrors(ID, "PROGRAM");
}

void Shader::InitializeShader(const char* vertexFile, const char* fragmentFile, const char* geometryFile) {

    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);
    std::string geometryCode = get_file_contents(geometryFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();
    const char* geometrySource = geometryCode.c_str();

    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    CompileErrors(vertexShader, "VERTEX");

    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    CompileErrors(fragmentShader, "FRAGMENT");

    const GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometrySource, NULL);
    glCompileShader(geometryShader);
    CompileErrors(geometryShader, "GEOMETRY");

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glAttachShader(ID, geometryShader);
    glLinkProgram(ID);
    CompileErrors(ID, "PROGRAM");
}

void Shader::InitializeCompute(const char* computeFile)
{
    if (ID)
        Delete();
    shaderLocation = computeFile;
    std::string sLocation = get_file_contents(computeFile);
    const char* computeSource = sLocation.c_str();
    unsigned int compute;

    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &computeSource, NULL);
    glCompileShader(compute);
    CompileErrors(compute, "COMPUTE");

    // shader creation
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    CompileErrors(ID, "PROGRAM");
}

void Shader::ReloadCompute()
{
    InitializeCompute(shaderLocation);
}

void Shader::Activate()
{
    glUseProgram(ID);
}



void Shader::Delete()
{
    glDeleteProgram(ID);
}


void Shader::SetMat4(const char* location, const glm::mat4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(ID, location),1,false, glm::value_ptr(value));
}

void Shader::SetVec4(const char* location, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(ID, location), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const char* location, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(ID, location),1,glm::value_ptr(value));
}
void Shader::SetVec2(const char* location, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(ID, location), 1, glm::value_ptr(value));
}


void Shader::SetInt(const char* location, int value) {
    glUniform1i(glGetUniformLocation(ID,location), value);
}
void Shader::SetFloat(const char* location, float value)
{
    glUniform1f(glGetUniformLocation(ID, location), value);
}

void Shader::SetBool(const char* location, bool value) {
    glUniform1i(glGetUniformLocation(ID, location), value);
}


void Shader::CompileErrors(unsigned int shader, const char* type)
{
    GLint hasCompiled;
    char infoLog[1024];
    if (type != std::string("PROGRAM")) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for:" << type << '\n' << infoLog << '\n' << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for:" << type << '\n' << infoLog << '\n' << std::endl;
        }
    }
}
