
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "Camera.h"

namespace Engine
{

    Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float fovy, float aspectRatio, float near, float far)
        : position(glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_direction = direction;
        m_up = up;

        setPerpective(fovy, aspectRatio, near, far);
    }

    Camera::Camera(float fovy, float aspectRatio, float near, float far)
    {
        setPerpective(fovy, aspectRatio, near, far);
    }

    void Camera::setPerpective(float fovy, float aspectRatio, float near, float far)
    {
        m_projectionMatrix = glm::perspective(glm::radians(fovy), aspectRatio, near, far);
    }

    void Camera::update()
    {
        m_viewMatrix = glm::lookAt(position.get(), position.get() + m_direction, m_up);
    }

    void Camera::setDirection(glm::vec3 direction)
    {
        m_direction = direction;
    }

    void Camera::setUp(glm::vec3 up)
    {
        m_up = up;
    }

    void Camera::rotate(float deltaX, float deltaY)
    {
        glm::mat4 yaw = glm::rotate(m_rotSpeed * -deltaX, m_up);
        glm::mat4 pitch = glm::rotate(m_rotSpeed * -deltaY,
                                    glm::normalize(cross(m_direction, m_up)));
        m_direction = glm::vec3(pitch * yaw * glm::vec4(m_direction, 0.0f));
    }

    void Camera::moveLeft()
    {
        this->moveLeft(m_moveSpeed);
    }

    void Camera::moveLeft(float speed)
    {
        position.set(position.get() - glm::normalize(glm::cross(m_direction, m_up)) * m_speedMultiplier * speed);
    }

    void Camera::moveRight()
    {
        this->moveRight(m_moveSpeed);
    }

    void Camera::moveRight(float speed)
    {
        position.set(position.get() + glm::normalize(glm::cross(m_direction, m_up)) * m_speedMultiplier * speed);
    }

    void Camera::moveForward()
    {
        this->moveForward(m_moveSpeed);
    }

    void Camera::moveForward(float speed)
    {
        position.set(position.get() + m_speedMultiplier * speed * m_direction);
    }

    void Camera::moveBack()
    {
        this->moveBack(m_moveSpeed);
    }

    void Camera::moveBack(float speed)
    {
        position.set(position.get() - m_speedMultiplier * speed * m_direction);
    }

    void Camera::setMoveSpeed(float speed)
    {
        m_moveSpeed = speed;
    }

    void Camera::setRotSpeed(float speed)
    {
        m_rotSpeed = speed;
    }

    glm::mat4 Camera::getProjection()
    {
        return m_projectionMatrix;
    }

    glm::mat4 Camera::getView()
    {
        return m_viewMatrix;
    }

    void Camera::setSpeedMultiplier(float mult)
    {
        m_speedMultiplier = mult;
    }
}
