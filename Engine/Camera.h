
#ifndef CRAFTBONE_CAMERA_H
#define CRAFTBONE_CAMERA_H


#include <glm/glm.hpp>

namespace Engine
{

class Camera
{

public:

    Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float fovy, float aspectRatio, float near, float far);

    Camera(float fovy, float aspectRatio, float near, float far);

    void setPosition(glm::vec3 position);

    void setDirection(glm::vec3 direction);

    void setUp(glm::vec3 up);

    void setMoveSpeed(float speed);

    void setRotSpeed(float speed);

    void setPerpective(float fovy, float aspectRatio, float near, float far);

    void update();

    void rotate(float deltaX, float deltaY);

    void moveLeft();

    void moveLeft(float speed);

    void moveRight();

    void moveRight(float speed);

    void moveForward();

    void moveForward(float speed);

    void moveBack();

    void moveBack(float speed);

    void setSpeedMultiplier(float mult);

    glm::mat4 getProjection();

    glm::mat4 getView();

    glm::vec3 getPosition() const;

private:
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_moveSpeed = 0.1f;
    float m_rotSpeed = 0.01f;
    float m_speedMultiplier = 1.0f;

};

}

#endif //CRAFTBONE_CAMERA_H
