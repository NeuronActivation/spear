#ifndef SPEAR_AUDIO_AUDIO_HH
#define SPEAR_AUDIO_AUDIO_HH

#include <SDL3/SDL_audio.h>

#include <glm/glm.hpp>

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

    /// Set the audio listener (player ear) position and orientation.
    /// Should be called once per frame with the camera state.
    void setListener(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up);

    const glm::vec3& getListenerPosition() const
    {
        return m_listenerPos;
    }
    const glm::vec3& getListenerFront() const
    {
        return m_listenerFront;
    }
    const glm::vec3& getListenerRight() const
    {
        return m_listenerRight;
    }

    int getDeviceChannels() const
    {
        return m_deviceSpec.channels;
    }

private:
    friend class Sound;

    void registerSound(Sound* sound);
    void unregisterSound(Sound* sound);

    SDL_AudioDeviceID m_deviceId = 0;
    SDL_AudioSpec m_deviceSpec{};
    std::vector<Sound*> m_sounds;

    glm::vec3 m_listenerPos{0.0f};
    glm::vec3 m_listenerFront{0.0f, 0.0f, -1.0f};
    glm::vec3 m_listenerUp{0.0f, 1.0f, 0.0f};
    glm::vec3 m_listenerRight{1.0f, 0.0f, 0.0f};
};

/// A loaded sound clip that can be played, overlapping with itself.
class Sound
{
public:
    /// Load a sound file. WAV clips use SDL_LoadWAV; everything else
    /// (MP3/OGG/FLAC) is decoded through SDL_mixer.
    Sound(AudioSystem& system, const std::string& filepath);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    Sound(Sound&&) = delete;
    Sound& operator=(Sound&&) = delete;

    /// Play the clip centered on the listener (non-spatial); safe to call
    /// repeatedly for rapid fire.
    void play();

    /// Play the clip at a world position with optional duration limit.
    /// durationSeconds > 0 truncates the clip (useful for short footstep
    /// snippets). 0 = play the whole clip.
    void playAt(const glm::vec3& position, float durationSeconds = 0.0f);

    /// Stop every currently playing voice of this clip immediately.
    void stop();

    /// True while at least one voice is still playing the clip.
    bool isPlaying() const;

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
        SDL_AudioStream* streamL = nullptr;
        SDL_AudioStream* streamR = nullptr;
        bool playing = false;
        float lastGainL = 1.0f;
        float lastGainR = 1.0f;
    };

    void updateVoices();
    void applySpatial(Voice& voice, const glm::vec3& position, float durationSeconds);
    std::size_t bytesPerSecond() const;

    AudioSystem& m_system;
    SDL_AudioSpec m_spec{};
    std::vector<std::uint8_t> m_data;
    std::vector<Voice> m_voices;
    std::size_t m_nextVoice = 0;
    float m_volume = 1.0f;
};

} // namespace spear::audio

#endif
