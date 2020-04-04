#pragma once

#include <BaseECS/system.hpp>
#include <vector>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>

class FirstPersonDisplaySystem : public ecs::System
{
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void confPlayerComp(uint32_t playerNum);
private:
    void setUsedComponents();
    void excludeOutVisionEntities();
private:
    uint32_t m_memPlayerEntity;
    Shader *m_shader;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    PlayerComp m_playerComp;
};
