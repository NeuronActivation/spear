#ifndef SPEAR_AUDIO_AUDIO_HH
#define SPEAR_AUDIO_AUDIO_HH

#include <SDL3/SDL_audio.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace spear::audio
{

class Sound;

/// SDL-powered audio output manager.
/// Owns a single playback device; all Sound effects mix onto it.
class AudioSystem
{
public:
    AudioSystem() = default;
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    /// Open the default playback device.
    /// Returns true on success.
    bool init();

    /// Reclaim finished playback voices. Call once per frame.
    void update();

    SDL_AudioDeviceID getDeviceId() const
    {
        return m_deviceId;
    }
    const SDL_AudioSpec& getDeviceSpec() const
    {
        return m_deviceSpec;
    }

private:
    friend class Sound;

    void registerSound(Sound* sound);
    void unregisterSound(Sound* sound);

    SDL_AudioDeviceID m_deviceId = 0;
    SDL_AudioSpec m_deviceSpec{};
    std::vector<Sound*> m_sounds;
};

/// A loaded sound clip that can be played, overlapping with itself.
class Sound
{
public:
    /// Load a WAV file (see SDL_LoadWAV).
    Sound(AudioSystem& system, const std::string& filepath);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    Sound(Sound&&) = delete;
    Sound& operator=(Sound&&) = delete;

    /// Play the clip; safe to call repeatedly for rapid fire.
    void play();

    /// Set the clip volume (0.0 = silent, 1.0 = full). Applied to all voices.
    void setVolume(float volume);

    float getVolume() const
    {
        return m_volume;
    }

private:
    friend class AudioSystem;

    struct Voice
    {
        SDL_AudioStream* stream = nullptr;
        bool playing = false;
    };

    void updateVoices();

    AudioSystem& m_system;
    SDL_AudioSpec m_spec{};
    std::vector<std::uint8_t> m_data;
    std::vector<Voice> m_voices;
    std::size_t m_nextVoice = 0;
    float m_volume = 1.0f;
};

} // namespace spear::audio

#endif