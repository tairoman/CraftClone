
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


#include "Camera.h"

namespace Engine
{

    Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float fovy, float aspectRatio, float near, float far)
    {
        this->position = position;
        this->direction = direction;
        this->up = up;

        this->setPerpective(fovy, aspectRatio, near, far);
    }

    Camera::Camera(float fovy, float aspectRatio, float near, float far)
    {
        this->setPerpective(fovy, aspectRatio, near, far);
    }

    void Camera::setPerpective(float fovy, float aspectRatio, float near, float far)
    {
        this->projectionMatrix = glm::perspective(glm::radians(fovy), aspectRatio, near, far);
    }

    void Camera::update()
    {
        this->viewMatrix = glm::lookAt(this->position, this->position + this->direction, this->up);
    }

    void Camera::setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    glm::vec3 Camera::getPosition() const
    {
        return this->position;
    }

    void Camera::setDirection(glm::vec3 direction)
    {
        this->direction = direction;
    }

    void Camera::setUp(glm::vec3 up)
    {
        this->up = up;
    }

    void Camera::rotate(float deltaX, float deltaY)
    {
        glm::mat4 yaw = glm::rotate(this->rotSpeed * -deltaX, this->up);
        glm::mat4 pitch = glm::rotate(this->rotSpeed * -deltaY,
                                    glm::normalize(cross(this->direction, this->up)));
        this->direction = glm::vec3(pitch * yaw * glm::vec4(this->direction, 0.0f));
    }

    void Camera::moveLeft()
    {
        this->moveLeft(this->moveSpeed);
    }

    void Camera::moveLeft(float speed)
    {
        this->position -= glm::normalize(glm::cross(this->direction, this->up)) * this->speedMultiplier * speed;
    }

    void Camera::moveRight()
    {
        this->moveRight(this->moveSpeed);
    }

    void Camera::moveRight(float speed)
    {
        this->position += glm::normalize(glm::cross(this->direction, this->up)) * this->speedMultiplier * speed;
    }

    void Camera::moveForward()
    {
        this->moveForward(this->moveSpeed);
    }

    void Camera::moveForward(float speed)
    {
        this->position += this->speedMultiplier * speed * this->direction;
    }

    void Camera::moveBack()
    {
        this->moveBack(this->moveSpeed);
    }

    void Camera::moveBack(float speed)
    {
        this->position -= this->speedMultiplier * speed * this->direction;
    }

    void Camera::setMoveSpeed(float speed)
    {
        this->moveSpeed = speed;
    }

    void Camera::setRotSpeed(float speed)
    {
        this->rotSpeed = speed;
    }

    glm::mat4 Camera::getProjection()
    {
        return this->projectionMatrix;
    }

    glm::mat4 Camera::getView()
    {
        return this->viewMatrix;
    }

    void Camera::setSpeedMultiplier(float mult)
    {
        this->speedMultiplier = mult;
    }
}
