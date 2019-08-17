#include "CollisionUtils.hpp"
#include <cmath>
#include <limits>

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
bool checkRectRectCollision(const pairFloat_t &rectOriginA,
                            const pairFloat_t &rectSizeA,
                            const pairFloat_t &rectOriginB,
                            const pairFloat_t &rectSizeB)
{
    return !(rectOriginA.first + rectSizeA.first < rectOriginB.first ||
       rectOriginB.first + rectSizeB.second < rectOriginA.first ||
       rectOriginA.second + rectSizeA.second < rectOriginB.second ||
       rectOriginB.second + rectSizeB.second < rectOriginA.second);
}

//===================================================================
bool checkLineRectCollision(const pairFloat_t &lineFirstPoint,
                    const pairFloat_t &lineSecondPoint,
                    const pairFloat_t &rectOrigin,
                    const pairFloat_t &rectSize)
{
    pairFloat_t currentPoint = lineFirstPoint;
    float minRectLine = std::min(rectSize.first, rectSize.second);
    float angle = atan((lineSecondPoint.first - lineFirstPoint.first) -
                       (lineSecondPoint.second - lineFirstPoint.second));
    pairFloat_t iterationPointValue = {std::cos(angle) * minRectLine,
                                       std::sin(angle) * minRectLine};
    //iterate while second point isn't reach
    do
    {
        if(checkPointRectCollision(currentPoint, rectOrigin, rectSize))
        {
            return true;
        }
        currentPoint += iterationPointValue;
        if(!checkPointPosition(lineFirstPoint, lineSecondPoint,
                             currentPoint))
        {
            return checkPointRectCollision(lineSecondPoint, rectOrigin,
                                           rectSize);
        }
    }while(true);
}

//===================================================================
bool checkPointPosition(const pairFloat_t &firstPoint,
                        const pairFloat_t &secondPoint,
                        const pairFloat_t &currentPoint)
{
    float checkEquality = firstPoint.first - secondPoint.first;
    if(checkEquality > std::numeric_limits<float>::epsilon())
    {
        if(firstPoint.first < secondPoint.first)
        {
            return currentPoint.first > currentPoint.first;
        }
        return currentPoint.first < currentPoint.first;
    }
    checkEquality = firstPoint.second - secondPoint.second;
    if(checkEquality > std::numeric_limits<float>::epsilon())
    {
        if(firstPoint.second < secondPoint.second)
        {
            return currentPoint.second > currentPoint.second;
        }
        return currentPoint.second < currentPoint.second;
    }
    else
    {
        return false;
    }
}

//===================================================================
bool checkCircleLineCollision(const pairFloat_t &circleCenter,
                              const float circleRay,
                              const pairFloat_t &lineFirstPoint,
                              const pairFloat_t &lineSecondPoint)
{
    if(checkPointCircleCollision(lineFirstPoint, circleCenter, circleRay) ||
       checkPointCircleCollision(lineSecondPoint, circleCenter, circleRay))
    {
        return true;
    }
    float size = getDistance(lineFirstPoint, lineSecondPoint);
    //produit scalaire segment cercle
    float dot = ( ((circleCenter.first - lineFirstPoint.first) *
             (lineSecondPoint.first - lineFirstPoint.first)) +
            ((circleCenter.second - lineFirstPoint.second) *
             (lineSecondPoint.second - lineFirstPoint.second)) ) /
            (size * size);

    float closestX = lineFirstPoint.first +
            (dot * (lineSecondPoint.first - lineFirstPoint.first));
    float closestY = lineFirstPoint.second +
            (dot * (lineSecondPoint.second - lineFirstPoint.second));
    return checkPointSegmentCollision({closestX, closestY}, lineFirstPoint,
                               lineSecondPoint);
}

//===================================================================
bool checkPointSegmentCollision(const pairFloat_t &point,
                                const pairFloat_t &firstSegmentPoint,
                                const pairFloat_t &secondSegmentPoint)
{
    float thalesA = (point.first - firstSegmentPoint.first) /
            (secondSegmentPoint.first - firstSegmentPoint.first),
    thalesB = (point.second - firstSegmentPoint.second) /
            (secondSegmentPoint.second - firstSegmentPoint.second);
    //if point is on segment
    if((thalesA - thalesB) <= std::numeric_limits<float>::epsilon())
    {
        float minX = std::min(firstSegmentPoint.first, secondSegmentPoint.first);
        float maxX = std::max(firstSegmentPoint.first, secondSegmentPoint.first);
        float minY = std::min(firstSegmentPoint.second, secondSegmentPoint.second);
        float maxY = std::max(firstSegmentPoint.second, secondSegmentPoint.second);
        if(point.first < minX || point.first > maxX ||
                point.second < minY || point.second > maxY)
        {
            return false;
        }
        return true;
    }
    return false;
}

//===================================================================
bool checkPointCircleCollision(const pairFloat_t &point,
                               const pairFloat_t &cicleCenter,
                               const float circleRay)
{
    return getDistance(point, cicleCenter) <= circleRay;
}

//===================================================================
//bool checkLineLineCollision(const pairFloat_t &lineFirstPointA,
//                            const pairFloat_t &lineSecondPointA,
//                            const pairFloat_t &lineFirstPointB,
//                            const pairFloat_t &lineSecondPointB)
//{

//}

//===================================================================
float getDistance(const pairFloat_t &pointA, const pairFloat_t &pointB)
{
    float distanceX = std::abs(pointA.first - pointB.first),
    distanceY = std::abs(pointA.second - pointB.second);
    return std::sqrt(distanceX * distanceX + distanceY * distanceY);
}

//===================================================================
bool checkPointRectCollision(const pairFloat_t &point,
                             const pairFloat_t &rectOrigin,
                             const pairFloat_t &rectSize)
{
    return !(point.first < rectOrigin.first ||
             point.first > rectOrigin.first + rectSize.first ||
             point.second < rectOrigin.second ||
             point.second > rectOrigin.second + rectSize.second);
}
