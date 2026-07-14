#include <DSP/DspProcessor.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void expectNear(float actual, float expected, const char *message)
{
    constexpr float epsilon = 0.0001f;
    if (std::fabs(actual - expected) > epsilon) {
        std::cerr << message << " expected " << expected << " but got " << actual << '\n';
        std::exit(1);
    }
}

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
    expectNear(DspProcessor::clampSample(1.5f), 1.0f, "Positive samples should clamp to +1");
    expectNear(DspProcessor::clampSample(-1.5f), -1.0f, "Negative samples should clamp to -1");
    expectNear(DspProcessor::applyGain(0.5f, 0.5f), 0.25f, "Gain should scale samples");
    expectNear(DspProcessor::panLeftGain(-1.0f), 1.0f, "Full-left pan keeps left gain");
    expectNear(DspProcessor::panRightGain(1.0f), 1.0f, "Full-right pan keeps right gain");
    expectNear(DspProcessor::dbToLinear(0.0f), 1.0f, "0 dB should be unity gain");
    expectNear(DspProcessor::applyThreeBandEq(0.5f, 0.0f, 0.0f, 0.0f), 0.5f, "Flat EQ should pass sample");

    const float boosted = DspProcessor::applyThreeBandEq(0.25f, 6.0f, 0.0f, 0.0f);
    expectTrue(boosted > 0.25f, "Positive low-band EQ should raise amplitude");

    const float compressed = DspProcessor::applyCompressor(1.0f, 0.5f, 4.0f);
    expectNear(compressed, 0.625f, "Compressor should reduce overshoot by ratio");

    TrackProcessParams audibleTrack;
    audibleTrack.volume = 1.0f;
    audibleTrack.pan = -1.0f;
    audibleTrack.audible = true;
    audibleTrack.fxBypass = true;
    const StereoSample leftOnly = DspProcessor::processTrackSample(0.8f, audibleTrack);
    expectNear(leftOnly.left, 0.8f, "Full-left pan should keep left channel");
    expectNear(leftOnly.right, 0.0f, "Full-left pan should mute right channel");

    TrackProcessParams mutedTrack = audibleTrack;
    mutedTrack.audible = false;
    const StereoSample silent = DspProcessor::processTrackSample(0.8f, mutedTrack);
    expectNear(silent.left, 0.0f, "Inaudible track should output silence left");
    expectNear(silent.right, 0.0f, "Inaudible track should output silence right");

    StereoSample mix;
    mix = DspProcessor::mixLinear(mix, leftOnly);
    mix = DspProcessor::mixLinear(mix, leftOnly);
    expectNear(mix.left, 1.6f, "Linear mix should sum channels without clamping");

    const StereoSample mastered = DspProcessor::applyMasterChain(mix, 1.0f);
    expectNear(mastered.left, 1.0f, "Master limiter should clamp summed peak to +1");
    expectNear(mastered.right, 0.0f, "Master chain should preserve silent right");

    const StereoSample attenuated = DspProcessor::applyMasterChain(leftOnly, 0.5f);
    expectNear(attenuated.left, 0.4f, "Master volume should scale before limit");

    return 0;
}
