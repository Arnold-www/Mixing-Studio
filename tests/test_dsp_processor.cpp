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

} // namespace

int main()
{
    expectNear(DspProcessor::clampSample(1.5f), 1.0f, "Positive samples should clamp to +1");
    expectNear(DspProcessor::clampSample(-1.5f), -1.0f, "Negative samples should clamp to -1");
    expectNear(DspProcessor::applyGain(0.5f, 0.5f), 0.25f, "Gain should scale samples");
    expectNear(DspProcessor::panLeftGain(-1.0f), 1.0f, "Full-left pan keeps left gain");
    expectNear(DspProcessor::panRightGain(1.0f), 1.0f, "Full-right pan keeps right gain");
    return 0;
}
