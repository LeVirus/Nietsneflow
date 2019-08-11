#include "PhysicalEngine.hpp"


//===================================================================
PhysicalEngine::PhysicalEngine()
{

}

//===================================================================
void PhysicalEngine::runIteration()
{
    m_inputSystem->execSystem();
}

//===================================================================
void PhysicalEngine::linkSystems(InputSystem *inputSystem)
{
    m_inputSystem = inputSystem;
}

