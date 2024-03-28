#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/version.h"
#include "TFT_eSPI.h"

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/component.h"

namespace esphome {
namespace tdisplays3 {

#if ESPHOME_VERSION_CODE >= VERSION_CODE(2023, 12, 0)
class TDisplayS3 : public display::DisplayBuffer {
#else
class TDisplayS3 : public PollingComponent, public display::DisplayBuffer {
#endif  // VERSION_CODE(2023, 12, 0)
 public:
  void dump_config() override;
  void setup() override;

  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  float get_setup_priority() const override { return esphome::setup_priority::HARDWARE; };

  void update() override;
  void updateArea(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *buffer, void (*ready_callback)(void)) override;  
  void set_dimensions(uint16_t width, uint16_t height);
  
  void set_disable_buffer(bool value) { this->disable_buffer_ = value; }

  void power_off();
  void power_on();

 private:
  TFT_eSPI *tft_{nullptr};
  TFT_eSprite *spr_{nullptr};
  uint16_t width_{0};
  uint16_t height_{0};
  bool disable_buffer_{false};
  bool powered_on_{false};
};

}  // namespace tdisplays3
}  // namespace esphome
