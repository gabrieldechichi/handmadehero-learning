#include <Windows.h>

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
        OutputDebugString("Destroy");
        break;
    case WM_CLOSE:
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
            PatBlt(deviceContext, x, y, w, h, WHITENESS);
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
            while (true)
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