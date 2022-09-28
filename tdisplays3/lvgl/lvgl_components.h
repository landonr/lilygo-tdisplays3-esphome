#pragma once

#include <esphome.h>
#include <esphome/core/component.h>

enum LvglSwitchRestoreMode {
    LVGL_SWITCH_RESTORE_DEFAULT_OFF,
    LVGL_SWITCH_RESTORE_DEFAULT_ON,
    LVGL_SWITCH_RESTORE_ALWAYS_OFF,
    LVGL_SWITCH_RESTORE_ALWAYS_ON
};


class LvglSwitch : public esphome::Component, public esphome::switch_::Switch
{
private:
  lv_obj_t *obj_{nullptr};
  LvglSwitchRestoreMode restore_mode_{LVGL_SWITCH_RESTORE_ALWAYS_OFF};

public:
  LvglSwitch(lv_obj_t *obj)
    : obj_{obj}
  {}

  LvglSwitch(lv_obj_t *obj, LvglSwitchRestoreMode restore_mode)
    : obj_{obj},
      restore_mode_{restore_mode}
  {}

  void setup() override {
    lv_obj_add_event_cb(obj_, lvgl_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

    bool initial_state{false};

    switch (restore_mode_) {
        case LVGL_SWITCH_RESTORE_DEFAULT_OFF:
            initial_state = this->get_initial_state().value_or(false);
            break;
        case LVGL_SWITCH_RESTORE_DEFAULT_ON:
            initial_state = this->get_initial_state().value_or(true);
            break;
        case LVGL_SWITCH_RESTORE_ALWAYS_OFF:
            initial_state = false;
            break;
        case LVGL_SWITCH_RESTORE_ALWAYS_ON:
            initial_state = true;
            break;
    }

    write_state(initial_state);
  }

  void write_state(bool state) override
  {
    if (state) {
      lv_obj_add_state(obj_, LV_STATE_CHECKED);
    }
    else {
      lv_obj_clear_state(obj_, LV_STATE_CHECKED);
    }

    // Acknowledge new state by publishing it
    publish_state(state);
  }

  static void lvgl_event_cb(lv_event_t *event)
  {
    lv_obj_t *target = lv_event_get_target(event);
    LvglSwitch *sw = (LvglSwitch *)event->user_data;

    bool state = (lv_obj_get_state(target) & LV_STATE_CHECKED);

    sw->publish_state(state);
  }
};
