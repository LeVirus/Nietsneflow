#pragma once

#include <functional>

using pairFloat_t = std::pair<float, float>;
using array3PairFloat_t = std::array<pairFloat_t, 3>;
using pairPairFloat_t = std::pair<pairFloat_t, pairFloat_t>;

bool checkCircleRectCollision(const pairFloat_t &cicleCenter,
                              const float circleRay,
                              const pairFloat_t &rectOrigin,
                              const pairFloat_t &rectSize);

bool checkTriangleCircleCollision(const array3PairFloat_t &trianglePoints,
                                  const pairFloat_t &cicleCenter,
                                  const float circleRay);

bool checkTrianglePointCollision(const array3PairFloat_t &trianglePoints,
                                 const pairFloat_t &point);

bool floatEqualsZero(float value);
bool checkFloatEqualsZero(float valueA, float valueB);

pairPairFloat_t getRectShapeFromTriangle(const array3PairFloat_t &trianglePoints);

bool checkTriangleRectCollision(const array3PairFloat_t &trianglePoints,
                                const pairPairFloat_t &rectShape);

bool checkSegmentSegmentCollision(const pairFloat_t &firstPointSegmentA, 
                                  const pairFloat_t &secondPointSegmentA, 
                                  const pairFloat_t &firstPointSegmentB, 
                                  const pairFloat_t &secondPointSegmentB);

bool checkCircleCircleCollision(const pairFloat_t &circleCenterA,
                                const float rayCircleA,
                                const pairFloat_t &circleCenterB,
                                const float rayCircleB);

bool checkRectRectCollision(const pairFloat_t &rectOriginA,
                            const pairFloat_t &rectSizeA,
                            const pairFloat_t &rectOriginB,
                            const pairFloat_t &rectSizeB);

bool checkSegmentRectCollision(const pairFloat_t &lineFirstPoint,
                               const pairFloat_t &lineSecondPoint,
                               const pairFloat_t &rectOrigin,
                               const pairFloat_t &rectSize);

bool checkPointRectCollision(const pairFloat_t &point,
                             const pairFloat_t &rectOrigin,
                             const pairFloat_t &rectSize);

bool checkPointCircleCollision(const pairFloat_t &point,
                               const pairFloat_t &cicleCenter,
                               const float circleRay);

bool checkPointSegmentCollision(const pairFloat_t &point,
                                const pairFloat_t &firstSegmentPoint,
                                const pairFloat_t &secondSegmentPoint);

bool checkCircleSegmentCollision(const pairFloat_t &circleCenter,
                                 const float circleRay,
                                 const pairFloat_t &lineFirstPoint,
                                 const pairFloat_t &lineSecondPoint);

bool checkPointPosition(const pairFloat_t &firstPoint,
                        const pairFloat_t &secondPoint,
                        const pairFloat_t &currentPoint);

//get angle from 2 points from trigo sense
float getTrigoAngle(const pairFloat_t &pointA, const pairFloat_t &pointB);
float getDistance(const pairFloat_t &pointA, const pairFloat_t &pointB);
float getCameraDistance(const pairFloat_t &pointA, const pairFloat_t &pointB, float observerAngle);
float getRectTriangleSide(float adj, float hyp);

pairFloat_t getCircleUpLeftCorner(const pairFloat_t &point, float ray);

bool checkFloatEquals(float a, float b);

//Adapt to GL context
template <typename T>
std::pair<T,T> &operator+=(std::pair<T,T> & l, const std::pair<T,T> & r)
{
    l.first += r.first;
    l.second += r.second;
    return l;
}

template <typename T>
std::pair<T,T> operator+(const std::pair<T,T> & l, const std::pair<T,T> & r)
{
    return {l.first + r.first, l.second + r.second};
}
