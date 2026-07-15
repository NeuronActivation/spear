#ifndef SPEAR_RENDERING_BASE_MODEL_HH
#define SPEAR_RENDERING_BASE_MODEL_HH

#include <spear/game_object.hh>
#include <spear/rendering/base_shader.hh>

#include <spear/physics/bullet/object_data.hh>

namespace spear::rendering
{

class BaseModel : public GameObject
{
public:
    /// Constructor.
    BaseModel(std::shared_ptr<rendering::BaseShader> shader, physics::bullet::ObjectData&& object_data);

    virtual void initialize() = 0;
};

} // namespace spear::rendering

#endif
