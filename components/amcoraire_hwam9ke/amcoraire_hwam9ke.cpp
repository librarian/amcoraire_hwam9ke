#include "amcoraire_hwam9ke.h"
#include "esphome/core/log.h"

namespace esphome {
namespace amcoraire_hwam9ke {

static const char *const TAG = "amcoraire_hwam9ke";

static constexpr uint16_t HDR_MARK = 7810;
static constexpr uint16_t HDR_SPACE = 2868;

static constexpr uint16_t BIT_MARK = 552;
static constexpr uint16_t ZERO_SPACE = 1447;
static constexpr uint16_t ONE_SPACE = 2868;

static constexpr uint16_t FOOTER_SPACE = 10126;
static constexpr uint16_t CARRIER_KHZ = 38;

using ProtocolMode = AmcoraireProtocolFrame::Mode;
using ProtocolFan = AmcoraireProtocolFrame::Fan;

static ProtocolMode to_protocol_mode(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_HEAT:
      return ProtocolMode::HEAT;
    case climate::CLIMATE_MODE_DRY:
      return ProtocolMode::DRY;
    case climate::CLIMATE_MODE_FAN_ONLY:
      return ProtocolMode::FAN_ONLY;
    case climate::CLIMATE_MODE_COOL:
    case climate::CLIMATE_MODE_OFF:
    default:
      return ProtocolMode::COOL;
  }
}

static ProtocolFan to_protocol_fan(climate::ClimateFanMode fan) {
  switch (fan) {
    case climate::CLIMATE_FAN_LOW:
      return ProtocolFan::SPEED_LOW;
    case climate::CLIMATE_FAN_MEDIUM:
      return ProtocolFan::SPEED_MEDIUM;
    case climate::CLIMATE_FAN_HIGH:
      return ProtocolFan::SPEED_HIGH;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return ProtocolFan::AUTO;
  }
}

void AmcoraireHWAM9KEClimate::setup() {
  this->ir_ = new IRsend(this->pin_);
  this->ir_->begin();

  this->mode = climate::CLIMATE_MODE_OFF;
  this->target_temperature = 24;
  this->fan_mode = climate::CLIMATE_FAN_AUTO;
  this->swing_mode = climate::CLIMATE_SWING_VERTICAL;

  this->publish_state();
}

void AmcoraireHWAM9KEClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "AmcorAire HW-AM9KE Climate:");
  ESP_LOGCONFIG(TAG, "  IR TX pin: GPIO%u", this->pin_);
}

climate::ClimateTraits AmcoraireHWAM9KEClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_FAN_ONLY,
  });

  traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  });

  traits.set_supported_swing_modes({
      climate::CLIMATE_SWING_OFF,
      climate::CLIMATE_SWING_VERTICAL,
  });

  traits.set_visual_min_temperature(18);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(1.0f);

  return traits;
}

void AmcoraireHWAM9KEClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    this->mode = *call.get_mode();
  }

  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    if (temp < 18) temp = 18;
    if (temp > 30) temp = 30;
    this->target_temperature = temp;
  }

  if (call.get_fan_mode().has_value()) {
    this->fan_mode = *call.get_fan_mode();
  }

  if (call.get_swing_mode().has_value()) {
    this->swing_mode = *call.get_swing_mode();
  }

  this->send_state_();
  this->publish_state();
}

AmcoraireProtocolFrame AmcoraireHWAM9KEClimate::build_frame_() const {
  AmcoraireProtocolFrame frame;

  frame.set_power(this->mode != climate::CLIMATE_MODE_OFF);
  frame.set_mode(to_protocol_mode(this->mode));
  frame.set_temperature(static_cast<uint8_t>(this->target_temperature));
  frame.set_fan(to_protocol_fan(this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)));
  frame.set_swing(this->swing_mode == climate::CLIMATE_SWING_VERTICAL);

  return frame;
}

void AmcoraireHWAM9KEClimate::send_state_() {
  if (this->ir_ == nullptr) {
    ESP_LOGW(TAG, "IR sender not initialized");
    return;
  }

  auto frame = this->build_frame_();

  uint8_t bytes[9];
  frame.encode(bytes);

  ESP_LOGD(TAG,
           "Sending frame: %02X %02X %02X %02X %02X %02X %02X %02X %02X",
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4],
           bytes[5], bytes[6], bytes[7], bytes[8]);

  this->send_frame_(bytes);
}

void AmcoraireHWAM9KEClimate::send_frame_(const uint8_t frame[9]) {
  uint16_t raw[148];
  size_t pos = 0;

  raw[pos++] = HDR_MARK;
  raw[pos++] = HDR_SPACE;

  for (int byte_index = 0; byte_index < 9; byte_index++) {
    uint8_t byte = frame[byte_index];

    for (int bit = 0; bit < 8; bit++) {
      const bool one = byte & (1 << bit);
      raw[pos++] = BIT_MARK;
      raw[pos++] = one ? ONE_SPACE : ZERO_SPACE;
    }
  }

  raw[pos++] = BIT_MARK;
  raw[pos++] = FOOTER_SPACE;

  ESP_LOGD(TAG, "Sending %u raw timings", pos);

  this->ir_->sendRaw(raw, pos, CARRIER_KHZ);
}

}  // namespace amcoraire_hwam9ke
}  // namespace esphome
