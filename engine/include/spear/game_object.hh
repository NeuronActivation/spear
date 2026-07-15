#ifndef SPEAR_GAME_OBJECT_HH
#define SPEAR_GAME_OBJECT_HH

#include <spear/mesh.hh>
#include <spear/physics/bullet/object.hh>

#include <memory>

namespace spear
{

class GameObject
    : public physics::bullet::Object,
      public Mesh,
      public std::enable_shared_from_this<GameObject>
{
public:
    /// Constructor.
    explicit GameObject(physics::bullet::ObjectData&& object_data, std::shared_ptr<rendering::BaseShader> shader);

    /// Move constructor.
    GameObject(GameObject&& other);

    /// Move assignment operator.
    GameObject& operator=(GameObject&& other);

    /// Deleted copy constructor.
    GameObject(const GameObject&) = delete;

    /// Deleted copy assignment operator.
    GameObject& operator=(const GameObject&) = delete;

    void updateGameObject();

    uint64_t getId() const
    {
        return m_id;
    }

private:
    uint64_t m_id;
};

} // namespace spear

#endif
