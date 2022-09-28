#pragma once

#include "esphome.h"
#include "lvgl.h"
#include "TFT_eSPI.h"

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <Wire.h>
#include "FT6336U.h"

#ifndef TFT_INVERT_COLORS
#define TFT_INVERT_COLORS 0
#endif

#define RST_PIN (TOUCH_RST) // -1 if pin is connected to VCC else set pin number
FT6336U* ft6336u_touch;
static inline void FT6336U_drv_init();

const size_t buf_pix_count = LV_HOR_RES_MAX * LV_VER_RES_MAX / 5;

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[buf_pix_count];
lv_style_t switch_style;

static bool ui_idle = false;

/* LVGL callbacks - Needs to be accessible from C library */
void IRAM_ATTR cap_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void IRAM_ATTR gui_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

TFT_eSPI tft;

class LvglComponent : public Component
{
public:
  LvglComponent() {
    // This will be called once to set up the component
    // think of it as the setup() call in Arduino
    tft_setup();
    lv_init();

#if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, buf_pix_count);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = gui_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = cap_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Make unchecked checkboxes darker grey
    lv_style_init(&switch_style);
    lv_style_set_bg_color(&switch_style, lv_palette_main(LV_PALETTE_GREY));

    // lv_demo_widgets();
    // lv_demo_music();

    this->high_freq_.start(); // avoid 16 ms delay
  }

  void setup() override
  {
  }
  void loop() override
  {
    // This will be called every "update_interval" milliseconds.
    lv_timer_handler(); // called by dispatch_loop
    ui_idle = lv_disp_get_inactive_time(NULL) > 10 * 1000;
    // this->high_freq_.stop();  // decrease the counter for check
    // if (high_freq_num_requests == 1)
    //   delay(5);
    // this->high_freq_.start();  // increase the counter for main loop
  }
  float get_setup_priority() const override { return esphome::setup_priority::DATA; }

private:
  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  void tft_setup()
  {
    // This will be called once to set up the component
    // think of it as the setup() call in Arduino
    tft.begin();
    tft.invertDisplay(TFT_INVERT_COLORS);
    tft.setSwapBytes(true); /* set endianess */
    tft.setRotation(TFT_ROTATION);

#ifdef TOUCH_CS
    uint16_t calData[5] = {TOUCH_CAL_DATA};
    tft.setTouch(calData);
    #else
            FT6336U_drv_init();
#endif
  }
};

/* Update the TFT - Needs to be accessible from C library */
void IRAM_ATTR gui_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  size_t len = lv_area_get_size(area);

  /* Update TFT */
  tft.startWrite();                                      /* Start new TFT transaction */
  tft.setWindow(area->x1, area->y1, area->x2, area->y2); /* set the working window */
#ifdef USE_DMA_TO_TFT
  tft.pushPixelsDMA((uint16_t *)color_p, len); /* Write words at once */
#else
  tft.pushPixels((uint16_t *)color_p, len); /* Write words at once */
#endif
  tft.endWrite(); /* terminate TFT transaction */

  /* Tell lvgl that flushing is done */
  lv_disp_flush_ready(disp);
}

void IRAM_ATTR cap_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (ft6336u_touch->read_touch_number() == 1)
  {
    if (ui_idle) {
      lv_disp_trig_activity(NULL);
      data->state = LV_INDEV_STATE_REL;
    }
    else {
      data->point.x = ft6336u_touch->read_touch1_x();
      data->point.y = ft6336u_touch->read_touch1_y();
      data->state = LV_INDEV_STATE_PR;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

static inline void FT6336U_drv_init()
{
    // LOG_INFO(TAG_DRVR, F("Touch SDA     : %d"), TOUCH_SDA);
    // LOG_INFO(TAG_DRVR, F("Touch SCL     : %d"), TOUCH_SCL);
    // LOG_INFO(TAG_DRVR, F("Touch freq.   : %d"), TOUCH_FREQUENCY);
    // LOG_INFO(TAG_DRVR, F("Touch address : %x"), I2C_ADDR_FT6336U);

    ft6336u_touch = new FT6336U(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);
    ft6336u_touch->begin();

    // From:  M5Core2/src/M5Touch.cpp
    // By default, the FT6336 will pulse the INT line for every touch
    // event. But because it shares the Wire1 TwoWire/I2C with other
    // devices, we cannot easily create an interrupt service routine to
    // handle these events. So instead, we set the INT wire to polled mode,
    // so it simply goes low as long as there is at least one valid touch.
    // ft6336u_touch->writeByte(0xA4, 0x00);
    Wire1.beginTransmission(I2C_ADDR_FT6336U);
    Wire1.write(0xA4); // address
    Wire1.write(0x00); // data
    Wire1.endTransmission();

    // touch_scan(Wire1);

    if(ft6336u_touch->read_chip_id() != 0) {
        printf("FT6336U touch driver started chipid: %d", ft6336u_touch->read_chip_id());
    } else {
        printf("FT6336U touch driver failed to start");
    }
}

class LvglCheckbox : public Component, public Switch
{
private:
  lv_coord_t x;
  lv_coord_t y;
  lv_coord_t w;
  lv_coord_t h;

public:
  // constructor
  lv_obj_t *obj = NULL;
  LvglCheckbox(lv_coord_t _x, lv_coord_t _y, lv_coord_t _w, lv_coord_t _h)
  {
    x = _x;
    y = _y;
    w = _w;
    h = _h;
  }

  void setup() override
  {
    // This will be called by App.setup()
    obj = lv_checkbox_create(lv_scr_act());
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_checkbox_set_text(obj, (this)->get_name().c_str());

    // Set Callback
    lv_obj_add_event_cb(obj, lvgl_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
  }

  void write_state(bool state) override
  {
    // This will be called every time the user requests a state change.
    ((state) ? lv_obj_add_state(obj, LV_STATE_CHECKED) : lv_obj_clear_state(obj, LV_STATE_CHECKED));

    // Acknowledge new state by publishing it
    publish_state(state);
  }

  static void lvgl_event_cb(lv_event_t *event)
  {
    lv_obj_t *target = lv_event_get_target(event);
    LvglCheckbox *sw = (LvglCheckbox *)event->user_data;

    // printf("Clicked\n");
    bool state = (lv_obj_get_state(target) & LV_STATE_CHECKED);

    // Acknowledge new state by publishing it
    sw->publish_state(state);
  }
};


class LvglToggleButton : public Component, public Switch
{
private:
  lv_coord_t x;
  lv_coord_t y;
  lv_coord_t w;
  lv_coord_t h;

public:
  // constructor
  lv_obj_t *obj = NULL;
  lv_obj_t *label = NULL;
  LvglToggleButton(lv_coord_t _x, lv_coord_t _y, lv_coord_t _w, lv_coord_t _h)
  {
    x = _x;
    y = _y;
    w = _w;
    h = _h;
  }

  void setup() override
  {
    // This will be called by App.setup()
    obj = lv_btn_create(lv_scr_act());
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE); // enable toggle

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    label = lv_label_create(obj);
    lv_label_set_text(label, (this)->get_name().c_str());
    lv_obj_center(label);

    // Set Callback
    lv_obj_add_event_cb(obj, lvgl_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
  }

  void write_state(bool state) override
  {
    // This will be called every time the user requests a state change.
    ((state) ? lv_obj_add_state(obj, LV_STATE_CHECKED) : lv_obj_clear_state(obj, LV_STATE_CHECKED));

    // Acknowledge new state by publishing it
    publish_state(state);
  }

  void set_text(const std::string &text)
  {
    if (!label)
      return;
    lv_label_set_text(label, text.c_str());
  }

  static void lvgl_event_cb(lv_event_t *event)
  {
    lv_obj_t *target = lv_event_get_target(event);
    LvglToggleButton *sw = (LvglToggleButton *)event->user_data;

    // printf("Clicked\n");
    bool state = (lv_obj_get_state(target) & LV_STATE_CHECKED);

    // Acknowledge new state by publishing it
    sw->publish_state(state);
  }
};

class LvglSlider : public number::Number, public Component
{
private:
  lv_coord_t x;
  lv_coord_t y;
  lv_coord_t w;
  lv_coord_t h;

public:
  // constructor
  lv_obj_t *obj = NULL;
  lv_obj_t *label = NULL;
  LvglSlider(lv_coord_t _x, lv_coord_t _y, lv_coord_t _w, lv_coord_t _h)
  {
    x = _x;
    y = _y;
    w = _w;
    h = _h;
  }

  void setup() override
  {
    // This will be called by App.setup()
    obj = lv_slider_create(lv_scr_act());
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE); // enable toggle

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    // Set Callback
    //  lv_obj_add_event_cb(obj, lvgl_event_cb, LV_EVENT_VALUE_CHANGED, (void *)&(this->make_call()));
  }

protected:
  void control(float value) override
  {
    lv_slider_set_value(obj, value + 0.5, LV_ANIM_ON);
    this->publish_state(value);
  }

private:
  static void lvgl_event_cb(lv_event_t *event)
  {
    lv_obj_t *target = lv_event_get_target(event);
    NumberCall *sw = (NumberCall *)event->user_data;

    // printf("Clicked\n");
    int32_t x = lv_slider_get_value(target);
    int32_t min = lv_slider_get_min_value(target);
    int32_t max = lv_slider_get_max_value(target);
    int32_t delta = max - min;

    float state = 0.0f;
    if (delta)
      state = (float)(x - min) * 100 / (float)delta;

    // Acknowledge new state by publishing it
    sw->set_value(state);
    sw->perform();
  }
};

class LvglLight : public Component, public LightOutput
{
private:
  lv_coord_t x;
  lv_coord_t y;
  lv_coord_t w;
  lv_coord_t h;

public:
  // constructor
  lv_obj_t *obj = NULL;
  lv_obj_t *label = NULL;
  LvglLight(lv_coord_t _x, lv_coord_t _y, lv_coord_t _w, lv_coord_t _h)
  {
    x = _x;
    y = _y;
    w = _w;
    h = _h;
  }

  void setup() override
  {
    // This will be called by App.setup()
    obj = lv_slider_create(lv_scr_act());

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    // Set Callback
    lv_obj_add_event_cb(obj, lvgl_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
  }

  LightTraits get_traits() override
  {
    // return the traits this light supports
    light::LightTraits traits{};
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(LightState *state) override
  {
    // This will be called every time the user requests a state change.
    // printf("Clicked\n");
    int32_t x = lv_slider_get_value(obj);
    int32_t min = lv_slider_get_min_value(obj);
    int32_t max = lv_slider_get_max_value(obj);
    int32_t delta = max - min;

    // This will be called by the light to get a new state to be written.
    float brightness = 0.0f;
    if (delta)
      brightness = (float)(x - min) / (float)delta;

    // use any of the provided current_values methods
    state->current_values_as_brightness(&brightness);
    // print(state.get_name().c_str());
    // Write red, green and blue to HW
    // ...
  }

  static void lvgl_event_cb(lv_event_t *event)
  {
    lv_obj_t *target = lv_event_get_target(event);
    LvglLight *sw = (LvglLight *)event->user_data;

    // printf("Clicked\n");
    bool state = (lv_obj_get_state(target) & LV_STATE_CHECKED);

    // Acknowledge new state by publishing it
    //sw->publish_state(state);
  }
};

class LvglIdleSensor : public PollingComponent, public binary_sensor::BinarySensor {

 public:
  LvglIdleSensor() : PollingComponent(66) {}

  void setup() override {
  }

  void update() override {
    publish_state(ui_idle);
  }

};
