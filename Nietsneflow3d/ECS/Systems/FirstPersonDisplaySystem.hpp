#ifndef FIRSTPERSONDISPLAYSYSTEM_H
#define FIRSTPERSONDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <vector>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>

class FirstPersonDisplaySystem
{
private:
    Shader *m_shader;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
public:
    FirstPersonDisplaySystem();
};

#endif // FIRSTPERSONDISPLAYSYSTEM_H
