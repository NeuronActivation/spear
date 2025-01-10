#include <spear/rendering/base_model.hh>

#include <GL/glew.h>

namespace spear::rendering
{

BaseModel::BaseModel(std::shared_ptr<rendering::BaseShader> shader, physics::bullet::ObjectData&& object_data)
    : GameObject(std::move(object_data), shader)
{
}

} // namespace spear::rendering
