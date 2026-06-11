#ifndef SPEAR_SCENE_MANAGER_HH
#define SPEAR_SCENE_MANAGER_HH

#include <spear/scene.hh>

namespace spear
{

class SceneManager
{
public:
    SceneManager() = default;

    void addScene(Scene&& scene)
    {
        m_scenes.emplace_back(std::move(scene));
    }

    /// Set m_currentScene.
    /// \param name The name of the scene.
    void loadScene(const std::string& name);

    /// Set m_currentScene.
    /// \param id The id of the scene.
    void loadScene(uint64_t id);

    void createScenes(std::vector<Scene>&& scenes)
    {
        m_scenes = std::move(scenes);
    }

    bool removeObject(const std::string& name);

    Scene* getCurrentScene()
    {
        return m_currentScene;
    }

    Scene* getSceneById(uint64_t id)
    {
        for (auto& scene : m_scenes)
        {
            if (scene.getId() == id)
                return &scene;
        }
        return nullptr;
    }

private:
    Scene* m_currentScene = nullptr;
    std::vector<Scene> m_scenes;
};

} // namespace spear

#endif
