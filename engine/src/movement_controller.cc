#include <spear/movement_controller.hh>

#include <btBulletDynamicsCommon.h>

namespace spear
{

MovementController::MovementController(Camera& camera, btDiscreteDynamicsWorld* physicsWorld, float eyeHeight)
    : m_camera(camera),
      m_physicsWorld(physicsWorld),
      m_eyeHeight(eyeHeight)
{
    m_prevKeyStates = {
            {SDLK_W, false}, {SDLK_S, false}, {SDLK_A, false}, {SDLK_D, false}, {SDLK_SPACE, false}};
}

void MovementController::processInput(const std::unordered_map<SDL_Keycode, bool>& keyStates, float deltaTime)
{
    // --- Horizontal movement (projected onto XZ plane) ---
    glm::vec3 front = m_camera.getFront();
    glm::vec3 forward = glm::normalize(glm::vec3(front.x, 0.0f, front.z));

    glm::vec3 rightVec = m_camera.getRight();
    glm::vec3 right = glm::normalize(glm::vec3(rightVec.x, 0.0f, rightVec.z));

    glm::vec3 direction(0.f);
    if (keyStates.at(SDLK_W))
        direction += forward;
    if (keyStates.at(SDLK_S))
        direction -= forward;
    if (keyStates.at(SDLK_A))
        direction -= right;
    if (keyStates.at(SDLK_D))
        direction += right;

    auto camPos = m_camera.getPosition();
    if (glm::length(direction) > 0.0f)
    {
        direction = glm::normalize(direction);
        camPos += direction * m_camera.getSpeed() * deltaTime;
        camPos.y = m_camera.getPosition().y;
        m_camera.setPosition(camPos);
    }

    // --- Jump (edge-triggered on space press) ---
    auto spaceIt = keyStates.find(SDLK_SPACE);
    bool spaceDown = (spaceIt != keyStates.end()) && spaceIt->second;
    bool spaceWasDown = m_prevKeyStates[SDLK_SPACE];

    if (spaceDown && !spaceWasDown && m_onGround)
    {
        m_verticalVelocity = JUMP_SPEED;
        m_onGround = false;
    }

    // --- Apply gravity ---
    if (!m_onGround)
    {
        m_verticalVelocity += GRAVITY * deltaTime;
    }

    // --- Apply vertical movement ---
    camPos = m_camera.getPosition();
    camPos.y += m_verticalVelocity * deltaTime;
    m_camera.setPosition(camPos);

    // --- Ground detection via raycast ---
    m_onGround = false;
    if (m_physicsWorld)
    {
        btVector3 rayFrom(camPos.x, camPos.y, camPos.z);
        btVector3 rayTo(camPos.x, camPos.y - (m_eyeHeight + 5.0f), camPos.z);

        btCollisionWorld::ClosestRayResultCallback cb(rayFrom, rayTo);
        m_physicsWorld->rayTest(rayFrom, rayTo, cb);

        if (cb.hasHit())
        {
            float hitY = cb.m_hitPointWorld.getY();
            float feetY = camPos.y - m_eyeHeight;

            if (feetY <= hitY + 0.1f && m_verticalVelocity <= 0.0f)
            {
                camPos.y = hitY + m_eyeHeight;
                m_camera.setPosition(camPos);
                m_verticalVelocity = 0.0f;
                m_onGround = true;
            }
        }
    }

    // Store previous key states for edge detection
    m_prevKeyStates[SDLK_W] = keyStates.at(SDLK_W);
    m_prevKeyStates[SDLK_S] = keyStates.at(SDLK_S);
    m_prevKeyStates[SDLK_A] = keyStates.at(SDLK_A);
    m_prevKeyStates[SDLK_D] = keyStates.at(SDLK_D);
    m_prevKeyStates[SDLK_SPACE] = spaceDown;
}

} // namespace spear