#pragma once

#include "esphome/core/defines.h"
#include "TFT_eSPI.h"

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace tdisplays3 {

class TDisplayS3 : public PollingComponent, public display::DisplayBuffer {
 public:
  void setup() override;
  void loop() override;

  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override;
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void update() override;

 private:
  TFT_eSPI tft = TFT_eSPI();
};

}  // namespace tdisplays3
}  // namespace esphome