#include <Model/AudioTrack.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void expectTrue(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main()
{
    AudioTrack track;
    expectTrue(std::abs(track.automationVolumeAtMs(100) - 1.0f) < 0.0001f,
               "Empty automation should return unity");

    track.volumeAutomation.append({500, 0.5f});
    expectTrue(std::abs(track.automationVolumeAtMs(0) - 0.5f) < 0.0001f,
               "Single point should clamp everywhere");
    expectTrue(std::abs(track.automationVolumeAtMs(900) - 0.5f) < 0.0001f,
               "Single point after time should clamp");

    track.volumeAutomation.clear();
    track.volumeAutomation.append({0, 0.0f});
    track.volumeAutomation.append({1000, 1.0f});
    expectTrue(std::abs(track.automationVolumeAtMs(-10) - 0.0f) < 0.0001f,
               "Before first point should clamp to first");
    expectTrue(std::abs(track.automationVolumeAtMs(2000) - 1.0f) < 0.0001f,
               "After last point should clamp to last");
    expectTrue(std::abs(track.automationVolumeAtMs(500) - 0.5f) < 0.0001f,
               "Midpoint should linearly interpolate");

    // Sample-accurate reads must advance within a single millisecond (not hold).
    AudioTrack pcm;
    pcm.sampleRate = 44100;
    pcm.pcmMono.resize(44100);
    for (int i = 0; i < pcm.pcmMono.size(); ++i) {
        pcm.pcmMono[i] = static_cast<float>(i) / 44100.0f;
    }
    const float a = pcm.sampleAtSeconds(0.0);
    const float b = pcm.sampleAtSeconds(1.0 / 44100.0);
    const float c = pcm.sampleAtSeconds(2.0 / 44100.0);
    expectTrue(std::abs(b - a) > 1.0e-6f, "Adjacent output frames must read distinct source samples");
    expectTrue(std::abs(c - b) > 1.0e-6f, "Sample-accurate playhead must not hold for a full millisecond");
    expectTrue(std::abs(pcm.sampleAtMs(0) - a) < 1.0e-6f, "sampleAtMs(0) should match sampleAtSeconds(0)");

    // Per-track content loop: past the end wraps instead of returning silence.
    AudioTrack shortClip;
    shortClip.sampleRate = 1000;
    shortClip.pcmMono = {0.0f, 0.25f, 0.5f, 0.75f}; // 4 ms at 1x
    expectTrue(std::abs(shortClip.sampleAtSeconds(0.005, false)) < 1.0e-6f,
               "Without loop, past end should be silence");
    expectTrue(std::abs(shortClip.sampleAtSeconds(0.004, true) - 0.0f) < 1.0e-6f,
               "With loop, t=contentLength wraps to start");
    expectTrue(std::abs(shortClip.sampleAtSeconds(0.005, true) - 0.25f) < 1.0e-6f,
               "With loop, short clip restarts independently of session length");
    shortClip.playbackRate = 2.0f;
    expectTrue(std::abs(shortClip.contentDurationSeconds() - 0.002) < 1.0e-9,
               "2x rate halves content cycle length");
    expectTrue(std::abs(shortClip.sampleAtSeconds(0.002, true) - 0.0f) < 1.0e-6f,
               "Faster rate wraps earlier on the same track");

    return 0;
}
