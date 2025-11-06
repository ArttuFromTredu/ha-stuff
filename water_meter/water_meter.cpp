#include "water_meter.h"
#include "esphome/core/log.h"

namespace esphome {
namespace water_meter {

static const char *const TAG = "water_meter";

void WaterMeterComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Water Meter...");
  pulse_count_ = 0;
  last_send_ = millis();
  pinMode(led_pin_, OUTPUT);  // initialize onboard LED as output
}

void WaterMeterComponent::loop() {
  uint32_t current_time = millis();

  if ((current_time - last_read_) > READ_FREQUENCY && current_time > read_delay_) {
    sens_ = analogRead(sensor_pin_);
    if (sens_ < 1024 && sens_ > 0) {  // Eliminate error values beyond sensor range
      last_read_ = current_time;
      read_delay_ = 0;
      distance_ = (1 - alpha_) * distance_ + alpha_ * sens_;

      slope_ = distance_ - old_distance_;
      old_distance_ = distance_;

      // start of a slope
      if (slope_ > ascend_limit_ && trend_ == false) {
        trend_ = true;
        analogWrite(led_pin_, 0);
        delay(10);
        analogWrite(led_pin_, 1023);
        on_pulse_();
      }

      // end of a slope, peak reached
      if (slope_ < descend_limit_ && trend_ == true) {
        trend_ = false;
        analogWrite(led_pin_, 0);
        delay(10);
        analogWrite(led_pin_, 1023);
        on_pulse_();
      }
    }
  }

  // Only send values at a maximum frequency or woken up from sleep
  if (current_time - last_send_ > SEND_FREQUENCY) {
    last_send_ = current_time;

    // No Pulse count increase in 30 sec
    if (current_time - last_pulse_ > 30000) {
      ESP_LOGD(TAG, "Flow stopped");
      flow_ = 0;
      start_pulse_ = pulse_count_;
      flow_start_ = last_pulse_;
    }

    if (flow_ != old_flow_) {
      old_flow_ = flow_;

      ESP_LOGD(TAG, "Flow rate: %.2f l/min", flow_);

      // Check that we don't get unreasonable large flow value.
      if (flow_ < ((uint32_t)MAX_FLOW)) {
        if (flow_sensor_ != nullptr)
          flow_sensor_->publish_state(flow_);
      }
    }

    // Pulse count has changed
    if (pulse_count_ != old_pulse_count_) {
      old_pulse_count_ = pulse_count_;

      ESP_LOGD(TAG, "Pulse count: %u", pulse_count_);

      double volume = ((double)pulse_count_ / ((double)PULSE_FACTOR));
      if (volume != old_volume_) {
        old_volume_ = volume;
        ESP_LOGD(TAG, "Total volume: %.4f L", volume);
        if (volume_sensor_ != nullptr)
          volume_sensor_->publish_state(volume * 1000);
      }
    }

    cur_volume_ = (((double)pulse_count_ - (double)start_pulse_) / ((double)PULSE_FACTOR));
    duration_ = (last_pulse_ - flow_start_) / 1000;
    if (cur_volume_ != old_cur_volume_) {
      old_cur_volume_ = cur_volume_;
      ESP_LOGD(TAG, "Current flow volume: %.4f L in %u seconds", cur_volume_, duration_);
      if (duration_sensor_ != nullptr)
        duration_sensor_->publish_state(duration_);
    }
  }
}

void WaterMeterComponent::on_pulse_() {
  uint32_t new_blink = micros();
  uint32_t interval = new_blink - last_blink_;

  if (interval != 0) {
    if (flow_ == 0) {
      ESP_LOGD(TAG, "Flow started");
      start_pulse_ = pulse_count_;
      flow_start_ = millis();
    }
    last_pulse_ = millis();
    flow_ = (60000000.0 / interval) / ppl_;
  }
  last_blink_ = new_blink;
  pulse_count_++;
}

}  // namespace water_meter
}  // namespace esphome
