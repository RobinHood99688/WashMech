#include "ButtonProcess.h"

static uint32_t API_get_tick(void)
{
    return HAL_GetTick();
}

static bool API_read_button(BUTTON* handle)
{
    return ((GPIOA->IDR) & handle->_pin) ? false : true;
}

static bool read_button(BUTTON* handle)
{
    if(API_read_button(handle) == true && handle->_active == High_Active)
        return true;
    else if(API_read_button(handle) == false && handle->_active == Low_Active)
        return true;
    else
        return false;
}

static bool is_a_button(BUTTON* handle)
{
    if(handle == NULL)
        return false;
    
    return true;
}

BUTTON* create_button(uint16_t pin,
                    bool use_doublePress, 
                    bool one_longPressEventProcess, 
                    Active active, 
                    uint32_t keepStateTime, 
                    uint32_t min_freshTime_ms, 
                    uint32_t min_longPressTime_ms, 
                    uint32_t max_doublePressTime_ms, 
                    uint32_t min_eliminateShockTime_ms)
{
    BUTTON* new_button = (BUTTON *)malloc(sizeof(BUTTON));
    if (new_button == NULL)
        return NULL;

    new_button->state = None;
    new_button->state_mechine = NONE_STATE;

    new_button->_pin = pin;
    new_button->_use_doublePress = use_doublePress;
    new_button->_one_longPressEventProcess = one_longPressEventProcess;
    new_button->_active = active;
    new_button->_keepStateTime = keepStateTime;
    new_button->_min_freshTime_ms = min_freshTime_ms;
    new_button->_min_longPressTime_ms = min_longPressTime_ms;
    new_button->_max_doublePressTime_ms = max_doublePressTime_ms;
    new_button->_min_eliminateShockTime_ms = min_eliminateShockTime_ms;

    new_button->_shortPress_event_callback.callback = NULL;
    new_button->_shortPress_event_callback.context = NULL;
    new_button->_shortPress_event_callback.is_regist = false;

    new_button->_doublePress_event_callback.callback = NULL;
    new_button->_doublePress_event_callback.context = NULL;
    new_button->_doublePress_event_callback.is_regist = false;

    new_button->_longPress_event_callback.callback = NULL;
    new_button->_longPress_event_callback.context = NULL;
    new_button->_longPress_event_callback.is_regist = false;

    return new_button;
}

bool delete_button(BUTTON* handle)
{
    if(is_a_button(handle) == false)
        return false;

    free(handle);

    return true;
}

bool set_shortPress_event(BUTTON* handle, ButtonCallback callback, void* context)
{
    if(is_a_button(handle) == false)
        return false;

    handle->_shortPress_event_callback.callback = callback;
    handle->_shortPress_event_callback.context = context;
    handle->_shortPress_event_callback.is_regist = true;

    return true;
}

bool set_doublePress_event(BUTTON* handle, ButtonCallback callback, void* context)
{
    if(is_a_button(handle) == false)
        return false;

    handle->_doublePress_event_callback.callback = callback;
    handle->_doublePress_event_callback.context = context;
    handle->_doublePress_event_callback.is_regist = true;

    return true;
}

bool set_longPress_event(BUTTON* handle, ButtonCallback callback, void* context)
{
    if(is_a_button(handle) == false)
        return false;

    handle->_longPress_event_callback.callback = callback;
    handle->_longPress_event_callback.context = context;
    handle->_longPress_event_callback.is_regist = true;

    return true;
}

bool set_useDobulePress(BUTTON* handle, bool use_doublePress)
{
    if(is_a_button(handle) == false)
        return false;

    handle->_use_doublePress = use_doublePress;

    return true;
}

bool set_oneLongPressEventProcess(BUTTON* handle, bool one_longPressEventProcess)
{
    if(is_a_button(handle) == false)
        return false;

    handle->_one_longPressEventProcess = one_longPressEventProcess; 

    return true;
}

Button_State get_ButtonState(BUTTON* handle)
{
    if(is_a_button(handle) == false)
        return None;

    return handle->state;
}

ButtonUpdateReturn updata_button(BUTTON* handle)
{
    static uint32_t last_time = 0;
    uint32_t current_time = API_get_tick();

    static uint32_t last_determine_time = 0;
    uint32_t current_determine_time = 0;

    static bool state_change = false;

    if(current_time - last_time < handle->_min_freshTime_ms)
        return FRESH_TIME;

    if(state_change == true && (current_time - last_time > handle->_keepStateTime))
    {
        state_change = false;
        handle->state = None;
    }

    switch(handle->state_mechine)
    {
        case NONE_STATE:
            if(read_button(handle) == true)
            {
                handle->state_mechine = ELIMINATE_SHOCK_STATE;
                last_determine_time = API_get_tick();
            }
            else 
                return NONE;
        case ELIMINATE_SHOCK_STATE:
            if(read_button(handle) == true && current_determine_time - last_determine_time >= handle->_min_eliminateShockTime_ms)
            {
                handle->state_mechine = DETERMINE_STATE;
                last_determine_time = API_get_tick();
            }
            else 
            {
                handle->state_mechine = NONE_STATE;
                return SHAKE;
            }
        case DETERMINE_STATE:
            current_determine_time = API_get_tick();
            if(read_button(handle) == false)
                handle->state_mechine = SHORT_PRESS_STATE;
            else if(current_determine_time - last_determine_time >= handle->_min_longPressTime_ms)
                handle->state_mechine = LONG_PRESS_STATE;
            break;
        case SHORT_PRESS_STATE:
            if(handle->_use_doublePress == true)
            {
                current_determine_time = API_get_tick();
                if(read_button(handle) == true && current_determine_time - last_determine_time >= handle->_min_freshTime_ms + handle->_min_eliminateShockTime_ms * 2)
                    handle->state_mechine = DOUBLE_PRESS_STATE;
                else if(current_determine_time - last_determine_time >= handle->_max_doublePressTime_ms)
                {
                    handle->state = ShortPress;
                    if(handle->_shortPress_event_callback.is_regist)
                        handle->_shortPress_event_callback.callback(handle->_shortPress_event_callback.context);
                    handle->state_mechine = END_STATE;
                }
            }
            else
            {
                handle->state = ShortPress;
                if(handle->_shortPress_event_callback.is_regist)
                    handle->_shortPress_event_callback.callback(handle->_shortPress_event_callback.context);
                handle->state_mechine = END_STATE; 
            }
            break;
        case DOUBLE_PRESS_STATE:
            handle->state = DoublePress;
            if(handle->_doublePress_event_callback.is_regist)
                handle->_doublePress_event_callback.callback(handle->_doublePress_event_callback.context);
            if(read_button(handle) == false)
                handle->state_mechine = END_STATE;
            break;
        case LONG_PRESS_STATE:
            handle->state = LongPress;
            if(handle->_longPress_event_callback.is_regist)
                handle->_longPress_event_callback.callback(handle->_longPress_event_callback.context);
            if(read_button(handle) == false || handle->_one_longPressEventProcess == true)
                handle->state_mechine = END_STATE;
            break;
        case END_STATE:
            if(handle->_one_longPressEventProcess == true)
            {
                if(read_button(handle) == false)
                {
                    handle->state_mechine = NONE_STATE;
                    state_change = true;
                    last_time = current_time;
                    return OK;
                }
            }
            else
            {
                handle->state_mechine = NONE_STATE;
                state_change = true;
                last_time = current_time;
                return OK;
            }
        default:
            handle->state_mechine = NONE_STATE;
            last_time = current_time;
            return beERROR;
    }

    return beERROR;
}

