#ifndef SPEAR_KEYBINDS_HH
#define SPEAR_KEYBINDS_HH

#include <SDL3/SDL_events.h>

#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace spear
{

namespace movement_keys
{
constexpr SDL_Keycode forward = SDLK_W;
constexpr SDL_Keycode backward = SDLK_S;
constexpr SDL_Keycode left = SDLK_A;
constexpr SDL_Keycode right = SDLK_D;
constexpr SDL_Keycode jump = SDLK_SPACE;
constexpr SDL_Keycode silentWalkLeft = SDLK_LSHIFT;
constexpr SDL_Keycode silentWalkRight = SDLK_RSHIFT;
constexpr SDL_Keycode crouchLeft = SDLK_LCTRL;
constexpr SDL_Keycode crouchRight = SDLK_RCTRL;
} // namespace movement_keys

class Keybinds
{
public:
    using Callback = std::function<void()>;

    /// Bind a callback to both key down and key up events.
    void bindInput(SDL_Keycode key, Callback callback);

    /// Bind a callback to key down events.
    void bindKeyPressed(SDL_Keycode key, Callback callback);

    /// Dispatch callbacks and update movement state for a keyboard event.
    void handleKeyEvent(const SDL_KeyboardEvent& event);

    void clearMovementKeys();
    bool movementKeyDown(SDL_Keycode key) const;
    const std::unordered_map<SDL_Keycode, bool>& movementKeyStates() const;

private:
    // clang-format off
    std::unordered_map<SDL_Keycode, bool> m_movementKeyStates = {
            {movement_keys::forward, false}, {movement_keys::backward, false},
            {movement_keys::left, false}, {movement_keys::right, false},
            {movement_keys::jump, false}, {movement_keys::silentWalkLeft, false},
            {movement_keys::silentWalkRight, false}, {movement_keys::crouchLeft, false},
            {movement_keys::crouchRight, false}};
    std::vector<std::pair<SDL_Keycode, Callback>> m_inputKeys;
    std::vector<std::pair<SDL_Keycode, Callback>> m_keyPressedCallbacks;
    // clang-format on
};

} // namespace spear

#endif
