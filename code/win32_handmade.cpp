#include <Windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool isRunning;

global_variable BITMAPINFO bitmapInfo;
global_variable VOID *bitmapBuffer;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

internal void Win32ResizeDIBSection(int width, int height)
{
    if (bitmapHandle) 
    {
        DeleteObject(bitmapHandle);
    }

    if (!bitmapDeviceContext)
    {
        bitmapDeviceContext = CreateCompatibleDC(0);
    }

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;

    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    bitmapHandle = CreateDIBSection(
        bitmapDeviceContext,
        &bitmapInfo,
        DIB_RGB_COLORS,
        &bitmapBuffer,
        NULL,
        NULL);
}

internal void Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height) 
{	
    StretchDIBits(
        deviceContext,
        x,y,width,height,
        x,y,width,height,
        bitmapBuffer,
        &bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
        );
}

internal void CalculateWidthAndHeight(LPRECT rect, _Out_ int* width, _Out_ int* height) 
{
    *width = rect->right - rect->left;
    *height = rect->bottom - rect->top;
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
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int w = paint.rcPaint.right - x;
            int h = paint.rcPaint.bottom - y;
            Win32UpdateWindow(deviceContext, x, y, w, h);

            //PatBlt(deviceContext, x, y, w, h, WHITENESS);

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
    windowClass.lpfnWndProc = MainWindowCallback;
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
            while (isRunning)
            {
                BOOL msgResult = GetMessage(&message, 0, 0, 0);
                if (msgResult > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                else
                {
                    break;
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