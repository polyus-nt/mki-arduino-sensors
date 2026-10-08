#pragma once
// Edit this tab in Arduino IDE after agreeing the real wiring.
// Current Austere defaults: MISO=2, MOSI=3, CLK=4, CS=5, IRQ=6.
// To relocate the link, define ALL five API_LINK_* constants together.
// BMP280: one shared SCL, three distinct SDA, each device address 0x76.
// A locally timeout-patched SoftwareWire 1.6.0 is bundled in src/softwarewire.
// The supplied example (SCL=2, SDA=3/4/5) CONFLICTS with current Austere pins.
// Do not uncomment it until one of the two pin groups has been relocated.
// #define API_BMP_SCL_PIN 2
// #define API_BMP_SDA0_PIN 3
// #define API_BMP_SDA1_PIN 4
// #define API_BMP_SDA2_PIN 5
// Optional door inputs: unique pins, no overlap with Austere or BMP.
// #define API_DOOR0_PIN ...
// #define API_DOOR1_PIN ...
// #define API_DOOR0_CLOSED_LEVEL LOW
// #define API_DOOR1_CLOSED_LEVEL LOW
