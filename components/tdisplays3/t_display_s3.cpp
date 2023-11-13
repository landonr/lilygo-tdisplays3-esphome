#include "t_display_s3.h"

#include "esphome/components/display/display_color_utils.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

namespace writer_shim {
  // A background task used to write the framebuffer to the display
  static void write_framebuffer_task(void *pv_params) {

    auto tft = (TFT_eSPI *) pv_params;

    while (true) {

      //Wait for notifications..
      auto x1 = (int16_t) ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(200));
      auto y1 = (int16_t) ulTaskNotifyTakeIndexed(1, pdTRUE, pdMS_TO_TICKS(200));
      auto x2 = (int16_t) ulTaskNotifyTakeIndexed(2, pdTRUE, pdMS_TO_TICKS(200));
      auto y2 = (int16_t) ulTaskNotifyTakeIndexed(3, pdTRUE, pdMS_TO_TICKS(200));
      auto buffer = (uint16_t*) ulTaskNotifyTakeIndexed(4, pdTRUE, pdMS_TO_TICKS(200));
      auto callback = (void (*)(void)) ulTaskNotifyTakeIndexed(5, pdTRUE, pdMS_TO_TICKS(200));

      if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
        continue;
      }

      //And push the image
      tft->pushImage(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1, buffer);
      
      //Notify we are done!
      //callback();
    }
  }
}


void TDisplayS3::setup() {
  this->tft_ = new TFT_eSPI();
  this->tft_->init();

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

  this->tft_->fillScreen(TFT_BLACK);

  this->spr_ = new TFT_eSprite(this->tft_);

  //Create a sprite with the same dimensions as the display, with 2 frames (double buffering) if using multithreading
  this->buffer_ = (uint8_t*) this->spr_->createSprite(get_width_internal(), get_height_internal(), this->multithreading_ ? 2 : 1);

  if (this->buffer_ == nullptr) {
    this->mark_failed();
  }

  // Create a task to write the framebuffer to the display, pinned to the second core.
  if (this->multithreading_) {
    xTaskCreatePinnedToCore(writer_shim::write_framebuffer_task, "framebuffer_task", 8192, this->tft_, 1, &(this->writeFrameTask_), tskNO_AFFINITY);
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

void TDisplayS3::updateArea(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *buffer, void (*ready_callback)(void)){

  if (this->multithreading_) {
    if (xTaskNotifyIndexed(this->writeFrameTask_, 0, (uint32_t) x1, eSetValueWithoutOverwrite) == pdFALSE) return;
    if (xTaskNotifyIndexed(this->writeFrameTask_, 1, (uint32_t) y1, eSetValueWithoutOverwrite) == pdFALSE) return;
    if (xTaskNotifyIndexed(this->writeFrameTask_, 2, (uint32_t) x2, eSetValueWithoutOverwrite) == pdFALSE) return;
    if (xTaskNotifyIndexed(this->writeFrameTask_, 3, (uint32_t) y2, eSetValueWithoutOverwrite) == pdFALSE) return;
    if (xTaskNotifyIndexed(this->writeFrameTask_, 4, (uint32_t) buffer, eSetValueWithoutOverwrite) == pdFALSE) return;
    if (xTaskNotifyIndexed(this->writeFrameTask_, 5, (uint32_t) ready_callback, eSetValueWithoutOverwrite) == pdFALSE) return;
  } else {
    this->tft_->pushImage(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1, buffer);
   ready_callback();
  }
}

void TDisplayS3::update() {
  //Let lambdas draw their last stuff
  this->do_update_();

  if (!this->multithreading_) {
    this->spr_->pushSprite(0, 0);  
    return;
  } else {

    //Grab and swap screen-buffers
    auto currentBuffer = this->buffer_;
    if (((uint8_t*) this->spr_->frameBuffer(1)) != currentBuffer) {
      this->buffer_ = (uint8_t*) this->spr_->frameBuffer(1);
    } else {
      this->buffer_ = (uint8_t*) this->spr_->frameBuffer(2);
    }

    this->updateArea((int16_t) 0, (int16_t) 0, this->spr_->width(), this->spr_->height(), (uint16_t*) this->buffer_, nullptr);
  }
}

void TDisplayS3::set_dimensions(uint16_t width, uint16_t height) {
  this->width_ = width;
  this->height_ = height;
}

}  // namespace tdisplays3
}  // namespace esphome
