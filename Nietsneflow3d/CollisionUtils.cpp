#include "CollisionUtils.hpp"
#include <cmath>
#include <limits>
#include <iostream>

//===================================================================
bool checkCircleRectCollision(const pairFloat_t &cicleCenter,
        const float circleRay,
        const pairFloat_t &rectOrigin,
        const pairFloat_t &rectSize)
{
    if(cicleCenter.first + circleRay <= rectOrigin.first ||
            cicleCenter.first - circleRay >= rectOrigin.first + rectSize.first ||
            cicleCenter.second + circleRay <= rectOrigin.second ||
            cicleCenter.second - circleRay >= rectOrigin.second + rectSize.second)
    {
        return false;
    }
    pairFloat_t rectCenterPoint = {rectOrigin.first + rectSize.first / 2,
        rectOrigin.second + rectSize.second / 2};
    float rectDiagonal = getDistance(rectOrigin, rectCenterPoint);
    //Add test
    if(getDistance(rectCenterPoint, cicleCenter) >
            (circleRay + rectDiagonal))
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
    std::cerr << "\n\nCOLLA " << circleCenterA.first << " " << circleCenterA.second << " ray " << rayCircleA <<"\n";
    std::cerr << "B " << circleCenterB.first << " " << circleCenterB.second << " ray " << rayCircleB << "\n";
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
bool checkSegmentRectCollision(const pairFloat_t &lineFirstPoint,
        const pairFloat_t &lineSecondPoint,
        const pairFloat_t &rectOrigin,
        const pairFloat_t &rectSize)
{
    float minX = std::min(lineFirstPoint.first, lineSecondPoint.first);
    float maxX = std::max(lineFirstPoint.first, lineSecondPoint.first);
    float minY = std::min(lineFirstPoint.second, lineSecondPoint.second);
    float maxY = std::max(lineFirstPoint.second, lineSecondPoint.second);
    if(!checkRectRectCollision(rectOrigin, rectSize, {minX, minY}, {maxX - minX, maxY - minY}))
    {
        return false;
    }
    if(checkPointRectCollision(lineFirstPoint, rectOrigin, rectSize) ||
            checkPointRectCollision(lineSecondPoint, rectOrigin, rectSize))
    {
        return true;
    }
    pairFloat_t secondRectPoint = {rectOrigin.first, rectOrigin.second + rectSize.second};
    pairFloat_t thirdRectPoint = {rectOrigin.first + rectSize.first, rectOrigin.second};
    if(checkSegmentSegmentCollision(lineFirstPoint, lineSecondPoint, rectOrigin, secondRectPoint))
    {
        return true;
    }
    if(checkSegmentSegmentCollision(lineFirstPoint, lineSecondPoint, rectOrigin, thirdRectPoint))
    {
        return true;
    }
    pairFloat_t fourthRectPoint = {rectOrigin.first + rectSize.first, rectOrigin.second + rectSize.second};
    if(checkSegmentSegmentCollision(lineFirstPoint, lineSecondPoint, fourthRectPoint, secondRectPoint))
    {
        return true;
    }
    if(checkSegmentSegmentCollision(lineFirstPoint, lineSecondPoint, fourthRectPoint, thirdRectPoint))
    {
        return true;
    }
    return false;
}

//===================================================================
bool checkSegmentSegmentCollision(const pairFloat_t &firstPointSegmentA, const pairFloat_t &secondPointSegmentA, 
        const pairFloat_t &firstPointSegmentB, const pairFloat_t &secondPointSegmentB)
{
    float distXA = secondPointSegmentA.first - firstPointSegmentA.first;
    float distYA = secondPointSegmentA.second - firstPointSegmentA.second;
    float distXB = secondPointSegmentB.first - firstPointSegmentB.first;
    float distYB = secondPointSegmentB.second - firstPointSegmentB.second;
    float denom = distXA * distYB - distYA * distXB;
    std::numeric_limits<float>::epsilon();
    if(denom == std::numeric_limits<float>::epsilon())
    {
        return false;   // erreur, cas limite
    }
    float t = - (firstPointSegmentA.first * distYB - 
                 firstPointSegmentB.first * distYB - 
                 distXB * firstPointSegmentA.second + 
                 distXB * firstPointSegmentB.second) / denom;
    if (t < 0 || t >= 1)
    {
        return false;
    }
    float u = - (-distXA * firstPointSegmentA.second + 
            distXA * firstPointSegmentB.second + 
            distYA * firstPointSegmentA.first - 
            distYA * firstPointSegmentB.first) / denom;
    if (u < 0 || u >= 1)
    {
        return false;
    }
    return true;
}

//===================================================================
bool checkPointPosition(const pairFloat_t &firstPoint,
        const pairFloat_t &secondPoint,
        const pairFloat_t &currentPoint)
{
    if(!checkFloatEquals(firstPoint.first, secondPoint.first))
    {
        if(firstPoint.first < secondPoint.first)
        {
            return currentPoint.first > currentPoint.first;
        }
        return currentPoint.first < currentPoint.first;
    }
    if(!checkFloatEquals(firstPoint.second, secondPoint.second))
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
bool checkCircleSegmentCollision(const pairFloat_t &circleCenter,
        const float circleRay,
        const pairFloat_t &lineFirstPoint,
        const pairFloat_t &lineSecondPoint)
{
    float minX = std::min(lineFirstPoint.first, lineSecondPoint.first);
    float maxX = std::max(lineFirstPoint.first, lineSecondPoint.first);
    float minY = std::min(lineFirstPoint.second, lineSecondPoint.second);
    float maxY = std::max(lineFirstPoint.second, lineSecondPoint.second);
    if(!checkCircleRectCollision(circleCenter, circleRay, {minX, minY}, {maxX - minX, maxY - minY}))
    {
        return false;
    }
    float distanceX = maxX - minX;
    float distanceY = maxY - minY;
    float A = distanceX * distanceX + distanceY * distanceY;
    float B = 2 * (distanceX * (lineFirstPoint.first - circleCenter.first) + distanceY * (lineFirstPoint.second - circleCenter.second));
    float C = (lineFirstPoint.first - circleCenter.first) * (lineFirstPoint.first - circleCenter.first) 
        + (lineFirstPoint.second - circleCenter.second) * (lineFirstPoint.second - circleCenter.second) - circleRay * circleRay;

    float det = B * B - 4 * A * C;
    return det >= std::numeric_limits<float>::epsilon();
}

//===================================================================
bool checkPointSegmentCollision(const pairFloat_t &point,
        const pairFloat_t &firstSegmentPoint,
        const pairFloat_t &secondSegmentPoint)
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
    //calculate equation
    //    y = ax + b
    float a = (maxY - minY) / (maxX - minX);
    float b = minY - a * minX;
    return checkFloatEquals(point.second, a * point.first + b);
}

//===================================================================
bool checkFloatEquals(float a, float b)
{
    float check = std::fabs(a - b);
    return check <= std::numeric_limits<float>::epsilon();
}

//===================================================================
bool checkPointCircleCollision(const pairFloat_t &point,
        const pairFloat_t &cicleCenter,
        const float circleRay)
{
    return getDistance(point, cicleCenter) <= circleRay;
}

//===================================================================
//bool checkSegmentSegmentCollision(const pairFloat_t &lineFirstPointA,
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
