/**
 * @file main.cpp
 * @brief Example 02: Encoders - Rotary Encoders to MIDI CC
 *
 * Building on Example 01, this adds rotary encoder support.
 * Each encoder controls a MIDI CC value.
 *
 * What you'll learn:
 * - EncoderDef: hardware pin configuration
 * - AppBuilder: fluent API to configure the app
 * - IContext: application mode with input bindings
 * - onEncoder().turn().then(): react to encoder changes
 *
 * Hardware required:
 * - Teensy 4.1
 * - 2 rotary encoders (A/B quadrature)
 *
 * Wiring:
 * - Encoder common pin → GND
 * - Encoder A/B pins → Teensy GPIO (internal pull-ups used)
 *
 * NOTE: Enable -D OC_LOG in platformio.ini build_flags to see debug output.
 *       Remove it for production (zero overhead, instant boot).
 */

#include <optional>

#include <oc/hal/teensy/Teensy.hpp>
#include <oc/app/OpenControlApp.hpp>
#include <oc/context/ContextBase.hpp>
#include <oc/context/Requirements.hpp>
#include <oc/hal/common/embedded/EncoderDef.hpp>

// ═══════════════════════════════════════════════════════════════════════════
// Configuration - Adapt to your hardware
// ═══════════════════════════════════════════════════════════════════════════

namespace Config {
    constexpr uint8_t MIDI_CHANNEL = 0;
    constexpr uint8_t CC_BASE = 16;  // Encoder 1 = CC 16, Encoder 2 = CC 17

    // Encoder hardware definitions - ADAPT pins to your wiring
    // EncoderDef(id, pinA, pinB, ppr, rangeAngle, ticksPerEvent, invertDirection)
    constexpr std::array<oc::hal::common::embedded::EncoderDef, 2> ENCODERS = {{
        oc::hal::common::embedded::EncoderDef(1, 22, 23, 24, 270, 4, true),  // ADAPT: pins 22, 23
        oc::hal::common::embedded::EncoderDef(2, 18, 19, 24, 270, 4, true),  // ADAPT: pins 18, 19
    }};
}

// ═══════════════════════════════════════════════════════════════════════════
// Context
// ═══════════════════════════════════════════════════════════════════════════

enum class ContextID : uint8_t { MAIN = 0 };

class MainContext : public oc::context::ContextBase {
public:
    static constexpr oc::context::Requirements REQUIRES{
        .button = false,
        .encoder = true,
        .midi = true
    };

    oc::Result<void> init() override {
        // Bind each encoder to a MIDI CC
        for (uint8_t i = 0; i < Config::ENCODERS.size(); ++i) {
            uint8_t id = Config::ENCODERS[i].id;
            uint8_t cc = Config::CC_BASE + i;

            onEncoder(id).turn().then([this, cc](float value) {
                uint8_t midiValue = static_cast<uint8_t>(value * 127.0f);
                midi().sendCC(Config::MIDI_CHANNEL, cc, midiValue);
                OC_LOG_DEBUG("Encoder: CC {} = {}", cc, midiValue);
            });
        }
        return oc::Result<void>::ok();
    }

    void update() override {}
    void cleanup() override {}
    const char* getName() const override { return "Encoders"; }
};

// ═══════════════════════════════════════════════════════════════════════════
// Application
// ═══════════════════════════════════════════════════════════════════════════

std::optional<oc::app::OpenControlApp> app;

void setup() {
    OC_LOG_INFO("Example 02: Encoders");

    app = oc::hal::teensy::AppBuilder()
        .midi()
        .encoders(Config::ENCODERS);

    app->registerContext<MainContext>(ContextID::MAIN, "Main");
    app->begin();

    OC_LOG_INFO("Ready - CC {}-{}", Config::CC_BASE, Config::CC_BASE + 1);
}

void loop() {
    app->update();
}
