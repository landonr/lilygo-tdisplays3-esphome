#include "t_display_s3_touchscreen.h"

#include "esphome/core/log.h"

namespace esphome {
namespace tdisplays3 {

using namespace touchscreen;

static const char *const TAG = "lilygo_tdisplay_s3.touchscreen";

static const uint8_t POWER_REGISTER = 0xD6;
static const uint8_t WORK_MODE_REGISTER = 0x00;

static const uint8_t TOUCH_NUM_INDEX = 0x02;
static const uint8_t TOUCH1_XH_INDEX = 0x03;
static const uint8_t TOUCH1_XL_INDEX = 0x04;
static const uint8_t TOUCH1_YH_INDEX = 0x05;
static const uint8_t TOUCH1_YL_INDEX = 0x06;
static const uint8_t FIRMWARE_LOW_INDEX = 0xA6;
static const uint8_t FIRMWARE_HIGH_INDEX = 0xA7;

static const uint8_t WAKEUP_CMD[1] = {0x06};

#define ERROR_CHECK(err) \
  if ((err) != i2c::ERROR_OK) { \
    ESP_LOGE(TAG, "Failed to communicate!"); \
    this->status_set_warning(); \
    return; \
  }

int16_t combine_h4l8(uint8_t high, uint8_t low) { return (high & 0x0F) << 8 | low; }

void Store::gpio_intr(Store *store) { store->touch = true; }

void LilygoTDisplayS3Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Lilygo T-Display S3 Touchscreen...");
  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->interrupt_pin_->setup();

  this->store_.pin = this->interrupt_pin_->to_isr();
  this->interrupt_pin_->attach_interrupt(Store::gpio_intr, &this->store_, gpio::INTERRUPT_FALLING_EDGE);

  this->write_register(POWER_REGISTER, WAKEUP_CMD, 1);

  uint8_t buffer[40] = {0};

  i2c::ErrorCode err;
  err = this->read_register(WORK_MODE_REGISTER, buffer, sizeof(buffer));
  ERROR_CHECK(err);

  this->firmware_version_ = buffer[FIRMWARE_HIGH_INDEX] << 8 & buffer[FIRMWARE_LOW_INDEX];
}

void LilygoTDisplayS3Touchscreen::loop() {
  if (!this->store_.touch) {
    for (auto *listener : this->touch_listeners_)
      listener->release();
    return;
  }
  this->store_.touch = false;

  uint8_t point = 0;
  uint8_t buffer[40] = {0};
  uint32_t sum_l = 0, sum_h = 0;

  i2c::ErrorCode err;
  err = this->read_register(WORK_MODE_REGISTER, buffer, sizeof(buffer));
  ERROR_CHECK(err);

  if (buffer[TOUCH_NUM_INDEX] == 0) {
    ESP_LOGV(TAG, "touch released");
  }

  point = buffer[TOUCH_NUM_INDEX] & 0x0f;

  TouchPoint tp;
  tp.id = point;
  int16_t x;
  int16_t y;

  uint8_t xh = buffer[TOUCH1_XH_INDEX];
  uint8_t xl = buffer[TOUCH1_XL_INDEX];
  uint8_t yh = buffer[TOUCH1_YH_INDEX];
  uint8_t yl = buffer[TOUCH1_YL_INDEX];

  tp.state = xh >> 6;
  x = combine_h4l8(xh, xl);
  y = combine_h4l8(yh, yl);

  switch (this->rotation_) {
    default:
    case ROTATE_0_DEGREES:
      tp.y = y;
      tp.x = x;
      break;
    case ROTATE_90_DEGREES:
      tp.x = y;
      tp.y = this->display_width_ - x;
      break;
    case ROTATE_180_DEGREES:
      tp.y = this->display_height_ - y;
      tp.x = this->display_width_ - x;
      break;
    case ROTATE_270_DEGREES:
      tp.x = this->display_height_ - y;
      tp.y = x;
      break;
  }
  tp.x += this->x_offset_;
  tp.y += this->y_offset_;

  this->x=tp.x;
  this->y=tp.y;
  if (point == 0) {
     ESP_LOGV(TAG, "Touch detected at (%d, %d). State: %d", tp.x, tp.y, tp.state);
     this->defer([this, tp]() { this->send_touch_(tp); });
  } else {
    for (auto *listener : this->touch_listeners_)
      listener->release();
  }

  this->status_clear_warning();
}

void LilygoTDisplayS3Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "Lilygo T-Display S3 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  ESP_LOGCONFIG(TAG, "  Rotation: %d", this->rotation_);
  ESP_LOGCONFIG(TAG, "  Offset: (%d, %d)", this->x_offset_, this->y_offset_);
  ESP_LOGCONFIG(TAG, "  Firmware version: %d", this->firmware_version_);
}

}  // namespace tdisplays3
}  // namespace esphome
