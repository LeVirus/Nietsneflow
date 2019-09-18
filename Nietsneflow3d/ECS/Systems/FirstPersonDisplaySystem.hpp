#ifndef FIRSTPERSONDISPLAYSYSTEM_H
#define FIRSTPERSONDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <vector>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>

class FirstPersonDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    PlayerComp m_playerComp;
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void confPlayerComp(uint32_t playerNum);
};

#endif // FIRSTPERSONDISPLAYSYSTEM_H
