#ifndef SPEAR_RENDERING_OPENGL_TEXTURE_TEXTURE_HH
#define SPEAR_RENDERING_OPENGL_TEXTURE_TEXTURE_HH

#include <spear/rendering/base_texture.hh>

namespace spear::rendering::opengl
{

class Texture : public BaseTexture
{
public:
    /// Default constructor.
    Texture();

    /// Move constructor.
    Texture(Texture&& other);

    /// Move assignment operator.
    Texture& operator=(Texture&& other);

    /// Deleted copy constructor.
    Texture(const Texture& other) = delete;

    /// Deleted copy assignment operator.
    Texture& operator=(const Texture& other) = delete;

    /// Destructor.
    ~Texture() override
    {
    }

    /// BaseTexture::bind implementation.
    void bind(uint32_t unit = 0) const override;

    /// BaseTexture::bind implementation.
    void unbind(uint32_t unit = 0) override;

    uint32_t getId() const
    {
        return m_texture;
    }

protected:
    uint32_t m_texture;
};

} // namespace spear::rendering::opengl

#endif
