#include <Windows.h>
#include <stdint.h>

#define local_persist static
#define global_variable static
#define internal static

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

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

internal void RenderWeirdGradient(win32_offscreen_buffer bitmap, int xOffset, int yOffset)
{	
    for (int y = 0; y < bitmap.bitmapHeight; y++)
    {
        for (int x = 0; x < bitmap.bitmapWidth; x++)
        {
            uint32* pixel = ((uint32*)bitmap.bitmapBuffer) + y*bitmap.bitmapWidth + x;
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

internal win32_offscreen_buffer Win32ResizeDIBSection(win32_offscreen_buffer buffer, int width, int height)
{
    if (buffer.bitmapBuffer) 
    {
        VirtualFree(buffer.bitmapBuffer, 0, MEM_RELEASE);
    }

    buffer.bitmapWidth = width;
    buffer.bitmapHeight = height;
    buffer.bytesPerPixel = 4;

    ///Create bitmpa info
    buffer.bitmapInfo.bmiHeader.biSize = sizeof(buffer.bitmapInfo.bmiHeader);
    buffer.bitmapInfo.bmiHeader.biWidth = buffer.bitmapWidth;
    buffer.bitmapInfo.bmiHeader.biHeight = -buffer.bitmapHeight;

    buffer.bitmapInfo.bmiHeader.biPlanes = 1;
    buffer.bitmapInfo.bmiHeader.biBitCount = 32;
    buffer.bitmapInfo.bmiHeader.biCompression = BI_RGB;
    ///

    int bufferSize = buffer.bytesPerPixel * buffer.bitmapWidth * buffer.bitmapHeight;
    buffer.bitmapBuffer = VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_READWRITE);

    return buffer;
}

/*
avoid passing pointers to things on the stack because this 
makes it harder for the compile to optimize (inline or assume not pointer aliasing)
*/
internal void Win32CopyBufferToWindow(win32_offscreen_buffer buffer, HDC deviceContext, int windowWidth, int windowHeight)
{
    //todo(gabriel): aspect ratio
    StretchDIBits(
        deviceContext,
        0,0, windowWidth, windowHeight,
        0,0, buffer.bitmapWidth, buffer.bitmapHeight,
        buffer.bitmapBuffer,
        &buffer.bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
        );
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
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            auto windowSize = Win32GetWindowSize(window);
            Win32CopyBufferToWindow(globalBackBuffer, deviceContext, windowSize.width, windowSize.height);
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
     WNDCLASS windowClass = {};

    //HREDRAW and VREDRAW makes sure how window is repainted if it gets scaled
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = (WNDPROC) MainWindowCallback;
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
            //
            globalBackBuffer = Win32ResizeDIBSection(globalBackBuffer, 1280, 720);   
            //

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
                ++yOffset;
                RenderWeirdGradient(globalBackBuffer, xOffset, yOffset);


                {
                    HDC deviceContext = GetDC(wHandle);
                    auto windowSize = Win32GetWindowSize(wHandle);
                    Win32CopyBufferToWindow(globalBackBuffer, deviceContext, windowSize.width, windowSize.height);
                    ReleaseDC(wHandle, deviceContext);	
                }
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