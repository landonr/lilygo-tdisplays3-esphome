#pragma once

#include "esphome/core/defines.h"
#include "TFT_eSPI.h"

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/component.h"

namespace esphome {
namespace tdisplays3 {

class TDisplayS3 : public PollingComponent, public display::DisplayBuffer {
 public:
  void dump_config() override;
  void setup() override;

  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void update() override;

  void set_dimensions(uint16_t width, uint16_t height);

 private:
  TFT_eSPI *tft_{nullptr};
  TFT_eSprite *spr_{nullptr};
  uint16_t width_{0};
  uint16_t height_{0};
};

}  // namespace tdisplays3
}  // namespace esphome
