
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "Camera.h"

namespace Engine
{

    Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, float fovy, float aspectRatio, float near, float far)
        : position(std::move(pos))
        , direction(std::move(dir))
        , m_up(std::move(up))
    {
        setPerspective(fovy, aspectRatio, near, far);

        position.onChange.listen([this](const glm::vec3&) {
            updateViewMatrix();
        });

        direction.onChange.listen([this](const glm::vec3&) {
            updateViewMatrix();
        });
    }

    Camera::Camera(float fovy, float aspectRatio, float near, float far)
        : Camera(glm::vec3{ 0,0,0 }, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, fovy, aspectRatio, near, far)
    {
        setPerspective(fovy, aspectRatio, near, far);
    }

    void Camera::setPerspective(float fovy, float aspectRatio, float near, float far)
    {
        m_projectionMatrix = glm::perspective(glm::radians(fovy), aspectRatio, near, far);
    }

    void Camera::updateViewMatrix()
    {
        m_viewMatrix = glm::lookAt(position.get(), position.get() + direction.get(), m_up);
    }

    void Camera::setUp(glm::vec3 up)
    {
        m_up = up;
    }

    void Camera::rotate(float deltaX, float deltaY)
    {
        glm::mat4 yaw = glm::rotate(m_rotSpeed * -deltaX, m_up);
        glm::mat4 pitch = glm::rotate(m_rotSpeed * -deltaY,
                                    glm::normalize(cross(direction.get(), m_up)));
        direction.set(glm::vec3(pitch * yaw * glm::vec4(direction.get(), 0.0f)));
    }

    void Camera::moveLeft(float speed)
    {
        move(glm::vec2{ -speed, 0 });
    }

    void Camera::moveRight(float speed)
    {
        move(glm::vec2{ speed, 0 });
    }

    void Camera::moveForward(float speed)
    {
        move(glm::vec2{ 0, speed });
    }

    void Camera::moveBack(float speed)
    {
        move(glm::vec2{ 0, -speed });
    }

    void Camera::move(const glm::vec2& moveVec)
    {
        if (glm::length(moveVec) == 0) {
            return;
        }

        auto newPos = position.get();
        
        if (moveVec.x != 0) {
            newPos += moveVec.x * glm::normalize(glm::cross(direction.get(), m_up)) * m_speedMultiplier;
        }

        if (moveVec.y != 0) {
            newPos += moveVec.y * m_speedMultiplier * direction.get();
        }

        position.set(newPos);
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
