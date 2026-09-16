#include <spear/audio/audio.hh>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace spear::audio
{

namespace
{
constexpr std::size_t kMaxVoices = 8;
constexpr int kDecodeChunkBytes = 8192;

// Spatial audio tuning. Distances are in engine units (a de_dust2 map spans
// thousands of units, players are ~20 unit cubes).
constexpr float kReferenceDistance = 300.0f; // distance at which gain halves
constexpr float kMaxDistance = 2500.0f;      // beyond this sounds are skipped
constexpr float kMinAudibleGain = 0.02f;     // below this gain skip the play

bool isWavFile(const std::string& filepath)
{
    const auto dot = filepath.find_last_of('.');
    if (dot == std::string::npos)
        return false;
    const auto ext = filepath.substr(dot + 1);
    return ext == "wav" || ext == "WAV";
}

/// Register SDL_mixer's decoders (MP3/OGG/FLAC) exactly once.
bool ensureMixerInitialized()
{
    static const bool initialized = MIX_Init();
    if (!initialized)
        std::cerr << "spear::audio::Sound: SDL_mixer init failed: " << SDL_GetError() << std::endl;
    return initialized;
}

/// Fully decode a non-WAV clip into the playback device's own format, so the
/// pre-created voice streams can consume the bytes without any conversion.
/// Returns false on failure or unexpected end-of-file.
bool decodeNonWav(const std::string& filepath,
                  const SDL_AudioSpec& deviceSpec,
                  SDL_AudioSpec& outSpec,
                  std::vector<std::uint8_t>& outData)
{
    if (!ensureMixerInitialized())
        return false;

    MIX_AudioDecoder* decoder = MIX_CreateAudioDecoder(filepath.c_str(), 0);
    if (!decoder)
    {
        std::cerr << "spear::audio::Sound: failed to create decoder for '" << filepath
                  << "': " << SDL_GetError() << std::endl;
        return false;
    }

    outSpec = deviceSpec;
    std::vector<std::uint8_t> chunk(kDecodeChunkBytes);
    std::vector<std::uint8_t> audio;
    for (;;)
    {
        const int decoded = MIX_DecodeAudio(decoder, chunk.data(), kDecodeChunkBytes, &outSpec);
        if (decoded < 0)
        {
            std::cerr << "spear::audio::Sound: failed to decode '" << filepath
                      << "': " << SDL_GetError() << std::endl;
            MIX_DestroyAudioDecoder(decoder);
            return false;
        }
        if (decoded == 0)
            break;
        audio.insert(audio.end(), chunk.begin(), chunk.begin() + decoded);
    }
    MIX_DestroyAudioDecoder(decoder);

    outData = std::move(audio);
    return !outData.empty();
}

} // namespace

AudioSystem::~AudioSystem()
{
    if (m_deviceId != 0)
    {
        SDL_CloseAudioDevice(m_deviceId);
        m_deviceId = 0;
    }
    MIX_Quit();
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

void AudioSystem::setListener(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up)
{
    m_listenerPos = position;
    m_listenerFront = glm::normalize(front);
    m_listenerUp = glm::normalize(up);

    glm::vec3 right = glm::cross(m_listenerFront, m_listenerUp);
    if (glm::dot(right, right) < 1e-6f)
        right = glm::vec3(1.0f, 0.0f, 0.0f); // front (nearly) parallel to up
    m_listenerRight = glm::normalize(right);
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
    if (isWavFile(filepath))
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
    }
    else if (!decodeNonWav(filepath, m_system.getDeviceSpec(), m_spec, m_data))
    {
        return;
    }

    // Downmix to mono so every voice stream works the same way and spatial
    // panning always has a single material channel to route (mono -> device).
    if (m_spec.channels > 1)
    {
        SDL_AudioSpec monoSpec = m_spec;
        monoSpec.channels = 1;
        Uint8* converted = nullptr;
        int converted_len = 0;
        if (SDL_ConvertAudioSamples(&m_spec, m_data.data(), static_cast<int>(m_data.size()),
                                    &monoSpec, &converted, &converted_len))
        {
            m_data.assign(converted, converted + converted_len);
            m_spec = monoSpec;
            SDL_free(converted);
        }
        else
        {
            std::cerr << "spear::audio::Sound: failed to downmix '" << filepath
                      << "': " << SDL_GetError() << std::endl;
        }
    }

    // Pre-create and bind a small pool of voices so a shot only has to
    // queue bytes, never allocate streams on the hot path. Each voice is a
    // pair of streams (left-only and right-only output) so a sound can be
    // placed anywhere in the stereo field by scaling each gain. SDL converts
    // to the device format while the device pulls from the streams.
    for (std::size_t i = 0; i < kMaxVoices; ++i)
    {
        Voice voice;
        voice.streamL = SDL_CreateAudioStream(&m_spec, &m_system.getDeviceSpec());
        if (!voice.streamL)
        {
            std::cerr << "spear::audio::Sound: failed to create voice stream: " << SDL_GetError() << std::endl;
            break;
        }
        SDL_BindAudioStream(m_system.getDeviceId(), voice.streamL);
        SDL_SetAudioStreamGain(voice.streamL, m_volume);

        if (m_system.getDeviceChannels() > 1)
        {
            voice.streamR = SDL_CreateAudioStream(&m_spec, &m_system.getDeviceSpec());
            if (!voice.streamR)
            {
                std::cerr << "spear::audio::Sound: failed to create voice stream: " << SDL_GetError() << std::endl;
                SDL_UnbindAudioStream(voice.streamL);
                SDL_DestroyAudioStream(voice.streamL);
                break;
            }
            SDL_BindAudioStream(m_system.getDeviceId(), voice.streamR);
            SDL_SetAudioStreamGain(voice.streamR, m_volume);
        }

        m_voices.push_back(voice);
    }

    m_system.registerSound(this);
}

Sound::~Sound()
{
    m_system.unregisterSound(this);

    for (auto& voice : m_voices)
    {
        if (voice.streamL)
        {
            SDL_UnbindAudioStream(voice.streamL);
            SDL_DestroyAudioStream(voice.streamL);
        }
        if (voice.streamR)
        {
            SDL_UnbindAudioStream(voice.streamR);
            SDL_DestroyAudioStream(voice.streamR);
        }
    }
}

std::size_t Sound::bytesPerSecond() const
{
    const int sampleSize = SDL_AUDIO_BYTESIZE(m_spec.format);
    return static_cast<std::size_t>(m_spec.freq) * m_spec.channels * sampleSize;
}

void Sound::applySpatial(Voice& voice, const glm::vec3& position, float durationSeconds)
{
    const glm::vec3 dir = position - m_system.getListenerPosition();
    const float distSq = glm::dot(dir, dir);
    const float dist = std::sqrt(distSq);

    if (dist > kMaxDistance)
        return;

    // Inverse-square-ish rolloff: 1.0 at the ear, half at the reference
    // distance, then a fast decay (muffled by distance, inaudible far away).
    const float attenuation = 1.0f / (1.0f + (dist / kReferenceDistance) * (dist / kReferenceDistance));

    // Equal-power pan across the stereo field: right = dot(direction, right).
    float leftGain = 1.0f;
    float rightGain = 1.0f;
    if (distSq > 1e-6f)
    {
        const glm::vec3 dirN = dir / dist;
        const float pan = std::clamp(glm::dot(dirN, m_system.getListenerRight()), -1.0f, 1.0f);
        const float t = pan * 0.5f + 0.5f; // 0 = far left, 1 = far right
        const float kHalfPi = glm::pi<float>() * 0.5f;
        leftGain = std::cos(t * kHalfPi);
        rightGain = std::sin(t * kHalfPi);
    }

    // Store the spatial-only gain so setVolume() can rescale live voices.
    voice.lastGainL = attenuation * leftGain;
    voice.lastGainR = attenuation * rightGain;
    if (m_volume * voice.lastGainL < kMinAudibleGain &&
        m_volume * voice.lastGainR < kMinAudibleGain)
        return;

    std::size_t queuedBytes = m_data.size();
    if (durationSeconds > 0.0f)
    {
        const std::size_t bps = bytesPerSecond();
        const std::size_t frameBytes = SDL_AUDIO_BYTESIZE(m_spec.format) * m_spec.channels;
        std::size_t wanted = static_cast<std::size_t>(durationSeconds * bps);
        wanted -= wanted % frameBytes;
        queuedBytes = std::min(queuedBytes, std::max(wanted, frameBytes));
    }

    const auto queueData = [&](SDL_AudioStream* stream, float gain)
    {
        if (!stream)
            return;
        SDL_ClearAudioStream(stream);
        SDL_SetAudioStreamGain(stream, m_volume * gain);

        const int channels = m_system.getDeviceSpec().channels;
        std::vector<int> map(channels, -1);
        // Route the (mono) signal into only one output channel: 0 = left on the
        // L stream, 1 = right on the R stream. Everything else is muted.
        map[stream == voice.streamL ? 0 : (channels > 1 ? 1 : 0)] = 0;
        SDL_SetAudioStreamOutputChannelMap(stream, map.data(), channels);

        if (!SDL_PutAudioStreamData(stream, m_data.data(), static_cast<int>(queuedBytes)))
        {
            std::cerr << "spear::audio::Sound: failed to queue audio data: " << SDL_GetError() << std::endl;
            return;
        }
        SDL_FlushAudioStream(stream);
    };

    queueData(voice.streamL, voice.lastGainL);
    queueData(voice.streamR, voice.lastGainR);
    voice.playing = true;
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

    applySpatial(*voice, m_system.getListenerPosition(), 0.0f);
}

void Sound::playAt(const glm::vec3& position, float durationSeconds)
{
    if (m_system.getDeviceId() == 0 || m_data.empty() || m_voices.empty())
    {
        return;
    }

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

    applySpatial(*voice, position, durationSeconds);
}

void Sound::stop()
{
    for (auto& voice : m_voices)
    {
        if (voice.streamL && voice.playing)
        {
            // Halt the voice: anything queued but not yet mixed is dropped.
            SDL_ClearAudioStream(voice.streamL);
            voice.playing = false;
        }
        if (voice.streamR)
        {
            SDL_ClearAudioStream(voice.streamR);
        }
    }
}

bool Sound::isPlaying() const
{
    for (const auto& voice : m_voices)
    {
        if (voice.playing)
            return true;
    }
    return false;
}

void Sound::setVolume(float volume)
{
    m_volume = std::clamp(volume, 0.0f, 1.0f);
    for (auto& voice : m_voices)
    {
        if (voice.streamL)
            SDL_SetAudioStreamGain(voice.streamL, m_volume * voice.lastGainL);
        if (voice.streamR)
            SDL_SetAudioStreamGain(voice.streamR, m_volume * voice.lastGainR);
    }
}

void Sound::updateVoices()
{
    for (auto& voice : m_voices)
    {
        if (!voice.playing)
            continue;
        const bool leftDone = !voice.streamL || SDL_GetAudioStreamAvailable(voice.streamL) == 0;
        const bool rightDone = !voice.streamR || SDL_GetAudioStreamAvailable(voice.streamR) == 0;
        if (leftDone && rightDone)
        {
            voice.playing = false;
        }
    }
}

} // namespace spear::audio