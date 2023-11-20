#include "t_display_s3.h"

#include "esphome/components/display/display_color_utils.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tdisplays3 {

static const char *const TAG = "TDisplayS3";

#ifdef TDISPLAYS3_USE_ASYNC_IO
namespace writer_shim {
  static TaskHandle_t writeFrameTask_ = nullptr;
  static int16_t x1 = 0;
  static int16_t x2 = 0;
  static int16_t y1 = 0;
  static int16_t y2 = 0;
  static uint16_t *buffer = nullptr;
  static void (*callback)(void) = nullptr;

  // A background task used to write the framebuffer to the display
  static void IRAM_ATTR write_framebuffer_task(void *pv_params) {

    auto tft = (TFT_eSPI *) pv_params;

    while (true) {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
        continue;
      }

      //And push the image
      tft->pushImage(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1, buffer);
      
      //Notify we are done!
      callback();
    }
  }

  static void init(TFT_eSPI *tft) {
    //The stacksize needed for this driver is about 724 bytes, but we need to be safe, so take some extra just to be sure..
    //The task is also pinned to CPU core 0, with low priority (just above idle), to decouple writing and rendering.    
    xTaskCreatePinnedToCore(writer_shim::write_framebuffer_task, "tft_drv", 1024, tft, 1, &writeFrameTask_, 0); //tskNO_AFFINITY
  }

  static void startWrite(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *buffer, void (*ready_callback)(void)) {
    writer_shim::x1 = x1;
    writer_shim::x2 = x2;
    writer_shim::y1 = y1;
    writer_shim::y2 = y2;
    writer_shim::buffer = buffer;
    writer_shim::callback = ready_callback;
    xTaskNotifyGive(writer_shim::writeFrameTask_);
  }
}

#endif

void TDisplayS3::setup() {
  this->tft_ = new TFT_eSPI();
  this->tft_->init();

  this->power_on();
  
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

  if (!this->disable_buffer_) {
    //Create a sprite with the same dimensions as the display, with 2 frames (double buffering) if using multithreading
    this->spr_ = new TFT_eSprite(this->tft_);

#ifdef TDISPLAYS3_USE_ASYNC_IO
    auto buffers = 2;
#else
    auto buffers = 1;
#endif
    this->buffer_ = (uint8_t*) this->spr_->createSprite(get_width_internal(), get_height_internal(), buffers);
    if (this->buffer_ == nullptr) {
      ESP_LOGE(TAG, "Not enough memory to create a screenbuffer!");
      this->mark_failed();
      return;
    }
  }
  
#ifdef TDISPLAYS3_USE_ASYNC_IO
  // Create a task to write the framebuffer to the display, pinned to the second core.
  writer_shim::init(this->tft_);
#endif
}

void TDisplayS3::dump_config() {
  LOG_DISPLAY("", "T-Display S3 (ST7789)", this);
#ifdef TDISPLAYS3_USE_ASYNC_IO
  ESP_LOGCONFIG(TAG, "Using Async IO");
#else
  ESP_LOGCONFIG(TAG, "NOT Using Async IO");
#endif
  LOG_UPDATE_INTERVAL(this);

}

void TDisplayS3::fill(Color color) { 
  if (!powered_on_) return;

  if (this->disable_buffer_) {
    this->tft_->fillScreen(display::ColorUtil::color_to_565(color));
  } else {
    this->spr_->fillScreen(display::ColorUtil::color_to_565(color)); 
  }
}

void TDisplayS3::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (!powered_on_) return;

  if (this->disable_buffer_) {
    this->tft_->drawPixel(x, y, display::ColorUtil::color_to_565(color));
  } else {
    this->spr_->drawPixel(x, y, display::ColorUtil::color_to_565(color));
  }
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

void TDisplayS3::updateArea(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *buffer, void (*ready_callback)(void)) {
  if (!powered_on_) return;

#ifdef TDISPLAYS3_USE_ASYNC_IO
    if (writer_shim::writeFrameTask_ == nullptr) {
      return;
    }    
    writer_shim::startWrite(x1, y1, x2, y2, buffer, ready_callback);
#else
    this->tft_->pushImage(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1, buffer);
   ready_callback();
#endif

}

void TDisplayS3::update() {
  if (!powered_on_) return;

  //Let lambdas draw their last stuff
  this->do_update_();

#ifdef TDISPLAYS3_USE_ASYNC_IO

    if (this->disable_buffer_) {
      return;
    }

    //Grab and swap screen-buffers
    auto currentBuffer = this->buffer_;
    if (((uint8_t*) this->spr_->frameBuffer(1)) != currentBuffer) {
      this->buffer_ = (uint8_t*) this->spr_->frameBuffer(1);
    } else {
      this->buffer_ = (uint8_t*) this->spr_->frameBuffer(2);
    }

    this->updateArea((int16_t) 0, (int16_t) 0, this->spr_->width(), this->spr_->height(), (uint16_t*) this->buffer_, nullptr);
#else
    this->spr_->pushSprite(0, 0);  
#endif

}

void TDisplayS3::set_dimensions(uint16_t width, uint16_t height) {
  this->width_ = width;
  this->height_ = height;
}

void TDisplayS3::power_off() {
  this->tft_->writecommand(ST7789_DISPOFF); //turn off lcd display
  this->powered_on_ = false;
}

void TDisplayS3::power_on() {
  this->tft_->writecommand(ST7789_DISPON); //turn on lcd display
  this->powered_on_ = true;
}


}  // namespace tdisplays3
}  // namespace esphome
