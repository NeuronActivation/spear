#ifndef SPEAR_PHYSICS_BULLET_OBJECT_HH
#define SPEAR_PHYSICS_BULLET_OBJECT_HH

#include <spear/physics/bullet/object_data.hh>
#include <spear/physics/physics_object.hh>

#include <btBulletDynamicsCommon.h>

#include <glm/vec3.hpp>

#include <memory>

namespace spear::physics::bullet
{

class Object : public PhysicsObject
{
public:
    /// Constructor.
    Object(ObjectData&& object_data);

    /// Destructor.
    ~Object();

    /// Move constuctor.
    Object(Object&& other);

    /// Move assigment operator.
    Object& operator=(Object&& other);

    /// Deleted copy constructor.
    Object(const Object&) = delete;

    /// Copy copy assigment operator.
    Object& operator=(const Object& other) = delete;

    void applyForce(const btVector3& force);
    void applyGravity();

    /// Replace the collision box with the given half-extents, keeping the
    /// visual transform (driven by ObjectData::getSize) unchanged.
    void setCollisionSize(const btVector3& half_extents);

    btVector3 getPosition() const;

    btRigidBody* getRigidBody() const
    {
        return m_rigidBody.get();
    }

private:
    btDiscreteDynamicsWorld* m_dynamicsWorld;
    std::unique_ptr<btCollisionShape> m_collisionShape;
    std::unique_ptr<btDefaultMotionState> m_motionState;
    std::unique_ptr<btRigidBody> m_rigidBody;
};

} // namespace spear::physics::bullet

#endif
