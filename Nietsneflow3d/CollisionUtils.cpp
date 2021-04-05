#include "CollisionUtils.hpp"
#include "PhysicalEngine.hpp"
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <cmath>
#include <limits>
#include <iostream>

//===================================================================
bool checkCircleRectCollision(const pairFloat_t &cicleCenter,
                              const float circleRay,
                              const pairFloat_t &rectOrigin,
                              const pairFloat_t &rectSize)
{
    float circleDiameter = circleRay * 2.0f;
    if(!checkRectRectCollision({cicleCenter.first - circleRay, cicleCenter.second - circleRay},
                               {circleDiameter, circleDiameter}, rectOrigin, rectSize))
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
    float circleDiameterA = rayCircleA * 2.0f,
    circleDiameterB = rayCircleB * 2.0f;
    if(!checkRectRectCollision(
        {circleCenterA.first - rayCircleA, circleCenterA.second - rayCircleA},
        {circleDiameterA, circleDiameterA},
        {circleCenterB.first - rayCircleB, circleCenterB.second - rayCircleB},
        {circleDiameterB, circleDiameterB}))
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
    return !(rectOriginA.first + rectSizeA.first <= rectOriginB.first ||
            rectOriginB.first + rectSizeB.first <= rectOriginA.first ||
            rectOriginA.second + rectSizeA.second <= rectOriginB.second ||
            rectOriginB.second + rectSizeB.second <= rectOriginA.second);
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
    if(std::abs(denom) <= EPSILON_FLOAT)
    {
        return false;   // erreur, cas limite
    }
    float t = - (firstPointSegmentA.first * distYB -
                 firstPointSegmentB.first * distYB -
                 distXB * firstPointSegmentA.second +
                 distXB * firstPointSegmentB.second) / denom;
    if (t < 0.0f || t >= 1.0f)
    {
        return false;
    }
    float u = - (-distXA * firstPointSegmentA.second +
            distXA * firstPointSegmentB.second +
            distYA * firstPointSegmentA.first -
            distYA * firstPointSegmentB.first) / denom;
    if (u < 0.0f || u >= 1.0f)
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
    float lenght = std::sqrt(distanceX * distanceX + distanceY * distanceY);
    float dot = (((circleCenter.first - lineFirstPoint.first) * (lineSecondPoint.first - lineFirstPoint.first)) +
                 ((circleCenter.second - lineFirstPoint.second)*(lineSecondPoint.second - lineFirstPoint.second))) /
            (lenght * lenght);

    float closestX = lineFirstPoint.first + (dot * (lineSecondPoint.first - lineFirstPoint.first));
    float closestY = lineFirstPoint.second + (dot * (lineSecondPoint.second - lineFirstPoint.second));
    return !(closestX < minX || closestX > maxX || closestY < minY || closestY > maxY);
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
    return check <= EPSILON_FLOAT;
}

//===================================================================
bool checkPointCircleCollision(const pairFloat_t &point,
        const pairFloat_t &cicleCenter,
        const float circleRay)
{
    return getDistance(point, cicleCenter) <= circleRay;
}

//===================================================================
float getDistance(const pairFloat_t &pointA, const pairFloat_t &pointB)
{
    float distanceX = std::abs(pointA.first - pointB.first),
            distanceY = std::abs(pointA.second - pointB.second);
    if(distanceY < 0.01f)
    {
        return distanceX;
    }
    if(distanceX < 0.01f)
    {
        return distanceY;
    }
    return distanceY / std::cos(std::atan(distanceX / distanceY));
}

//===================================================================
float getCameraDistance(const pairFloat_t &observerPoint, const pairFloat_t &targetPoint,
                        float observerAngleRadiant, bool limit)
{
    float angleCalc;
    if(limit)
    {
        angleCalc = PI_QUARTER;
    }
    else
    {
        angleCalc = std::abs(getTrigoAngle(observerPoint, targetPoint, false) - observerAngleRadiant);
    }
    float hyp = getDistance(observerPoint, targetPoint);
    return std::abs(hyp * std::cos(angleCalc));
}

//===================================================================
float getTrigoAngle(const pairFloat_t &pointA, const pairFloat_t &pointB, bool degree)
{
    float X = std::abs(pointB.first - pointA.first);
    float Y = std::abs(pointB.second - pointA.second);
    float angle;
    //Bas Gauche
    if(pointA.first >= pointB.first && pointA.second <= pointB.second)
    {
        angle = getDegreeAngle(std::atan(Y / X)) + 180.0f;
    }
    //Bas Droite
    else if(pointA.first <= pointB.first && pointA.second <= pointB.second)
    {
        angle = getDegreeAngle(std::atan(X / Y)) + 270.0f;
    }
    //Haut Droite
    else if(pointA.first <= pointB.first && pointA.second >= pointB.second)
    {
        angle = getDegreeAngle(std::atan(Y / X));
    }
    //Haut Gauche
    else
    {
        angle = getDegreeAngle(std::atan(X / Y)) + 90.0f;
    }
    if(!degree)
    {
        angle = getRadiantAngle(angle);
    }
    return angle;
}

//===================================================================
float getRectTriangleSide(float adj, float hyp)
{
    float radAngle = std::acos(adj / hyp);
    return std::sin(radAngle) * hyp;
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

//===================================================================
bool checkTriangleCircleCollision(const array3PairFloat_t &trianglePoints,
                                  const pairFloat_t &cicleCenter, const float circleRay)
{
    float circleDiameter = circleRay * 2;
    pairPairFloat_t rectShapeTriangle = getRectShapeFromTriangle(trianglePoints);
    if(!checkRectRectCollision({cicleCenter.first - circleRay, cicleCenter.second - circleRay},
                               {circleDiameter, circleDiameter}, rectShapeTriangle.first,
                               rectShapeTriangle.second))
    {
        return false;
    }
    if(checkTrianglePointCollision(trianglePoints, cicleCenter))
    {
        return true;
    }
    if(checkCircleSegmentCollision(cicleCenter, circleRay, trianglePoints[0], trianglePoints[1]) ||
            checkCircleSegmentCollision(cicleCenter, circleRay, trianglePoints[0], trianglePoints[2]) ||
            checkCircleSegmentCollision(cicleCenter, circleRay, trianglePoints[1], trianglePoints[2]))
    {
        return true;
    }
    return false;
}

//===================================================================
bool checkTriangleRectCollision(const array3PairFloat_t &trianglePoints,
                                const pairPairFloat_t &rectShape)
{
    pairPairFloat_t rectShapeTriangle = getRectShapeFromTriangle(trianglePoints);
    if(!checkRectRectCollision(rectShape.first, rectShape.second,
                               rectShapeTriangle.first, rectShapeTriangle.second))
    {
        return false;
    }
    if(checkTrianglePointCollision(trianglePoints, {rectShape.first.first,
                                                    rectShape.first.second}) ||
            checkTrianglePointCollision(trianglePoints, {rectShape.first.first + rectShape.second.first,
                                                         rectShape.first.second}) ||
            checkTrianglePointCollision(trianglePoints, {rectShape.first.first + rectShape.second.first,
                                                         rectShape.first.second + rectShape.second.second}) ||
            checkTrianglePointCollision(trianglePoints, {rectShape.first.first,
                                                         rectShape.first.second + rectShape.second.second}))
    {
        return true;
    }
    uint32_t j;
    for(uint32_t i = 0; i < 3; ++i)
    {
        j = (i == 2) ?  0 : (i + 1);
        if(checkSegmentSegmentCollision(trianglePoints[i], trianglePoints[j],
                                        {rectShape.first.first, rectShape.first.second},
                                        {rectShape.first.first + rectShape.second.first, rectShape.first.second}) ||
                checkSegmentSegmentCollision(trianglePoints[i], trianglePoints[j],
                                             {rectShape.first.first + rectShape.second.first, rectShape.first.second},
                                             {rectShape.first.first + rectShape.second.first,
                                             rectShape.first.second + rectShape.second.second}) ||
                checkSegmentSegmentCollision(trianglePoints[i], trianglePoints[j],
                                             {rectShape.first.first + rectShape.second.first,
                                             rectShape.first.second + rectShape.second.second},
                                             {rectShape.first.first, rectShape.first.second + rectShape.second.second}) ||
                checkSegmentSegmentCollision(trianglePoints[i], trianglePoints[j],
                                             {rectShape.first.first, rectShape.first.second + rectShape.second.second},
                                             {rectShape.first.first, rectShape.first.second}))
        {
            return true;
        }
    }
    return false;
}

//===================================================================
bool checkTrianglePointCollision(const array3PairFloat_t &trianglePoints,
                                 const pairFloat_t &point)
{
//    d = Dx ∗ Ty − Dy ∗ Tx
    float determinant, j;
    bool memPrevious = true;//remove warning
    pairFloat_t vectA, vectB;
    for(uint32_t i = 0; i < 3; ++i)
    {
        j = (i == 2) ?  0 : (i + 1);
        vectA = {trianglePoints[j].first - trianglePoints[i].first,
                 trianglePoints[j].second - trianglePoints[i].second};
        vectB = {point.first - trianglePoints[i].first,
                 point.second - trianglePoints[i].second};
        determinant = vectA.first * vectB.second - vectA.second * vectB.first;
        //point on the line
        if(floatEqualsZero(determinant))
        {
            return true;
        }
        if(i == 0)
        {
            memPrevious = (determinant < 0.00f);
        }
        else if(memPrevious != (determinant < 0.00f))
        {
            return false;
        }
    }
    return true;
}

//===================================================================
pairPairFloat_t getRectShapeFromTriangle(const array3PairFloat_t &trianglePoints)
{
    float minX, maxX, minY, maxY;
    minX = std::min(trianglePoints[0].first, trianglePoints[1].first);
    minX = std::min(minX, trianglePoints[2].first);
    maxX = std::max(trianglePoints[0].first, trianglePoints[1].first);
    maxX = std::max(maxX, trianglePoints[2].first);

    minY = std::min(trianglePoints[0].second, trianglePoints[1].second);
    minY = std::min(minY, trianglePoints[2].second);
    maxY = std::max(trianglePoints[0].second, trianglePoints[1].second);
    maxY = std::max(maxY, trianglePoints[2].second);

    return {{minX, minY}, {(maxX - minX), (maxY - minY)}};
}

//===================================================================
bool floatEqualsZero(float value)
{
    return static_cast<int32_t>(value * 100.0f) == 0;
}

//===================================================================
bool checkFloatEqualsZero(float valueA, float valueB)
{
    return static_cast<int32_t>(valueA * 100.0f) == static_cast<int32_t>(valueB * 100.0f);
}

//===================================================================
pairFloat_t getCircleUpLeftCorner(const pairFloat_t &point, float ray)
{
    return {point.first - ray, point.second - ray};
}
