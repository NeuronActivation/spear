#include <spear/keybinds.hh>

#include <utility>

namespace spear
{

void Keybinds::bindInput(SDL_Keycode key, Callback callback)
{
    m_inputKeys.emplace_back(key, std::move(callback));
}

void Keybinds::bindKeyPressed(SDL_Keycode key, Callback callback)
{
    m_keyPressedCallbacks.emplace_back(key, std::move(callback));
}

void Keybinds::handleKeyEvent(const SDL_KeyboardEvent& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP)
        return;

    for (const auto& [key, callback] : m_inputKeys)
    {
        if (event.key == key)
            callback();
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        for (const auto& [key, callback] : m_keyPressedCallbacks)
        {
            if (event.key == key)
                callback();
        }
    }

    auto it = m_movementKeyStates.find(event.key);
    if (it != m_movementKeyStates.end())
        it->second = event.type == SDL_EVENT_KEY_DOWN;
}

void Keybinds::clearMovementKeys()
{
    for (auto& [key, state] : m_movementKeyStates)
        state = false;
}

bool Keybinds::movementKeyDown(SDL_Keycode key) const
{
    const auto it = m_movementKeyStates.find(key);
    return it != m_movementKeyStates.end() && it->second;
}

const std::unordered_map<SDL_Keycode, bool>& Keybinds::movementKeyStates() const
{
    return m_movementKeyStates;
}

} // namespace spear
