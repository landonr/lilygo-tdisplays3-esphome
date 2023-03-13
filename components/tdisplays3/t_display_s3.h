#ifndef TDISPLAY_S3
#define TDISPLAY_S3

#include "TFT_eSPI.h"
#include "esphome.h"

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/display/display_color_utils.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

class TDisplayS3 : public PollingComponent,
                   public display::DisplayBuffer
{
  public:
    void setup() override {
        tft.init();
        spr.createSprite(get_width_internal(), get_height_internal());
        tft.fillScreen(TFT_BLACK);
    }

    void loop() override {
    }

    //////////
    // DisplayBuffer methods
    //////////
    void fill(Color color) override {
        spr.fillScreen(display::ColorUtil::color_to_565(color));
    }

    int get_width_internal() override {
        return tft.getViewportWidth();
    }

    int get_height_internal() override {
	return tft.getViewportHeight();
    }

    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_COLOR;
    }

    void draw_absolute_pixel_internal(int x, int y, Color color) override {
        spr.drawPixel(x, y, display::ColorUtil::color_to_565(color));
    }

    /////////////
    // PollingComponent Methods
    /////////////
    void update() override {
        this->do_update_();
        spr.pushSprite(0, 0);
    }

  private:
    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite spr = TFT_eSprite(&tft);
};

}  // namespace tdisplays3
}  // namespace esphome

#endif
