//
// Created by Syl Morrison on 04/07/2025.
//
#include <catch2/catch_test_macros.hpp>
#include <iostream>
// #if defined SIGNALSMITH_USE_ACCELERATE
// #undef SIGNALSMITH_USE_ACCELERATE
// #endif
#include <signalsmith-stretch/signalsmith-stretch.h>

template<int N>
auto generate_impulse() -> std::vector<float> {
    std::vector<float> impulse(N, 0.0f);
    impulse[0] = 1.0f;
    return impulse;
}

auto nan_check(std::vector<std::vector<float> > &buffer) -> bool {
    for (auto &channel: buffer) {
        for (const auto &sample: channel) {
            if (std::isnan(sample)) {
                return false;
            }
        }
    }
    return true;
}

auto do_stretch(std::vector<std::vector<float> > &inBuffer, std::vector<std::vector<float> > &outBuffer,
                signalsmith::stretch::SignalsmithStretch<float> &engine) -> void {
    const auto inSamples = inBuffer.front().size();
    const auto outSamples = outBuffer.front().size();
    const auto inBufferLatency = engine.inputLatency();
    const auto outputLatency = engine.outputLatency();
    engine.seek(inBuffer, inBufferLatency, 1.0);
    std::vector<float *> flushedOutput;
    for (auto &channel: outBuffer) {
        channel.resize(outSamples + outputLatency);
        flushedOutput.push_back(channel.data() + outSamples);
    }
    engine.process(inBuffer, static_cast<int>(inSamples), outBuffer, static_cast<int>(outSamples));
    engine.flush(flushedOutput, outputLatency);
}

template<int InSamples, int OutSamples>
auto test_stretch(signalsmith::stretch::SignalsmithStretch<float> &stretch) -> void {
    std::cout << "========================\n IN: " << InSamples << "\n OUT: " << OutSamples << "\n....\n";
    stretch.presetDefault(2, 44100.0f);
    std::vector<std::vector<float> > input;
    input.push_back(generate_impulse<InSamples>());
    input.push_back(generate_impulse<InSamples>());
    std::vector<std::vector<float> > outputBuffer;
    outputBuffer.emplace_back();
    outputBuffer.back().resize(OutSamples);
    outputBuffer.emplace_back();
    outputBuffer.back().resize(OutSamples);
    do_stretch(input, outputBuffer, stretch);
    REQUIRE(nan_check(outputBuffer));
    stretch.reset();
    std::cout << "PASSED!\n========================\n";
}

template<int InSamples, int OutSamples>
auto test_stretch_padded(signalsmith::stretch::SignalsmithStretch<float> &stretch) -> void {
    const auto zero_pad = [](std::vector<std::vector<float> > &toPad, int minSize) -> void {
        const auto currentSize = toPad.front().size();
        if (currentSize >= minSize) return;
        for (auto &channel: toPad) {
            const auto newSize = minSize;
            channel.resize(newSize, 0.0f);
        }
    };
    stretch.presetDefault(2, 44100.0f);
    const auto inputLatency = stretch.inputLatency();
    std::vector<std::vector<float> > input, output;
    input.emplace_back(generate_impulse<InSamples>());
    input.emplace_back(generate_impulse<InSamples>());
    zero_pad(input, inputLatency);
    output.resize(2);
    for (auto &channel: output) {
        channel.resize(OutSamples, 0.0f);
    }
    do_stretch(input, output, stretch);
    REQUIRE(nan_check(output));
    stretch.reset();
}

template<int N, bool Pad>
auto test_equal_sizes(signalsmith::stretch::SignalsmithStretch<float> &engine) -> void {
    if constexpr (N != 0) {
        if constexpr (Pad) {
            test_stretch_padded<N, N>(engine);
        } else {
            test_stretch<N, N>(engine);
        }
        test_equal_sizes<N - 1, Pad>(engine);
    }
}

template<int N, bool Pad>
auto test_different_sizes(signalsmith::stretch::SignalsmithStretch<float> &engine) -> void {
    if constexpr (N != 0) {
        if constexpr (Pad) {
            test_stretch_padded<N, N * 2>(engine);
        } else {
            test_stretch<N, N * 2>(engine);
        }
        test_different_sizes<N - 1, Pad>(engine);
    }
}

TEST_CASE("Test small buffers with padding") {
    signalsmith::stretch::SignalsmithStretch<float> stretch;
    test_equal_sizes<1023, true>(stretch);
    test_different_sizes<1023, true>(stretch);
}

TEST_CASE("Test small buffers without padding") {
    signalsmith::stretch::SignalsmithStretch<float> stretch;
    test_equal_sizes<1023, false>(stretch);
    test_different_sizes<1023, false>(stretch);
}
