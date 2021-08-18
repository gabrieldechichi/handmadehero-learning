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


global_variable bool isRunning;

global_variable BITMAPINFO bitmapInfo;
global_variable VOID *bitmapBuffer;
global_variable int bitmapWidth;
global_variable int bitmapHeight;

internal void CalculateWidthAndHeight(LPRECT rect, _Out_ int* width, _Out_ int* height) 
{
    *width = rect->right - rect->left;
    *height = rect->bottom - rect->top;
}

internal void RenderWeirdGradient(int xOffset, int yOffset) 
{	
    for (int y = 0; y < bitmapHeight; y++)
    {
        for (int x = 0; x < bitmapWidth; x++)
        {
            uint32* pixel = ((uint32*)bitmapBuffer) + y*bitmapWidth + x;
            //mem layour: b g r x (little endian architecture)
            //register layout: xx rr gg bb 

            uint8 red = (uint8)(x + xOffset);
            uint8 green = (uint8)(y + yOffset);
            uint8 blue = (uint8)yOffset;
            blue = 0;

            *pixel = (red << 16) | (green << 8) | (blue << 0);
        }
    }
}

internal void Win32ResizeDIBSection(int width, int height)
{
    if (bitmapBuffer) 
    {
        VirtualFree(bitmapBuffer, 0, MEM_RELEASE);
    }

    bitmapWidth = width;
    bitmapHeight = height;

    ///Create bitmpa info
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight = -bitmapHeight;

    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    ///

    int bytesPerPixel = 4;
    int bufferSize = 4 * bitmapWidth * bitmapHeight;
    bitmapBuffer = VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC deviceContext, RECT* windowRect) 
{
    int x = windowRect->left;
    int y = windowRect->top;
    int windowWidth, windowHeight;
    CalculateWidthAndHeight(windowRect, &windowWidth, &windowHeight);
    StretchDIBits(
        deviceContext,
        0,0,bitmapWidth, bitmapHeight,
        x,y,windowWidth, windowHeight,
        bitmapBuffer,
        &bitmapInfo,
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
        RECT rect;
        GetClientRect(window, &rect);
        int width, height;
        CalculateWidthAndHeight(&rect, &width, &height);
        Win32ResizeDIBSection(width, height);
        break;
    case WM_DESTROY:
        isRunning = false;
        OutputDebugString("Destroy");
        break;
    case WM_CLOSE:
        isRunning = false;
        OutputDebugString("Close");
        break;
    case WM_ACTIVATEAPP:
        OutputDebugString("Activate");
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            Win32UpdateWindow(deviceContext, &paint.rcPaint);
            EndPaint(window, &paint); 	
        }
        break;
    default:
        //OutputDebugString("Size");
        result = DefWindowProc(window, message, wParam, lParam);
        break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
     WNDCLASS windowClass = {};

    //TODO(dechichi): Chekc if CS_OWNDC, CS_HREDRAW, CS_VREDRAW stil matter
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
            MSG message;
            isRunning = true;

            int xOffset = 0;
            int yOffset = 0;
            while (isRunning)
            {
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {	
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                ++xOffset;
                ++yOffset;
                RenderWeirdGradient(xOffset, yOffset);

                RECT rect;
                GetClientRect(wHandle, &rect);
                HDC deviceContext = GetDC(wHandle);
                Win32UpdateWindow(deviceContext, &rect);
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