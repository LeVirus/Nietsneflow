#include "CollisionUtils.hpp"

//===================================================================
bool checkCircleRectCollision(const pairFloat_t &cicleCenter,
                    const float circleRay,
                    const pairFloat_t &rectOrigin,
                    const pairFloat_t &rectSize)
{
    if(cicleCenter.first + circleRay < rectOrigin.first ||
       cicleCenter.first - circleRay > rectOrigin.first + rectSize.first ||
       cicleCenter.second + circleRay < rectOrigin.second ||
       cicleCenter.second - circleRay > rectOrigin.second + rectSize.second)
    {
        return false;
    }
    pairFloat_t rectCenterPoint = {rectOrigin.first + rectSize.first / 2,
                                   rectOrigin.second + rectSize.second / 2};
    float rectDiagonal = getDistance(rectOrigin, rectCenterPoint);
    //Add test
    if(getDistance(rectCenterPoint, cicleCenter) > (circleRay + rectDiagonal))
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkCircleCircleCollision(const pairFloat_t &circleCenterA,
                    const float rayCircleA,
                    const pairFloat_t &circleCenterB,
                    const float rayCircleB)
{
    if((circleCenterA.first + rayCircleA) < (circleCenterB.first - rayCircleB) ||
       (circleCenterB.first + rayCircleB) < (circleCenterA.first - rayCircleA) ||
       (circleCenterA.second + rayCircleA) < (circleCenterB.second - rayCircleB) ||
       (circleCenterB.second + rayCircleB) < (circleCenterA.second - rayCircleA))
    {
        return false;
    }
    if(getDistance(circleCenterA, circleCenterB) > rayCircleA + rayCircleB)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkRectRectCollision(const pairFloat_t &rectSizeA,
                    const pairFloat_t &rectSizeB,
                    const pairFloat_t &rectOriginA,
                    const pairFloat_t &rectOriginB)
{
    if(rectOriginA.first + rectSizeA.first < rectOriginB.first ||
       rectOriginB.first + rectSizeB.second < rectOriginA.first ||
       rectOriginA.second + rectSizeA.second < rectOriginB.second ||
       rectOriginB.second + rectSizeB.second < rectOriginA.second)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkLineRectCollision(const pairFloat_t &lineFirstPoint,
                    const pairFloat_t &lineSecondPoint,
                    const pairFloat_t &rectOrigin,
                    const pairFloat_t &rectSize)
{

}

//===================================================================
bool checkCircleLineCollision(const pairFloat_t &circleCenter,
                    const float circleRay,
                    const pairFloat_t &lineFirstPoint,
                    const pairFloat_t &lineSecondPoint)
{

}

//===================================================================
bool checkLineLineCollision(const pairFloat_t &lineFirstPointA,
                    const pairFloat_t &lineSecondPointA,
                    const pairFloat_t &lineFirstPointB,
                    const pairFloat_t &lineSecondPointB)
{

}

//===================================================================
float getDistance(const pairFloat_t &pointA, const pairFloat_t &pointB)
{
    float distanceX = std::abs(pointA.first - pairB.first),
    distanceY = std::abs(pointA.second - pairB.second);
    return std::sqrt(distanceX * distanceX + distanceY * distanceY);
}
