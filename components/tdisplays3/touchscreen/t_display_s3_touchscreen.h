#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"

namespace esphome {
namespace tdisplays3 {

/**
 * Store interrupt related information.
 */
struct Store {
  volatile bool touch;
  ISRInternalGPIOPin pin;

  static void gpio_intr(Store *store);
};

class LilygoTDisplayS3Touchscreen : public touchscreen::Touchscreen, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_offset(int16_t x_offset, int16_t y_offset) { x_offset_ = x_offset; y_offset_ = y_offset; }

  int16_t x;
  int16_t y;

 protected:
  InternalGPIOPin *interrupt_pin_;
  int16_t x_offset_;
  int16_t y_offset_;
  uint16_t firmware_version_;
  Store store_;

};

}  // namespace lilygo_tdisplay_s3
}  // namespace esphome
