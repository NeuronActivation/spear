#include <spear/event_handler.hh>

namespace spear
{

/// Process SDL events and call registered callbacks
void EventHandler::handleEvents(MovementController& movement_controller, float delta_time,
                                bool allowMovement)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        auto it = callbacks.find(event.type);
        if (it != callbacks.end())
        {
            it->second(event); // Call the registered callback
        }

        // If no callback was set for SDL_QUIT, set running to false directly.
        if (event.type == SDL_EVENT_QUIT && callbacks.find(SDL_EVENT_QUIT) == callbacks.end())
        {
            exit(0);
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
            keybinds.handleKeyEvent(event.key);
    }

    if (allowMovement)
        movement_controller.processInput(keybinds.movementKeyStates(), delta_time);
}

void EventHandler::handleInput(SDL_Keycode key, const std::function<void()>& callback)
{
    keybinds.bindInput(key, callback);
}

void EventHandler::handleKeyPressed(SDL_Keycode key, const std::function<void()>& callback)
{
    keybinds.bindKeyPressed(key, callback);
}

void EventHandler::clearMovementKeys()
{
    keybinds.clearMovementKeys();
}

} // namespace spear
