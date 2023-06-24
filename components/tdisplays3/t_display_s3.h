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
  int get_width_internal() override { return this->tft_->getViewportWidth(); }
  int get_height_internal() override { return this->tft_->getViewportHeight(); }
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void update() override;

 private:
  TFT_eSPI *tft_{nullptr};
  TFT_eSprite *spr_{nullptr};
};

}  // namespace tdisplays3
}  // namespace esphome
