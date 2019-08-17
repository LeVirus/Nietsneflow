#ifndef COLLISIONUTILS_H
#define COLLISIONUTILS_H

#include <functional>

using pairFloat_t = std::pair<float, float>;

bool checkCircleRectCollision(const pairFloat_t &cicleCenter,
                              const float circleRay,
                              const pairFloat_t &rectOrigin,
                              const pairFloat_t &rectSize);

bool checkCircleCircleCollision(const pairFloat_t &circleCenterA,
                                const float rayCircleA,
                                const pairFloat_t &circleCenterB,
                                const float rayCircleB);

bool checkRectRectCollision(const pairFloat_t &rectOriginA,
                            const pairFloat_t &rectSizeA,
                            const pairFloat_t &rectOriginB,
                            const pairFloat_t &rectSizeB);

bool checkLineRectCollision(const pairFloat_t &lineFirstPoint,
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

bool checkCircleLineCollision(const pairFloat_t &circleCenter,
                              const float circleRay,
                              const pairFloat_t &lineFirstPoint,
                              const pairFloat_t &lineSecondPoint);

//bool checkLineLineCollision(const pairFloat_t &lineFirstPointA,
//                            const pairFloat_t &lineSecondPointA,
//                            const pairFloat_t &lineFirstPointB,
//                            const pairFloat_t &lineSecondPointB);


bool checkPointPosition(const pairFloat_t &firstPoint,
                        const pairFloat_t &secondPoint,
                        const pairFloat_t &currentPoint);

float getDistance(const pairFloat_t &pointA, const pairFloat_t &pointB);

//Adapt to GL context
template <typename T>
std::pair<T,T> &operator+=(std::pair<T,T> & l, const std::pair<T,T> & r)
{
    l.first += r.first;
    l.second += r.second;
    return l;
}
#endif // COLLISIONUTILS_H
