#include <glad.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.hpp"

//===================================================================
Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    setSources(vertexPath, fragmentPath);
}

//===================================================================
void Shader::setSources(const std::string &vertexPath,
                        const std::string &fragmentPath)
{
    std::ifstream vertexShaderFile, fragmentShaderFile;
    vertexShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vertexShaderFile.open(vertexPath);
        fragmentShaderFile.open(fragmentPath);
        std::stringstream vertexShaderStream, fragmentShaderStream;
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << vertexShaderFile.rdbuf();
        m_vertexSource = vertexShaderStream.str();
        m_fragmentSource = fragmentShaderStream.str();
        vertexShaderFile.close();
        fragmentShaderFile.close();
    }
    catch(std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << std::endl;
    }
    generateShader();
}

//===================================================================
void Shader::use()
{
    glUseProgram(m_shaderProgram);
}

//===================================================================
void Shader::generateShader()
{
    bool success;
    success = generateVertexShader();
    assert(success && "Error while generating vertex shader");
    success = generateFragmentShader();
    assert(success && "Error while generating fragment shader");
    linkShader();
}

//===================================================================
void Shader::linkShader()
{
    // link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
    char infoLog[512];
    int success;
    // check for linking errors
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        assert("ERROR::SHADER::PROGRAM::LINKING_FAILED");
    }
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);
}

//===================================================================
bool Shader::generateVertexShader()
{
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertSource = m_vertexSource.c_str();
    glShaderSource(m_vertexShader, 1, &vertSource, NULL);
    glCompileShader(m_vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return success;
}

//===================================================================
bool Shader::generateFragmentShader()
{
    m_fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragSource = m_fragmentSource.c_str();
    glShaderSource(m_fragmentShader, 1, &fragSource, NULL);
    glCompileShader(m_fragmentShader);
    char infoLog[512];
    int success;
    // check for shader compile errors
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return success;
}
