#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader
{
private:
    std::string m_vertexSource, m_fragmentSource;
    uint32_t m_vertexShader, m_fragmentShader, m_shaderProgram;
private:
    bool generateVertexShader();
    bool generateFragmentShader();
public:
    /**
     * @brief Shader
     * @param vertexPath Path to vertex shaders sources.
     * @param fragmentPath Path to fragment shaders sources.
     */
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    bool compileSource()const;
    void linkShader();
    void generateShader();
    void setSources(const std::string &vertexPath, const std::string &fragmentPath);
    void setUniformValueInt(const std::string &id, int value);
    void use();
    void display()const;
};

#endif // SHADER_H
