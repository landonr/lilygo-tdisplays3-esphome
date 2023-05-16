#include "t_display_s3.h"

#include "esphome/components/display/display_color_utils.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

void TDisplayS3::setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
}

void TDisplayS3::loop() {}

void TDisplayS3::fill(Color color) { tft.fillScreen(display::ColorUtil::color_to_565(color)); }

int TDisplayS3::get_width_internal() { return tft.getViewportWidth(); }

int TDisplayS3::get_height_internal() { return tft.getViewportHeight(); }

display::DisplayType TDisplayS3::get_display_type() { return display::DisplayType::DISPLAY_TYPE_COLOR; }

void TDisplayS3::draw_absolute_pixel_internal(int x, int y, Color color) {
  tft.drawPixel(x, y, display::ColorUtil::color_to_565(color));
}

void TDisplayS3::update() { this->do_update_(); }

}  // namespace tdisplays3
}  // namespace esphome