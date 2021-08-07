
#ifndef CRAFTBONE_CAMERA_H
#define CRAFTBONE_CAMERA_H

#include <glm/glm.hpp>

#include "utils/Property.h"

namespace Engine
{

class Camera
{

public:

    Camera(glm::vec3 pos, glm::vec3 direction, glm::vec3 up, float fovy, float aspectRatio, float near, float far);

    Camera(float fovy, float aspectRatio, float near, float far);

    void setDirection(glm::vec3 direction);

    void setUp(glm::vec3 up);

    void setMoveSpeed(float speed);

    void setRotSpeed(float speed);

    void setPerspective(float fovy, float aspectRatio, float near, float far);

    void update();

    void rotate(float deltaX, float deltaY);

    void moveLeft(float speed);

    void moveRight(float speed);

    void moveForward(float speed);

    void moveBack(float speed);

    void move(const glm::vec2& moveVec);

    void setSpeedMultiplier(float mult);

    glm::mat4 getProjection();

    glm::mat4 getView();

    Property<glm::vec3> position;

private:
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    glm::vec3 m_direction;
    glm::vec3 m_up;

    float m_moveSpeed = 0.1f;
    float m_rotSpeed = 0.01f;
    float m_speedMultiplier = 1.0f;

};

}

#endif //CRAFTBONE_CAMERA_H
