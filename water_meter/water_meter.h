#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace water_meter {

class WaterMeterComponent : public Component {
  public:

    // Wait for everything to be ready before starting the scan
    float get_setup_priority() const override { return esphome::setup_priority::LATE; }

    void set_flow_sensor(sensor::Sensor *sensor) { flow_sensor_ = sensor; }
    void set_volume_sensor(sensor::Sensor *sensor) { volume_sensor_ = sensor; }
    void set_duration_sensor(sensor::Sensor *sensor) { duration_sensor_ = sensor; }
    void set_sensor_pin(int pin) { sensor_pin_ = pin; }

    void setup() override;
    void loop() override;

  protected:
    sensor::Sensor *flow_sensor_{nullptr};
    sensor::Sensor *volume_sensor_{nullptr};
    sensor::Sensor *duration_sensor_{nullptr};

    static constexpr uint32_t PULSE_FACTOR = 120000;
    static constexpr uint32_t MAX_FLOW = 40;  // Max flow (l/min) value to report. This filters outliers.
    static constexpr uint32_t SEND_FREQUENCY = 30000;  // Minimum time between send (in milliseconds).
    static constexpr uint32_t READ_FREQUENCY = 10;     // ms between sensor reads
    
    const double ppl_ = ((double)PULSE_FACTOR) / 1000.0;  // Pulses per liter

    volatile uint32_t pulse_count_ = 0;
    uint32_t old_pulse_count_ = 0;
    volatile uint32_t last_blink_ = 0;
    volatile double flow_ = 0;
    uint32_t new_blink_ = 0;
    double old_flow_ = 0;
    double volume_ = 0;
    double old_volume_ = 0;
    uint32_t last_send_ = 0;
    uint32_t last_pulse_ = 0;
    uint32_t last_read_ = 0;
    double cur_volume_ = 0;
    double old_cur_volume_ = 0;
    uint32_t start_pulse_ = 0;
    uint32_t duration_ = 0;
    uint32_t flow_start_ = 0;
    uint32_t read_delay_ = 5000;  // delay before first read to get network ready

    // PINS
    const int led_pin_ = 2;  // onboard LED (GPIO2 is common for ESP8266/ESP32)
    int sensor_pin_ = 15;  // Default to GPIO 15, configurable from YAML

    // Calculation
    float distance_ = 0;
    float old_distance_ = 0;
    float sens_ = 0;
    float alpha_ = 0.02;               // smoothing the measurements, smaller is smoother
    bool trend_ = true;                // decreasing = false, increasing = true
    float slope_ = 0;                  // slope value
    float threshold_ = 0.5;            // threshold of two consecutive measurements
    float ascend_limit_ = threshold_;  // ascend limit of oldest and newest measurement
    float descend_limit_ = -threshold_; // descend limit of oldest and newest measurement

    void on_pulse_();
};

}  // namespace water_meter
}  // namespace esphome
