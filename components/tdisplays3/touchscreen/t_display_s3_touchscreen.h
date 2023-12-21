#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"
#include "esphome/core/version.h"

namespace esphome {
namespace tdisplays3 {

#if ESPHOME_VERSION_CODE < VERSION_CODE(2023, 12, 0)
/**
 * Store interrupt related information.
 */
struct Store {
  volatile bool touch;
  ISRInternalGPIOPin pin;

  static void gpio_intr(Store *store);
};
#endif  // VERSION_CODE(2023, 12, 0)

class LilygoTDisplayS3Touchscreen : public touchscreen::Touchscreen,
#if ESPHOME_VERSION_CODE < VERSION_CODE(2023, 12, 0)
                                    public Component,
#endif  // VERSION_CODE(2023, 12, 0)
                                    public i2c::I2CDevice {
 public:
  void setup() override;
#if ESPHOME_VERSION_CODE < VERSION_CODE(2023, 12, 0)
  void loop() override;
#endif  // VERSION_CODE(2023, 12, 0)
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_offset(int16_t x_offset, int16_t y_offset) {
    x_offset_ = x_offset;
    y_offset_ = y_offset;
  }

  int16_t x;
  int16_t y;

 protected:
  InternalGPIOPin *interrupt_pin_;
  int16_t x_offset_;
  int16_t y_offset_;
  uint16_t firmware_version_;
#if ESPHOME_VERSION_CODE < VERSION_CODE(2023, 12, 0)
  Store store_;
#else
  void update_touches() override;
#endif  // VERSION_CODE(2023, 12, 0)
};

}  // namespace tdisplays3
}  // namespace esphome
