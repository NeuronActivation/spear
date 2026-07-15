#include <iostream>
#include <spear/game_object.hh>
#include <spear/util.hh>

namespace spear
{

GameObject::GameObject(physics::bullet::ObjectData&& object_data, std::shared_ptr<rendering::BaseShader> shader)
    : physics::bullet::Object(std::move(object_data)),
      Mesh(shader),
      m_id(Util::generateRandomUint64())
{
}

GameObject::GameObject(GameObject&& other)
    : physics::bullet::Object(std::move(other)),
      Mesh(std::move(other)),
      m_id(std::move(other.m_id))
{
}

GameObject& GameObject::operator=(GameObject&& other)
{
    if (this != &other)
    {
        physics::bullet::Object::operator=(std::move(other));
        Mesh::operator=(std::move(other));
        m_id = std::move(other.m_id);
    }
    return *this;
}

void GameObject::updateGameObject()
{
    auto og_pos = Transform::getPosition();
    std::cout << "Transform x:" << og_pos.x << " y: " << og_pos.y << " z: " << og_pos.z << std::endl;

    auto pos = physics::bullet::Object::getPosition();
    std::cout << "Transform x:" << pos.x() << " y: " << pos.y() << " z: " << pos.z() << std::endl;

    // glm::vec3 deltaPosition(pos.x() - og_pos.x, pos.y() - og_pos.y, pos.z() - og_pos.z);
    // std::cout << "Delta x:" << deltaPosition.x << " y: " << deltaPosition.y << " z: "<< deltaPosition.z << std::endl;
    Transform::translate(glm::vec3(pos.x(), pos.y(), pos.z()));
}

} // namespace spear
