//
// Created by Syl Morrison on 04/07/2025.
//
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#if defined SIGNALSMITH_USE_ACCELERATE
#undef SIGNALSMITH_USE_ACCELERATE
#endif
#include <signalsmith-stretch/signalsmith-stretch.h>

template <int N>
auto generate_impulse() -> std::vector<float> {
    std::vector<float> impulse(N, 0.0f);
    impulse[0] = 1.0f;
    return impulse;
}

template <int InSamples, int OutSamples>
auto test_stretch(signalsmith::stretch::SignalsmithStretch<float>& stretch) -> void {
    std::cout << "========================\n IN: " << InSamples << "\n OUT: " << OutSamples << "\n....\n";
    std::vector<std::vector<float>> input;
    input.push_back(generate_impulse<InSamples>());
    input.push_back(generate_impulse<InSamples>());
    std::vector<std::vector<float>> outputBuffer;
    outputBuffer.emplace_back();
    outputBuffer.back().resize(OutSamples);
    outputBuffer.emplace_back();
    outputBuffer.back().resize(OutSamples);

    stretch.presetDefault(2, 44100.0f);
    stretch.reset();
    const auto inputLatency = stretch.inputLatency();
    const auto outputLatency = stretch.outputLatency();
    stretch.seek(input, inputLatency, 1.0);
    std::vector<float*> flushedOutput;
    for (auto& channel : outputBuffer) {
        channel.resize(OutSamples + outputLatency);
        flushedOutput.push_back(channel.data() + OutSamples);
    }
    stretch.process(input, InSamples, outputBuffer, OutSamples);
    stretch.flush(flushedOutput, outputLatency);
    for (auto& x : outputBuffer) {
        for (auto& s : x) {
            REQUIRE(!std::isnan(s));
        }
    }
    stretch.reset();
    std::cout << "PASSED!\n========================\n";
}
TEST_CASE("Test Stretch with small buffer") {
    signalsmith::stretch::SignalsmithStretch<float> stretch;
    test_stretch<26, 26>(stretch);
    test_stretch<30, 30>(stretch);
    test_stretch<256, 256>(stretch);
    test_stretch<200, 200>(stretch);
    test_stretch<100, 100>(stretch);
}
