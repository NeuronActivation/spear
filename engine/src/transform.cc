#include <spear/transform.hh>

namespace spear
{

Transform::Transform()
    : m_model(glm::mat4(1))
{
}

Transform::Transform(const Transform& other)
    : m_model(other.m_model)
{
}

Transform::Transform(Transform&& other)
    : m_model(std::move(other.m_model))
{
}

Transform& Transform::operator=(const Transform& other)
{
    if (this != &other)
    {
        m_model = other.m_model;
    }
    return *this;
}

Transform& Transform::operator=(Transform&& other)
{
    if (this != &other)
    {
        m_model = std::move(other.m_model);
    }
    return *this;
}

void Transform::setPosition(const glm::vec3& position)
{
    m_model = glm::translate(glm::mat4(1.0f), position);
}

void Transform::translate(const glm::vec3& position)
{
    m_model = glm::translate(m_model, position);
}

void Transform::rotate(float speed, const glm::vec3& direction)
{
    m_model = glm::rotate(m_model, speed, direction);
}

void Transform::scale(const glm::vec3& scale)
{
    m_model = glm::scale(m_model, scale);
}

glm::vec3 Transform::getPosition() const
{
    return glm::vec3(m_model[3][0], m_model[3][1], m_model[3][2]);
}

} // namespace spear
