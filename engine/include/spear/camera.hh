#ifndef SPEAR_CAMERA_HH
#define SPEAR_CAMERA_HH

#include <glm/mat4x2.hpp>
#include <glm/vec3.hpp>

#include <SDL3/SDL_keycode.h>
#include <shared_mutex>

namespace spear
{

class Camera
{
public:
    const float ASPECT_RATIO = 16.f / 9.f;

    /// Constructor.
    Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
           glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
           float yaw = -90.f,
           float pitch = 0.f,
           float movement_speed = 10.f,
           float mouse_sentitivity = 0.1f,
           float fov = 45.0f);

    /// \defgroup CameraGetters.

    /// \ingroup CameraGetters
    glm::mat4 getViewMatrix() const;

    /// \ingroup CameraGetters
    glm::mat4 getProjectionMatrix() const;

    /// \ingroup CameraGetters
    glm::vec3 getPosition() const;

    /// \ingroup CameraGetters
    float getSpeed() const;

    /// \ingroup CameraGetters
    glm::vec3 getWorldUp() const;

    /// \ingroup CameraGetters
    glm::vec3 getUp() const;

    /// \ingroup CameraGetters
    glm::vec3 getFront() const;

    /// \ingroup CameraGetters
    glm::vec3 getRight() const;

    /// \ingroup CameraMove
    void moveForward(float delta_time);

    /// \ingroup CameraMove
    void moveBackward(float delta_time);

    /// \ingroup CameraMove
    void moveRight(float delta_time);

    /// \ingroup CameraMove
    void moveLeft(float delta_time);

    /// \ingroup CameraMove
    void moveUp(float delta_time);

    /// \ingroup CameraMove
    void moveDown(float delta_time);

    /// \ingroup CameraMove
    void rotate(float x_offset, float y_offset, bool constrain_pitch = true);

    // Zoom fov.
    void zoom(float y_offset);

    void setPosition(const glm::vec3& newPosition);

private:
    // Updates front, right, and up vectors based on updated Euler angles
    void updateCameraVectors();

private:
    // Camera Attributes.
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    // Euler Angles.
    float m_yaw;
    float m_pitch;

    // Camera Options.
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_fov;

    mutable std::shared_mutex m_mutex;
};

} // namespace spear

#endif
