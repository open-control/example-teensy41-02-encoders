/**
 * @file main.cpp
 * @brief Example 02: Encoders - Rotary Encoders to MIDI CC
 *
 * Building on Example 01, this adds rotary encoder support.
 * Each encoder controls a MIDI CC value.
 *
 * What you'll learn:
 * - How to configure EncoderDef for hardware description
 * - How to create and initialize EncoderController
 * - How to use callbacks to react to encoder changes
 * - EncoderMode options (NORMALIZED, RAW, RELATIVE)
 *
 * Hardware required:
 * - Teensy 4.1
 * - 2 rotary encoders (A/B quadrature)
 *   - Encoder 1: pins 22 (A), 23 (B)
 *   - Encoder 2: pins 18 (A), 19 (B)
 *
 * Wiring:
 * - Encoder common pin → GND
 * - Encoder A/B pins → Teensy GPIO (internal pull-ups used)
 */

#include <Arduino.h>
#include <oc/core/Result.hpp>
#include <oc/teensy/EncoderController.hpp>
#include <oc/teensy/EncoderToolHardware.hpp>
#include <oc/teensy/UsbMidi.hpp>
#include <oc/common/EncoderDef.hpp>

// ═══════════════════════════════════════════════════════════════════════════
// Configuration
// ═══════════════════════════════════════════════════════════════════════════

constexpr uint8_t MIDI_CHANNEL = 0;
constexpr uint8_t CC_BASE = 16;  // Encoder 1 = CC 16, Encoder 2 = CC 17

// Encoder hardware definitions
// EncoderDef(id, pinA, pinB, ppr, rangeAngle, ticksPerEvent, invertDirection)
constexpr std::array<oc::common::EncoderDef, 2> ENCODERS = {{
    oc::common::EncoderDef(1, 22, 23, 24, 270, 4, false),  // Encoder 1
    oc::common::EncoderDef(2, 18, 19, 24, 270, 4, false),  // Encoder 2
}};

// ═══════════════════════════════════════════════════════════════════════════
// Global instances
// ═══════════════════════════════════════════════════════════════════════════

oc::teensy::UsbMidi midi;
oc::teensy::EncoderToolFactory encoderFactory;
oc::teensy::EncoderController<2> encoders(ENCODERS, encoderFactory);

// ═══════════════════════════════════════════════════════════════════════════
// Arduino Entry Points
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {}

    Serial.println("\n[Example 02] Encoders");
    Serial.println("=====================\n");

    // Initialize MIDI
    if (auto r = midi.init(); !r) {
        Serial.printf("[ERROR] MIDI: %s\n", oc::core::errorCodeToString(r.error().code));
        while (true);
    }

    // Initialize encoders
    if (auto r = encoders.init(); !r) {
        Serial.printf("[ERROR] Encoders: %s\n", oc::core::errorCodeToString(r.error().code));
        while (true);
    }

    // Set callback for encoder changes
    // Value is normalized [0.0, 1.0] by default (EncoderMode::NORMALIZED)
    encoders.setCallback([](oc::hal::EncoderID id, float value) {
        uint8_t cc = CC_BASE + id - 1;  // id is 1-based
        uint8_t midiValue = static_cast<uint8_t>(value * 127.0f);

        midi.sendCC(MIDI_CHANNEL, cc, midiValue);
        Serial.printf("[Encoder %d] CC %d = %d (%.2f)\n", id, cc, midiValue, value);
    });

    Serial.println("[OK] Ready");
    Serial.printf("Encoders send CC %d-%d on channel %d\n\n",
                  CC_BASE, CC_BASE + 1, MIDI_CHANNEL + 1);
}

void loop() {
    // Poll encoders and trigger callbacks
    encoders.update();

    // Process MIDI
    midi.update();
}
