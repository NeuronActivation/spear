#include <glm/gtc/matrix_transform.hpp>
#include <spear/camera.hh>

namespace spear
{

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float movement_speed, float mouse_sensitivity, float fov)
    : m_position(position),
      m_worldUp(up),
      m_yaw(yaw),
      m_pitch(pitch),
      m_movementSpeed(movement_speed),
      m_mouseSensitivity(mouse_sensitivity),
      m_fov(fov)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    std::shared_lock lock(m_mutex);
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    std::shared_lock lock(m_mutex);
    return glm::perspective(glm::radians(m_fov), ASPECT_RATIO, 0.1f, 100.0f);
}

glm::vec3 Camera::getPosition() const
{
    std::shared_lock lock(m_mutex);
    return m_position;
}

float Camera::getSpeed() const
{
    std::shared_lock lock(m_mutex);
    return m_movementSpeed;
}

glm::vec3 Camera::getWorldUp() const
{
    std::shared_lock lock(m_mutex);
    return m_worldUp;
}

/// \ingroup CameraGetters
glm::vec3 Camera::getUp() const
{
    std::shared_lock lock(m_mutex);
    return m_up;
}

/// \ingroup CameraGetters
glm::vec3 Camera::getFront() const
{
    std::shared_lock lock(m_mutex);
    return m_front;
}

/// \ingroup CameraGetters
glm::vec3 Camera::getRight() const
{
    std::shared_lock lock(m_mutex);
    return m_right;
}

void Camera::moveForward(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position += m_front * m_movementSpeed * delta_time;
}

void Camera::moveBackward(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position -= m_front * m_movementSpeed * delta_time;
}

void Camera::moveRight(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position += m_right * m_movementSpeed * delta_time;
}

void Camera::moveLeft(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position -= m_right * m_movementSpeed * delta_time;
}

void Camera::moveUp(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position += m_up * m_movementSpeed * delta_time;
}

void Camera::moveDown(float delta_time)
{
    std::shared_lock lock(m_mutex);
    m_position -= m_up * m_movementSpeed * delta_time;
}

void Camera::setPosition(const glm::vec3& newPosition)
{
    std::shared_lock lock(m_mutex);
    m_position = newPosition;
}

void Camera::rotate(float xoffset, float yoffset, bool constrain_pitch)
{
    std::shared_lock lock(m_mutex);
    m_yaw += xoffset * m_mouseSensitivity;
    m_pitch -= yoffset * m_mouseSensitivity;

    if (constrain_pitch)
    {
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::zoom(float yoffset)
{
    m_fov -= yoffset;
    if (m_fov < 1.0f)
        m_fov = 1.0f;
    if (m_fov > 45.0f)
        m_fov = 45.0f;
}

void Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    newFront.y = sin(glm::radians(m_pitch));
    newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(newFront);

    // Recalculate right and up vectors.
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace spear
