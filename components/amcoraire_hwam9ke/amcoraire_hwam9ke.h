#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>

namespace esphome {
namespace amcoraire_hwam9ke {

class AmcoraireProtocolFrame {
 public:
  enum class Mode : uint8_t {
    COOL = 0x01,
    DRY = 0x02,
    FAN_ONLY = 0x03,
    HEAT = 0x04,
  };

  enum class Fan : uint8_t {
    AUTO = 0x00,
    SPEED_LOW = 0x40,
    SPEED_MEDIUM = 0x80,
    SPEED_HIGH = 0xC0,
  };

  void set_power(bool on) { this->power_on_ = on; }
  void set_mode(Mode mode) { this->mode_ = mode; }
  void set_fan(Fan fan) { this->fan_ = fan; }
  void set_swing(bool on) { this->swing_on_ = on; }

  void set_temperature(uint8_t celsius) {
    if (celsius < 18) celsius = 18;
    if (celsius > 30) celsius = 30;
    this->temperature_c_ = celsius;
  }

  void encode(uint8_t out[9]) const {
    out[0] = 0xBC;
    out[1] = this->encode_power_mode_();
    out[2] = this->encode_fan_swing_();
    out[3] = this->temperature_c_ - 16;
    out[4] = 0x00;
    out[5] = 0x00;
    out[6] = 0x00;
    out[7] = 0x00;
    out[8] = this->checksum_(out);
  }

 private:
  bool power_on_{true};
  bool swing_on_{true};
  uint8_t temperature_c_{24};
  Mode mode_{Mode::COOL};
  Fan fan_{Fan::AUTO};

  uint8_t encode_power_mode_() const {
    uint8_t value = static_cast<uint8_t>(this->mode_);
    if (this->power_on_) {
      value |= 0x80;
    }
    return value;
  }

  uint8_t encode_fan_swing_() const {
    uint8_t value = static_cast<uint8_t>(this->fan_);
    if (this->swing_on_) {
      value |= 0x3F;
    }
    return value;
  }

  static uint8_t checksum_(const uint8_t frame[9]) {
    uint8_t sum = 0;
    for (int i = 0; i < 8; i++) {
      sum += frame[i];
    }
    return sum;
  }
};

class AmcoraireHWAM9KEClimate : public climate::Climate, public Component {
 public:
  void set_pin(uint16_t pin) { this->pin_ = pin; }

  void setup() override;
  void dump_config() override;

  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

 protected:
  uint16_t pin_{4};
  IRsend *ir_{nullptr};

  AmcoraireProtocolFrame build_frame_() const;
  void send_state_();
  void send_frame_(const uint8_t frame[9]);
};

}  // namespace amcoraire_hwam9ke
}  // namespace esphome
