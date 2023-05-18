#include "t_display_s3.h"

#include "esphome/components/display/display_color_utils.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

void TDisplayS3::setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
}

void TDisplayS3::dump_config() {
  LOG_DISPLAY("", "T-Display S3 (ST7789)", this);
  LOG_UPDATE_INTERVAL(this);
}

void TDisplayS3::loop() {}

void TDisplayS3::fill(Color color) { tft.fillScreen(display::ColorUtil::color_to_565(color)); }

int TDisplayS3::get_width_internal() { return tft.getViewportWidth(); }

int TDisplayS3::get_height_internal() { return tft.getViewportHeight(); }

display::DisplayType TDisplayS3::get_display_type() { return display::DisplayType::DISPLAY_TYPE_COLOR; }

void TDisplayS3::draw_absolute_pixel_internal(int x, int y, Color color) {
  // TODO: Performance optimization.
  // Currently every pixel is individually send to display. A full update via `it.filled_rectangle(0, 0, 320, 170,
  // Color(255, 0, 0));` takes ~180ms. Alternative is to build framebuffer and send data as block (using `setWindow`
  // and `tft_Write_16`).
  tft.drawPixel(x, y, display::ColorUtil::color_to_565(color));
}

void TDisplayS3::update() { this->do_update_(); }

}  // namespace tdisplays3
}  // namespace esphome