#ifndef SPEAR_MOVEMENT_CONTROLLER_HH
#define SPEAR_MOVEMENT_CONTROLLER_HH

#include <spear/camera.hh>

#include <SDL3/SDL_keycode.h>
#include <glm/vec3.hpp>

#include <unordered_map>

class btDiscreteDynamicsWorld;

namespace spear
{

class MovementController
{
public:
    MovementController(Camera& camera, btDiscreteDynamicsWorld* physicsWorld = nullptr);

    void processInput(const std::unordered_map<SDL_Keycode, bool>& keyStates, float deltaTime);

    void setPhysicsWorld(btDiscreteDynamicsWorld* world)
    {
        m_physicsWorld = world;
    }
    bool isOnGround() const
    {
        return m_onGround;
    }
    float getVerticalVelocity() const
    {
        return m_verticalVelocity;
    }

private:
    Camera& m_camera;
    btDiscreteDynamicsWorld* m_physicsWorld;
    float m_verticalVelocity = 0.0f;
    bool m_onGround = true;
    std::unordered_map<SDL_Keycode, bool> m_prevKeyStates;

    static constexpr float GRAVITY = -30.0f;
    static constexpr float JUMP_SPEED = 10.0f;
    static constexpr float EYE_HEIGHT = 3.0f;
};

} // namespace spear

#endif
