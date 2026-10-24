#include <spear/movement_controller.hh>
#include <spear/keybinds.hh>

#include <btBulletDynamicsCommon.h>

#include <algorithm>
#include <array>
#include <optional>

namespace spear
{

namespace
{
constexpr float kGroundNormalY = 0.65f;
constexpr float kCollisionMargin = 0.01f;

struct ContactCheck : btCollisionWorld::ContactResultCallback
{
    bool blocked = false;

    btScalar addSingleResult(btManifoldPoint&, const btCollisionObjectWrapper*, int, int,
                             const btCollisionObjectWrapper*, int, int) override
    {
        blocked = true;
        return 0.0f;
    }
};

std::optional<float> groundYAt(btDiscreteDynamicsWorld& world, float x, float z,
                               float fromY, float toY)
{
    const btVector3 from(x, fromY, z);
    const btVector3 to(x, toY, z);
    btCollisionWorld::ClosestRayResultCallback hit(from, to);
    world.rayTest(from, to, hit);
    if (!hit.hasHit() || hit.m_hitNormalWorld.y() < kGroundNormalY)
        return std::nullopt;
    return hit.m_hitPointWorld.y();
}
} // namespace

MovementController::MovementController(Camera& camera, btDiscreteDynamicsWorld* physicsWorld, float eyeHeight, float gravity, float jumpSpeed)
    : m_camera(camera),
      m_physicsWorld(physicsWorld),
      m_eyeHeight(eyeHeight),
      m_currentEyeHeight(eyeHeight),
      m_gravity(gravity),
      m_jumpSpeed(jumpSpeed)
{
    m_prevKeyStates = {
            {movement_keys::forward, false}, {movement_keys::backward, false},
            {movement_keys::left, false}, {movement_keys::right, false},
            {movement_keys::jump, false}};
}

void MovementController::setMovementModifiers(float crouchEyeHeight, float crouchSpeedScale,
                                              float silentWalkSpeedScale)
{
    m_crouchEyeHeight = std::clamp(crouchEyeHeight, 0.0f, m_eyeHeight);
    m_crouchSpeedScale = std::clamp(crouchSpeedScale, 0.0f, 1.0f);
    m_silentWalkSpeedScale = std::clamp(silentWalkSpeedScale, 0.0f, 1.0f);
}

void MovementController::resetStance()
{
    m_crouched = false;
    m_currentEyeHeight = m_eyeHeight;
}

void MovementController::processInput(const std::unordered_map<SDL_Keycode, bool>& keyStates, float deltaTime)
{
    auto keyDown = [&keyStates](SDL_Keycode key)
    {
        const auto it = keyStates.find(key);
        return it != keyStates.end() && it->second;
    };
    const bool crouchRequested = keyDown(movement_keys::crouchLeft) ||
                                 keyDown(movement_keys::crouchRight);
    const bool silentWalk = keyDown(movement_keys::silentWalkLeft) ||
                            keyDown(movement_keys::silentWalkRight);
    if (m_crouchEyeHeight > 0.0f && crouchRequested && !m_crouched)
    {
        auto position = m_camera.getPosition();
        position.y -= m_eyeHeight - m_crouchEyeHeight;
        m_camera.setPosition(position);
        m_currentEyeHeight = m_crouchEyeHeight;
        m_crouched = true;
    }
    else if (m_crouched && !crouchRequested)
    {
        bool canStand = true;
        if (m_physicsWorld)
        {
            const float radius = m_eyeHeight * 0.12f;
            const float totalHeight = m_eyeHeight * 0.9f;
            const float feetY = m_camera.getPosition().y - m_currentEyeHeight;
            btCapsuleShape capsule(radius, totalHeight - 2.0f * radius);
            btCollisionObject standing;
            standing.setCollisionShape(&capsule);
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(m_camera.getPosition().x,
                                          feetY + m_eyeHeight * 0.15f + totalHeight * 0.5f,
                                          m_camera.getPosition().z));
            standing.setWorldTransform(transform);
            ContactCheck contacts;
            m_physicsWorld->contactTest(&standing, contacts);
            canStand = !contacts.blocked;
        }
        if (canStand)
        {
            auto position = m_camera.getPosition();
            position.y += m_eyeHeight - m_currentEyeHeight;
            m_camera.setPosition(position);
            m_currentEyeHeight = m_eyeHeight;
            m_crouched = false;
        }
    }

    // --- Horizontal movement (projected onto XZ plane) ---
    glm::vec3 front = m_camera.getFront();
    glm::vec3 forward = glm::normalize(glm::vec3(front.x, 0.0f, front.z));

    glm::vec3 rightVec = m_camera.getRight();
    glm::vec3 right = glm::normalize(glm::vec3(rightVec.x, 0.0f, rightVec.z));

    glm::vec3 direction(0.f);
    if (keyStates.at(movement_keys::forward))
        direction += forward;
    if (keyStates.at(movement_keys::backward))
        direction -= forward;
    if (keyStates.at(movement_keys::left))
        direction -= right;
    if (keyStates.at(movement_keys::right))
        direction += right;

    auto camPos = m_camera.getPosition();
    const float maxStepHeight = m_eyeHeight * 0.375f;
    const float radius = m_eyeHeight * 0.12f;
    const float supportOffset = radius * 0.9f;
    if (glm::length(direction) > 0.0f)
    {
        direction = glm::normalize(direction);
        const float speedScale = m_crouched ? m_crouchSpeedScale : silentWalk ? m_silentWalkSpeedScale
                                                                              : 1.0f;
        glm::vec3 targetPos = camPos + direction * m_camera.getSpeed() * speedScale * deltaTime;
        targetPos.y = camPos.y;

        // --- Horizontal collision check (prevent walking through walls) ---
        if (m_physicsWorld)
        {
            const float totalHeight = m_currentEyeHeight * 0.9f;
            const float halfHeight = totalHeight * 0.5f;
            const float bottomClearance = m_currentEyeHeight * 0.15f;

            // Capsule spanning from slightly above the feet up to the head.
            btCapsuleShape capsule(radius, totalHeight - 2.0f * radius);

            auto capsuleOriginY = [this, halfHeight, bottomClearance](float headY)
            {
                return headY - m_currentEyeHeight + bottomClearance + halfHeight;
            };

            btTransform from;
            from.setIdentity();
            from.setOrigin(btVector3(camPos.x, capsuleOriginY(camPos.y), camPos.z));
            btTransform to;
            to.setIdentity();
            to.setOrigin(btVector3(targetPos.x, capsuleOriginY(targetPos.y), targetPos.z));

            auto sweepCapsule = [this, &capsule](const btTransform& start, const btTransform& end,
                                                 btCollisionWorld::ClosestConvexResultCallback& result)
            {
                m_physicsWorld->convexSweepTest(&capsule, start, end, result);
            };

            btCollisionWorld::ClosestConvexResultCallback sweep(from.getOrigin(), to.getOrigin());
            sweepCapsule(from, to, sweep);

            if (sweep.hasHit())
            {
                bool stepped = false;
                if (m_onGround)
                {
                    const btVector3 lift(0.0f, maxStepHeight, 0.0f);
                    btTransform raisedFrom = from;
                    raisedFrom.setOrigin(from.getOrigin() + lift);
                    btTransform raisedTo = to;
                    raisedTo.setOrigin(to.getOrigin() + lift);

                    btCollisionWorld::ClosestConvexResultCallback liftSweep(from.getOrigin(), raisedFrom.getOrigin());
                    sweepCapsule(from, raisedFrom, liftSweep);
                    if (!liftSweep.hasHit())
                    {
                        btCollisionWorld::ClosestConvexResultCallback raisedSweep(
                                raisedFrom.getOrigin(), raisedTo.getOrigin());
                        sweepCapsule(raisedFrom, raisedTo, raisedSweep);
                        if (!raisedSweep.hasHit())
                        {
                            const float rayTop = camPos.y + maxStepHeight;
                            const float rayBottom = camPos.y - m_currentEyeHeight - 1.0f;
                            auto ground = groundYAt(*m_physicsWorld, targetPos.x, targetPos.z,
                                                    rayTop, rayBottom);
                            auto leadingGround = groundYAt(
                                    *m_physicsWorld,
                                    targetPos.x + direction.x * supportOffset,
                                    targetPos.z + direction.z * supportOffset,
                                    rayTop, rayBottom);
                            if (leadingGround && (!ground || *leadingGround > *ground))
                                ground = leadingGround;
                            if (ground)
                            {
                                const float rise = *ground - (camPos.y - m_currentEyeHeight);
                                if (rise > 0.1f && rise <= maxStepHeight + 0.1f)
                                {
                                    targetPos.y = *ground + m_currentEyeHeight;
                                    m_verticalVelocity = 0.0f;
                                    stepped = true;
                                }
                            }
                        }
                    }
                }

                if (!stepped)
                {
                    const float fraction = std::max(0.0f, sweep.m_closestHitFraction - kCollisionMargin);
                    const glm::vec3 remaining = targetPos - camPos;
                    targetPos = camPos + remaining * fraction;

                    // Continue along a wall instead of losing all movement
                    // when only one component of the direction is blocked.
                    btVector3 wallNormal = sweep.m_hitNormalWorld;
                    wallNormal.setY(0.0f);
                    if (wallNormal.length2() > 0.01f)
                    {
                        wallNormal.normalize();
                        glm::vec3 slide = remaining * (1.0f - fraction);
                        const float intoWall = slide.x * wallNormal.x() + slide.z * wallNormal.z();
                        if (intoWall < 0.0f)
                        {
                            slide.x -= intoWall * wallNormal.x();
                            slide.z -= intoWall * wallNormal.z();
                        }
                        btTransform slideFrom = from;
                        slideFrom.setOrigin(btVector3(targetPos.x, capsuleOriginY(targetPos.y), targetPos.z));
                        btTransform slideTo = slideFrom;
                        slideTo.setOrigin(slideFrom.getOrigin() + btVector3(slide.x, 0.0f, slide.z));
                        btCollisionWorld::ClosestConvexResultCallback slideSweep(
                                slideFrom.getOrigin(), slideTo.getOrigin());
                        sweepCapsule(slideFrom, slideTo, slideSweep);
                        const float slideFraction = slideSweep.hasHit()
                                                            ? std::max(0.0f, slideSweep.m_closestHitFraction - kCollisionMargin)
                                                            : 1.0f;
                        targetPos += slide * slideFraction;
                    }
                }
            }
        }

        m_camera.setPosition(targetPos);
    }

    // --- Jump (edge-triggered on space press) ---
    auto spaceIt = keyStates.find(movement_keys::jump);
    bool spaceDown = (spaceIt != keyStates.end()) && spaceIt->second;
    bool spaceWasDown = m_prevKeyStates[movement_keys::jump];

    if (spaceDown && !spaceWasDown && m_onGround && !m_crouched)
    {
        m_verticalVelocity = m_jumpSpeed;
        m_onGround = false;
    }

    // --- Apply gravity ---
    if (!m_onGround)
    {
        m_verticalVelocity += m_gravity * deltaTime;
    }

    // --- Apply vertical movement ---
    camPos = m_camera.getPosition();
    const float previousHeadY = camPos.y;
    camPos.y += m_verticalVelocity * deltaTime;
    m_camera.setPosition(camPos);

    // --- Ground detection via raycast ---
    const bool wasGrounded = m_onGround;
    m_onGround = false;
    if (m_physicsWorld)
    {
        // Probe the capsule's center and edges. The leading edge can be on a
        // stair while the center is still over the lower tread.
        const std::array<glm::vec3, 5> probes{{{0.0f, 0.0f, 0.0f},
                                               {supportOffset, 0.0f, 0.0f},
                                               {-supportOffset, 0.0f, 0.0f},
                                               {0.0f, 0.0f, supportOffset},
                                               {0.0f, 0.0f, -supportOffset}}};
        std::optional<float> ground;
        for (const auto& offset : probes)
        {
            // Start at the previous height so a long frame cannot move the
            // eye below a floor before the ray finds it.
            auto hit = groundYAt(*m_physicsWorld, camPos.x + offset.x,
                                 camPos.z + offset.z,
                                 std::max(previousHeadY, camPos.y),
                                 camPos.y - m_currentEyeHeight - maxStepHeight);

            const float allowedRise = wasGrounded ? maxStepHeight + 0.1f : 0.1f;
            if (hit && *hit <= previousHeadY - m_currentEyeHeight + allowedRise &&

                (!ground || *hit > *ground))
                ground = hit;
        }

        const float feetY = camPos.y - m_currentEyeHeight;
        if (ground && m_verticalVelocity <= 0.0f &&
            (feetY <= *ground + 0.1f ||
             (wasGrounded && feetY - *ground <= maxStepHeight)))
        {
            camPos.y = *ground + m_currentEyeHeight;
            m_camera.setPosition(camPos);
            m_verticalVelocity = 0.0f;
            m_onGround = true;
        }
    }

    // Store previous key states for edge detection
    m_prevKeyStates[movement_keys::forward] = keyStates.at(movement_keys::forward);
    m_prevKeyStates[movement_keys::backward] = keyStates.at(movement_keys::backward);
    m_prevKeyStates[movement_keys::left] = keyStates.at(movement_keys::left);
    m_prevKeyStates[movement_keys::right] = keyStates.at(movement_keys::right);
    m_prevKeyStates[movement_keys::jump] = spaceDown;
}

} // namespace spear
