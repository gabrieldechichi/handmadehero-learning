#pragma once

#include <xinput.h>
#include "defines.h"
#include <Windows.h>

#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)

///// BEGIN XInputGetStat
typedef XINPUT_GET_STATE(x_input_get_state);
XINPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_
///// END XInputGetStat

typedef XINPUT_SET_STATE(x_input_set_state);
XINPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

void Win32LoadXInput(void)
{
    HMODULE xinputLib = LoadLibrary("xinput1_4.dll");
    if (xinputLib) 
    {
        XInputGetState = (x_input_get_state*) GetProcAddress(xinputLib, "XInputGetState");
        XInputSetState = (x_input_set_state*) GetProcAddress(xinputLib, "XInputSetState");
    }
}


