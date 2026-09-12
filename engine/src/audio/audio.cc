#include <spear/audio/audio.hh>

#include <SDL3/SDL.h>

#include <iostream>

namespace spear::audio
{

namespace
{
constexpr std::size_t kMaxVoices = 8;

} // namespace

AudioSystem::~AudioSystem()
{
    if (m_deviceId != 0)
    {
        SDL_CloseAudioDevice(m_deviceId);
        m_deviceId = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool AudioSystem::init()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        std::cerr << "spear::audio::AudioSystem: SDL audio init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Small device buffer so effects start quickly instead of after
    // accumulating a large chunk of samples.
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "256");

    m_deviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (m_deviceId == 0)
    {
        std::cerr << "spear::audio::AudioSystem: could not open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_GetAudioDeviceFormat(m_deviceId, &m_deviceSpec, nullptr))
    {
        std::cerr << "spear::audio::AudioSystem: could not query audio device format: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void AudioSystem::update()
{
    for (auto* sound : m_sounds)
    {
        if (sound)
        {
            sound->updateVoices();
        }
    }
}

void AudioSystem::registerSound(Sound* sound)
{
    m_sounds.push_back(sound);
}

void AudioSystem::unregisterSound(Sound* sound)
{
    for (auto it = m_sounds.begin(); it != m_sounds.end(); ++it)
    {
        if (*it == sound)
        {
            m_sounds.erase(it);
            return;
        }
    }
}

Sound::Sound(AudioSystem& system, const std::string& filepath)
    : m_system(system)
{
    SDL_AudioSpec loaded_spec{};
    Uint8* audio_buf = nullptr;
    Uint32 audio_len = 0;
    if (!SDL_LoadWAV(filepath.c_str(), &loaded_spec, &audio_buf, &audio_len))
    {
        std::cerr << "spear::audio::Sound: failed to load '" << filepath << "': " << SDL_GetError() << std::endl;
        return;
    }

    m_spec = loaded_spec;
    m_data.assign(audio_buf, audio_buf + audio_len);
    SDL_free(audio_buf);

    // Pre-create and bind a small pool of voices so a shot only has to
    // queue bytes, never allocate streams on the hot path. SDL converts
    // to the device format while the device pulls from the stream.
    for (std::size_t i = 0; i < kMaxVoices; ++i)
    {
        Voice voice;
        voice.stream = SDL_CreateAudioStream(&m_spec, &m_system.getDeviceSpec());
        if (!voice.stream)
        {
            std::cerr << "spear::audio::Sound: failed to create voice stream: " << SDL_GetError() << std::endl;
            break;
        }
        SDL_BindAudioStream(m_system.getDeviceId(), voice.stream);
        m_voices.push_back(voice);
    }

    m_system.registerSound(this);
}

Sound::~Sound()
{
    m_system.unregisterSound(this);

    for (auto& voice : m_voices)
    {
        if (voice.stream)
        {
            SDL_UnbindAudioStream(voice.stream);
            SDL_DestroyAudioStream(voice.stream);
        }
    }
}

void Sound::play()
{
    if (m_system.getDeviceId() == 0 || m_data.empty() || m_voices.empty())
    {
        return;
    }

    // Find a finished voice; steal a busy one round-robin if all are playing.
    Voice* voice = nullptr;
    for (auto& v : m_voices)
    {
        if (!v.playing)
        {
            voice = &v;
            break;
        }
    }
    if (!voice)
    {
        voice = &m_voices[m_nextVoice % m_voices.size()];
        m_nextVoice++;
    }

    SDL_ClearAudioStream(voice->stream);
    if (!SDL_PutAudioStreamData(voice->stream, m_data.data(), static_cast<int>(m_data.size())))
    {
        std::cerr << "spear::audio::Sound: failed to queue audio data: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_FlushAudioStream(voice->stream);
    voice->playing = true;
}

void Sound::updateVoices()
{
    for (auto& voice : m_voices)
    {
        if (voice.playing && voice.stream &&
            SDL_GetAudioStreamAvailable(voice.stream) == 0)
        {
            voice.playing = false;
        }
    }
}

} // namespace spear::audio