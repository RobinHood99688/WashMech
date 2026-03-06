#ifndef __BUTTONPROCESS_H__
#define __BUTTONPROCESS_H__

#include "main.h"
#include "stdlib.h"

#define true 1
#define false 0
typedef uint8_t bool;

typedef void (*ButtonCallback)(void* context);

typedef enum
{
    None = 0,
    ShortPress,
    LongPress,
    DoublePress,
}Button_State;

typedef enum
{
    High_Active = 0,
    Low_Active
}Active;

typedef enum 
{
    NONE_STATE = 0,
    ERROR_STATE,
    ELIMINATE_SHOCK_STATE,
    DETERMINE_STATE,
    SHORT_PRESS_STATE,
    LONG_PRESS_STATE,
    DOUBLE_PRESS_STATE,
    END_STATE,
}StateTypedef;

typedef enum 
{
    OK = 0,
    FRESH_TIME,
    NONE,
    SHAKE,
    beERROR,
}ButtonUpdateReturn;

typedef struct
{
    bool is_regist;
    ButtonCallback callback;
    void* context;
}ButtonEvent;

typedef struct
{
    StateTypedef state_mechine;
    Button_State state;

    ButtonEvent _shortPress_event_callback;
    ButtonEvent _longPress_event_callback;
    ButtonEvent _doublePress_event_callback;

    uint16_t _pin;
    bool _use_doublePress;
    bool _one_longPressEventProcess;
    Active _active;
    uint32_t _keepStateTime;
    uint32_t _min_freshTime_ms; 
    uint32_t _min_longPressTime_ms; 
    uint32_t _max_doublePressTime_ms; 
    uint32_t _min_eliminateShockTime_ms;

}BUTTON;

static bool API_read_button(BUTTON* handle);
static uint32_t API_get_tick(void);
static bool read_button(BUTTON* handle);

static bool is_a_button(BUTTON* handle);

BUTTON* create_button(uint16_t pin,
                    bool use_doublePress, 
                    bool one_longPressEventProcess, 
                    Active active, 
                    uint32_t keepStateTime, 
                    uint32_t min_freshTime_ms, 
                    uint32_t min_longPressTime_ms, 
                    uint32_t max_doublePressTime_ms, 
                    uint32_t min_eliminateShockTime_ms);
bool delete_button(BUTTON* handle);

bool set_shortPress_event(BUTTON* handle, ButtonCallback callback, void* context);
bool set_longPress_event(BUTTON* handle, ButtonCallback callback, void* context);
bool set_doublePress_event(BUTTON* handle, ButtonCallback callback, void* context);

bool set_useDobulePress(BUTTON* handle, bool use_doublePress);
bool set_oneLongPressEventProcess(BUTTON* handle, bool one_longPressEventProcess);

Button_State get_ButtonState(BUTTON* handle);

ButtonUpdateReturn updata_button(BUTTON* handle);

#endif
