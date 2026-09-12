#include <glm/gtc/matrix_transform.hpp>
#include <spear/camera.hh>

#include <algorithm>
#include <cmath>
#include <mutex>

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
    // Apply the recoil aim-punch to the view direction so the world kicks
    // without permanently altering yaw/pitch.
    glm::mat4 recoil = recoilRotation();
    glm::vec3 front = glm::normalize(glm::vec3(recoil * glm::vec4(m_front, 0.0f)));
    glm::vec3 up = glm::normalize(glm::vec3(recoil * glm::vec4(m_up, 0.0f)));
    return glm::lookAt(m_position, m_position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    std::shared_lock lock(m_mutex);
    return glm::perspective(glm::radians(m_fov), ASPECT_RATIO, 1.0f, 10000.0f);
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
    if (m_recoilPitch == 0.0f && m_recoilYaw == 0.0f)
        return m_up;
    return glm::normalize(glm::vec3(recoilRotation() * glm::vec4(m_up, 0.0f)));
}

/// \ingroup CameraGetters
glm::vec3 Camera::getFront() const
{
    std::shared_lock lock(m_mutex);
    if (m_recoilPitch == 0.0f && m_recoilYaw == 0.0f)
        return m_front;
    return glm::normalize(glm::vec3(recoilRotation() * glm::vec4(m_front, 0.0f)));
}

/// \ingroup CameraGetters
glm::vec3 Camera::getRight() const
{
    std::shared_lock lock(m_mutex);
    if (m_recoilPitch == 0.0f && m_recoilYaw == 0.0f)
        return m_right;
    return glm::normalize(glm::vec3(recoilRotation() * glm::vec4(m_right, 0.0f)));
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

glm::mat4 Camera::recoilRotation() const
{
    glm::mat4 recoil = glm::rotate(glm::mat4(1.0f), glm::radians(m_recoilPitch), m_right);
    recoil = glm::rotate(recoil, glm::radians(m_recoilYaw), m_worldUp);
    return recoil;
}

void Camera::addRecoilOffset(float pitch, float yaw)
{
    std::unique_lock lock(m_mutex);
    m_recoilPitch += pitch;
    m_recoilYaw += yaw;
    if (m_recoilPitch > 6.0f)
        m_recoilPitch = 6.0f;
    if (m_recoilPitch < -6.0f)
        m_recoilPitch = -6.0f;
    if (m_recoilYaw > 4.0f)
        m_recoilYaw = 4.0f;
    if (m_recoilYaw < -4.0f)
        m_recoilYaw = -4.0f;
}

void Camera::updateRecoil(float delta_time)
{
    std::unique_lock lock(m_mutex);
    float decay = std::max(0.0f, 1.0f - 9.0f * delta_time);
    m_recoilPitch *= decay;
    m_recoilYaw *= decay;
    if (std::abs(m_recoilPitch) < 0.01f)
        m_recoilPitch = 0.0f;
    if (std::abs(m_recoilYaw) < 0.01f)
        m_recoilYaw = 0.0f;
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
