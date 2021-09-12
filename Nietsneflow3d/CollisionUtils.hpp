#pragma once

#include <functional>

using PairFloat_t = std::pair<float, float>;
using array3PairFloat_t = std::array<PairFloat_t, 3>;
using pairPairFloat_t = std::pair<PairFloat_t, PairFloat_t>;

bool checkCircleRectCollision(const PairFloat_t &cicleCenter,
                              const float circleRay,
                              const PairFloat_t &rectOrigin,
                              const PairFloat_t &rectSize);

bool checkTriangleCircleCollision(const array3PairFloat_t &trianglePoints,
                                  const PairFloat_t &cicleCenter,
                                  const float circleRay);

bool checkTrianglePointCollision(const array3PairFloat_t &trianglePoints,
                                 const PairFloat_t &point);

bool floatEqualsZero(float value);
bool checkFloatEqualsZero(float valueA, float valueB);

pairPairFloat_t getRectShapeFromTriangle(const array3PairFloat_t &trianglePoints);

bool checkTriangleRectCollision(const array3PairFloat_t &trianglePoints,
                                const pairPairFloat_t &rectShape);

bool checkSegmentSegmentCollision(const PairFloat_t &firstPointSegmentA, 
                                  const PairFloat_t &secondPointSegmentA, 
                                  const PairFloat_t &firstPointSegmentB, 
                                  const PairFloat_t &secondPointSegmentB);

bool checkCircleCircleCollision(const PairFloat_t &circleCenterA,
                                const float rayCircleA,
                                const PairFloat_t &circleCenterB,
                                const float rayCircleB);

bool checkRectRectCollision(const PairFloat_t &rectOriginA,
                            const PairFloat_t &rectSizeA,
                            const PairFloat_t &rectOriginB,
                            const PairFloat_t &rectSizeB);

bool checkSegmentRectCollision(const PairFloat_t &lineFirstPoint,
                               const PairFloat_t &lineSecondPoint,
                               const PairFloat_t &rectOrigin,
                               const PairFloat_t &rectSize);

bool checkPointRectCollision(const PairFloat_t &point,
                             const PairFloat_t &rectOrigin,
                             const PairFloat_t &rectSize);

bool checkPointCircleCollision(const PairFloat_t &point,
                               const PairFloat_t &cicleCenter,
                               const float circleRay);

bool checkPointSegmentCollision(const PairFloat_t &point,
                                const PairFloat_t &firstSegmentPoint,
                                const PairFloat_t &secondSegmentPoint);

bool checkCircleSegmentCollision(const PairFloat_t &circleCenter,
                                 const float circleRay,
                                 const PairFloat_t &lineFirstPoint,
                                 const PairFloat_t &lineSecondPoint);

bool checkPointPosition(const PairFloat_t &firstPoint,
                        const PairFloat_t &secondPoint,
                        const PairFloat_t &currentPoint);

//get angle from 2 points from trigo sense
float getTrigoAngle(const PairFloat_t &pointA, const PairFloat_t &pointB, bool degree = true);
float getDistance(const PairFloat_t &pointA, const PairFloat_t &pointB);
float getCameraDistance(const PairFloat_t &observerPoint, const PairFloat_t &targetPoint,
                        float observerAngleRadiant, bool limit = false);
float getRectTriangleSide(float adj, float hyp);

PairFloat_t getCircleUpLeftCorner(const PairFloat_t &point, float ray);

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
