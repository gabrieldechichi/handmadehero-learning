#include <Windows.h>
#include "defines.h"
#include "win32_xinput.h"
#include "win32_dsound.h"

struct win32_offscreen_buffer
{
    BITMAPINFO bitmapInfo;
    VOID *bitmapBuffer;
    int bitmapWidth;
    int bitmapHeight;
    int bytesPerPixel;
};

global_variable bool globalIsRunning;
global_variable win32_offscreen_buffer globalBackBuffer;

struct win32_window_size
{
    int width;
    int height;
};

internal win32_window_size Win32GetWindowSize(HWND window)
{
    RECT rect;
    GetClientRect(window, &rect);
    win32_window_size size;
    size.width = rect.right - rect.left;
    size.height = rect.bottom - rect.top;
    return size;
}

internal void RenderWeirdGradient(win32_offscreen_buffer *bitmap, int xOffset, int yOffset)
{
    for (int y = 0; y < bitmap->bitmapHeight; y++)
    {
        for (int x = 0; x < bitmap->bitmapWidth; x++)
        {
            uint32 *pixel = ((uint32 *)bitmap->bitmapBuffer) + y * bitmap->bitmapWidth + x;
            //mem layour: b g r x (little endian architecture)
            //register layout: xx rr gg bb

            uint8 red = (uint8)(x + xOffset);
            uint8 green = (uint8)(y + yOffset);
            uint8 blue = (uint8)yOffset;
            // blue = 0;

            *pixel = (red << 16) | (green << 8) | (blue << 0);
        }
    }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
    if (buffer->bitmapBuffer)
    {
        VirtualFree(buffer->bitmapBuffer, 0, MEM_RELEASE);
    }

    buffer->bitmapWidth = width;
    buffer->bitmapHeight = height;
    buffer->bytesPerPixel = 4;

    ///Create bitmpa info
    buffer->bitmapInfo.bmiHeader.biSize = sizeof(buffer->bitmapInfo.bmiHeader);
    buffer->bitmapInfo.bmiHeader.biWidth = buffer->bitmapWidth;
    buffer->bitmapInfo.bmiHeader.biHeight = -buffer->bitmapHeight;

    buffer->bitmapInfo.bmiHeader.biPlanes = 1;
    buffer->bitmapInfo.bmiHeader.biBitCount = 32;
    buffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;
    ///

    int bufferSize = buffer->bytesPerPixel * buffer->bitmapWidth * buffer->bitmapHeight;
    buffer->bitmapBuffer = VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_READWRITE);
}

/*
avoid passing pointers to things on the stack because this 
makes it harder for the compile to optimize (inline or assume not pointer aliasing)
*/
internal void Win32CopyBufferToWindow(win32_offscreen_buffer *buffer, HDC deviceContext, int windowWidth, int windowHeight)
{
    //todo(gabriel): aspect ratio
    StretchDIBits(
        deviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->bitmapWidth, buffer->bitmapHeight,
        buffer->bitmapBuffer,
        &buffer->bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
}

internal void Win32VibrateController(int controllerIndex, WORD leftVibration, WORD rightVibration)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = leftVibration;
    vibration.wRightMotorSpeed = rightVibration;
    XInputSetState(controllerIndex, &vibration);
}

internal void Win32GamepadHandleInput(int *xOffset, int *yOffset)
{
    for (int controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
    {
        XINPUT_STATE controllerState;
        if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
        {
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

            bool up = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
            bool down = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
            bool left = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
            bool right = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

            bool start = pad->wButtons & XINPUT_GAMEPAD_START;
            bool back = pad->wButtons & XINPUT_GAMEPAD_BACK;
            bool leftShoulder = pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
            bool rightShoulder = pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

            bool aButton = pad->wButtons & XINPUT_GAMEPAD_A;
            bool bButton = pad->wButtons & XINPUT_GAMEPAD_B;
            bool xButton = pad->wButtons & XINPUT_GAMEPAD_X;
            bool yButton = pad->wButtons & XINPUT_GAMEPAD_Y;

            int16 stickX = pad->sThumbLX;
            int16 stickY = pad->sThumbLY;

            if (aButton)
            {
                (*yOffset)++;
                Win32VibrateController(controllerIndex, 32000, 32000);
            }
            else
            {
                Win32VibrateController(controllerIndex, 0, 0);
            }
        }
    }
}

internal void Win32HandleKeyboardInput(WPARAM wParam, LPARAM lParam)
{
    uint32 vkCode = wParam;
    bool wasDown = ((1 << 30) & lParam) != 0;
    bool isDown = ((1 << 31) & lParam) == 0;

    if (wasDown != isDown)
    {
        switch (vkCode)
        {
        case 'W':
        case VK_UP:
            OutputDebugString("W");
            if (!wasDown)
            {
                OutputDebugString("W Pressed");
            }
            else if (wasDown && !isDown)
            {
                OutputDebugString("W Released");
            }
            break;
        case 'A':
        case VK_LEFT:
            OutputDebugString("A");
            break;
        case 'S':
        case VK_DOWN:
            OutputDebugString("S");
            break;
        case 'D':
        case VK_RIGHT:
            OutputDebugString("D");
            break;
        case 'Q':
            OutputDebugString("Q");
            break;
        case 'E':
            OutputDebugString("E");
            break;
        case VK_ESCAPE:
            OutputDebugString("Escape");
            break;
        case VK_SPACE:
            OutputDebugString("Space");
            break;
        case VK_F4:
        {
            bool altKeyWasDown = (lParam & (1 << 29));
            if (altKeyWasDown)
            {
                globalIsRunning = false;
            }
        }
        break;
        default:
            break;
        }
    }
}

LRESULT MainWindowCallback(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
    case WM_SIZE:
        OutputDebugString("Size");
        break;
    case WM_DESTROY:
        globalIsRunning = false;
        OutputDebugString("Destroy");
        break;
    case WM_CLOSE:
        globalIsRunning = false;
        OutputDebugString("Close");
        break;
    case WM_ACTIVATEAPP:
        OutputDebugString("Activate");
        break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
        Win32HandleKeyboardInput(wParam, lParam);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC deviceContext = BeginPaint(window, &paint);
        auto windowSize = Win32GetWindowSize(window);
        Win32CopyBufferToWindow(&globalBackBuffer, deviceContext, windowSize.width, windowSize.height);
        EndPaint(window, &paint);
    }
    break;
    default:
        result = DefWindowProc(window, message, wParam, lParam);
        break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
    //Initialize backbuffer
    Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);

    //Initialize XInput
    Win32LoadXInput();

    WNDCLASS windowClass = {};

    //HREDRAW and VREDRAW makes sure how window is repainted if it gets scaled
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = (WNDPROC)MainWindowCallback;
    windowClass.hInstance = instance;
    //windowClass.hIcon;
    windowClass.lpszClassName = "HandmadeHeroWindow";

    if (RegisterClass(&windowClass))
    {
        HWND wHandle = CreateWindowEx(
            0,
            windowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instance,
            0);

        if (wHandle)
        {
            win32_audio_player player = {};
            player.samplesPerSecond = 48000;
            player.toneHz = 256;
            player.toneVolume = 3000;
            player.runningSampleIndex = 0;
            player.period = player.samplesPerSecond / player.toneHz;
            player.bytesPerSample = sizeof(int16) * 2;
            player.bufferSize = player.samplesPerSecond * player.bytesPerSample;
            Win32InitDSound(wHandle, player.samplesPerSecond, player.bufferSize);

            //Since we specify CS_OWNDC we can use this device context forever
            HDC deviceContext = GetDC(wHandle);
            MSG message;
            globalIsRunning = true;

            int xOffset = 0;
            int yOffset = 0;
            while (globalIsRunning)
            {
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                ++xOffset;
                Win32GamepadHandleInput(&xOffset, &yOffset);

                //Render test
                {
                    RenderWeirdGradient(&globalBackBuffer, xOffset, yOffset);
                    auto windowSize = Win32GetWindowSize(wHandle);
                    Win32CopyBufferToWindow(&globalBackBuffer, deviceContext, windowSize.width, windowSize.height);
                    ReleaseDC(wHandle, deviceContext);
                }

                //Sound test
                Win32PlaySound(&player);
            }
        }
        else
        {
            //todo: logging
        }
    }
    else
    {
        //TODO(dechichi): logging
    }

    return (0);
}