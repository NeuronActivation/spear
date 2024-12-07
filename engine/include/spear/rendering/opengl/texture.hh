#ifndef SPEAR_TEXTURE_HH
#define SPEAR_TEXTURE_HH

#include <spear/rendering/base_texture.hh>
#include <string>

namespace spear::rendering::opengl
{

class Texture : public BaseTexture
{
public:
    /// Constructor.
    /// \param id Shader id.
    Texture(uint32_t id);

    /// Destructor.
    ~Texture();

    /// Move constructor.
    Texture(Texture&& other);

    /// Move assignment operator.
    Texture& operator=(Texture&& other);

    /// Deleted copy constructor.
    Texture(const Texture&) = delete;

    /// Deleted copy assignment.
    Texture& operator=(const Texture&) = delete;

    // Load texture from file.
    bool loadFile(const std::string& path, bool asset_path = true);

    // Bind texture
    void bind(uint32_t unit = 0) const override;

    // Unbind texture
    static void unbind(uint32_t unit = 0);

    // Free texture
    void free();
};

}

#endif
