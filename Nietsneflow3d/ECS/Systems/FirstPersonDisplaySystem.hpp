#pragma once

#include <BaseECS/system.hpp>
#include <vector>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>

struct GeneralCollisionComponent;

class FirstPersonDisplaySystem : public ecs::System
{
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void setVectTextures(std::vector<Texture> &vectTexture);
    void setShader(Shader &shader);
private:
    void setUsedComponents();
    void confCompVertexMemEntities();
    void confVertex(uint32_t numEntity, GeneralCollisionComponent *genCollComp,
                    VisionComponent *visionComp, float lateralPosDegree, float distance);
    void drawVertex();
    pairFloat_t getCenterPosition(MapCoordComponent const *mapComp, GeneralCollisionComponent *genCollComp, float numEntity);
    void fillVertexFromEntitie(uint32_t numEntity, uint32_t numIteration);
private:
    Shader *m_shader;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    uint32_t m_numVertexToDraw;
};
