#include "t_display_s3.h"

#include "esphome/components/display/display_color_utils.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

void TDisplayS3::setup() {
  this->tft_ = new TFT_eSPI();
  this->tft_->init();
  //this->tft_->initDMA(false);

  //Set the roptation on the TFT driver, instead of the intertal display modules.
  switch (this->get_rotation())
  {
    case esphome::display::DisplayRotation::DISPLAY_ROTATION_90_DEGREES:
      this->tft_->setRotation(1);
      break;
    case esphome::display::DisplayRotation::DISPLAY_ROTATION_180_DEGREES:
      this->tft_->setRotation(2);
      break;
    case esphome::display::DisplayRotation::DISPLAY_ROTATION_270_DEGREES:
      this->tft_->setRotation(3);
      break;    
    default:
      break;
    }
  this->set_rotation(esphome::display::DisplayRotation::DISPLAY_ROTATION_0_DEGREES);


  this->tft_->fillScreen(TFT_WHITE);

  this->spr_ = new TFT_eSprite(this->tft_);
  this->buffer_ = (uint8_t*) this->spr_->createSprite(get_width_internal(), get_height_internal());
  if (this->buffer_ == nullptr) {
    this->mark_failed();
  }
}

void TDisplayS3::dump_config() {
  LOG_DISPLAY("", "T-Display S3 (ST7789)", this);
  LOG_UPDATE_INTERVAL(this);
}

void TDisplayS3::fill(Color color) { this->spr_->fillScreen(display::ColorUtil::color_to_565(color)); }

void TDisplayS3::draw_absolute_pixel_internal(int x, int y, Color color) {
  this->spr_->drawPixel(x, y, display::ColorUtil::color_to_565(color));
}

int TDisplayS3::get_width_internal() {
  if (this->tft_) {
    return this->tft_->getViewportWidth();
  } else {
    return this->width_;
  }
}

int TDisplayS3::get_height_internal() {
  if (this->tft_) {
    return this->tft_->getViewportHeight();
  } else {
    return this->height_;
  }
}

void TDisplayS3::update() {
  this->do_update_();
  this->spr_->pushSprite(0, 0);
  
  // this->tft_->startWrite();
  // this->tft_->pushImageDMA(0, 0, get_width_internal(), get_height_internal(), this->spr_buffer);
  // this->tft_->dmaWait();
  // this->tft_->endWrite();
}

void TDisplayS3::set_dimensions(uint16_t width, uint16_t height) {
  this->width_ = width;
  this->height_ = height;
}

}  // namespace tdisplays3
}  // namespace esphome
