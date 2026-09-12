#include "spear/transform.hh"
#include <spear/physics/bullet/object.hh>
#include <spear/physics/constants.hh>

#include <iostream>

namespace spear::physics::bullet
{

Object::Object(ObjectData&& object_data)
    : PhysicsObject(object_data.getMass()),
      m_dynamicsWorld(object_data.getWorld()),
      m_collisionShape(std::make_unique<btBoxShape>(object_data.getSizeAsBulletVec3()))
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(object_data.getPositionAsBulletVec3());

    // Calculate the local inertia
    btVector3 localInertia(0, 0, 0);
    if (object_data.getMass() != 0.0f)
    {
        m_collisionShape->calculateLocalInertia(object_data.getMass(), localInertia);
    }

    // Create the motion state
    m_motionState = std::make_unique<btDefaultMotionState>(transform);

    // Rigid body.
    btRigidBody::btRigidBodyConstructionInfo rbInfo(object_data.getMass(), m_motionState.get(), m_collisionShape.get(), localInertia);
    m_rigidBody = std::make_unique<btRigidBody>(rbInfo);
    m_dynamicsWorld->addRigidBody(m_rigidBody.get());

    // Sync the render transform with the initial physics position.
    auto pos = object_data.getPosition();
    Transform::translate(pos);
    Transform::scale(object_data.getSize());
}

Object::Object(Object&& other)
    : PhysicsObject(std::move(other)),
      m_dynamicsWorld(other.m_dynamicsWorld),
      m_collisionShape(std::move(other.m_collisionShape)),
      m_motionState(std::move(other.m_motionState)),
      m_rigidBody(std::move(other.m_rigidBody))
{
    other.m_dynamicsWorld = nullptr;
}

Object& Object::operator=(Object&& other)
{
    if (this != &other)
    {
        PhysicsObject::operator=(std::move(other)),
        m_dynamicsWorld = other.m_dynamicsWorld;
        other.m_dynamicsWorld = nullptr;
        m_collisionShape = std::move(other.m_collisionShape);
        m_motionState = std::move(other.m_motionState);
        m_rigidBody = std::move(other.m_rigidBody);
    }
    return *this;
}

Object::~Object()
{
    if (m_dynamicsWorld && m_rigidBody)
    {
        m_dynamicsWorld->removeRigidBody(m_rigidBody.get());
    }
}

void Object::applyForce(const btVector3& force)
{
    if (m_rigidBody)
    {
        m_rigidBody->applyCentralForce(force);
    }
}

void Object::applyGravity()
{
    if (m_rigidBody && !m_rigidBody->isStaticObject())
    {
        auto gravity = Constants::getGravity();
        btVector3 currentGravity = btVector3(gravity.x, gravity.y, gravity.z);
        m_rigidBody->applyCentralForce(currentGravity);

        btVector3 position = getPosition();
        Transform::translate(glm::vec3(position.x(), position.y(), position.z()));
    }
}

void Object::setCollisionSize(const btVector3& half_extents)
{
    if (!m_rigidBody)
        return;

    m_collisionShape = std::make_unique<btBoxShape>(half_extents);

    btVector3 localInertia(0, 0, 0);
    if (m_mass != 0.0f)
        m_collisionShape->calculateLocalInertia(m_mass, localInertia);

    m_rigidBody->setCollisionShape(m_collisionShape.get());
    m_rigidBody->setMassProps(m_mass, localInertia);
    m_rigidBody->updateInertiaTensor();
    m_rigidBody->activate();
}

btVector3 Object::getPosition() const
{
    btTransform transform;
    m_rigidBody->getMotionState()->getWorldTransform(transform);
    return transform.getOrigin();
}

} // namespace spear::physics::bullet
