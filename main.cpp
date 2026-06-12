#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"

// Explicitly linking Discord RPC Header (Requires discord-rpc SDK dependency linked)
#include "discord_rpc.h"

using namespace Microsoft::WRL;

// Global Handle Declarations
HWND hWnd;
static TCHAR szWindowClass[] = _T("LarpMediaApp");
static TCHAR szTitle[] = _T("LarpMedia");
wil::com_ptr<ICoreWebView2Controller> webviewController;
wil::com_ptr<ICoreWebView2> webviewWindow;

// Discord Rich Presence Setup Engine
void InitializeDiscordPresence() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    
    // Discord RPC
    Discord_Initialize("1515011986021421218", &handlers, 1, NULL);
    
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = "Being a Larp God";
    discordPresence.details = "Larping";
    discordPresence.largeImageKey = "larpmedia";
    discordPresence.largeImageText = "LarpMedia";
    discordPresence.instance = 1;
    
    Discord_UpdatePresence(&discordPresence);
}

// Window Event Messaging Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
            if (webviewController != nullptr) {
                RECT bounds;
                GetClientRect(hWnd, &bounds);
                webviewController->put_Bounds(bounds);
            }
            break;
        case WM_DESTROY:
            Discord_Shutdown();
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Initialize Discord Status Thread
    InitializeDiscordPresence();

    // 2. Class Registration
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        return 1;
    }

    // 3. Create Desktop Frame Window
    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
                        1300, 850, NULL, NULL, hInstance, NULL);

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 4. Fire Up Edge Chromium Instance
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            webviewController = controller;
                            webviewController->get_CoreWebView2(&webviewWindow);
                        }
                        RECT bounds;
                        GetClientRect(hWnd, &bounds);
                        webviewController->put_Bounds(bounds);

                        // Points execution straight to the application's UI file
                        webviewWindow->Navigate(L"file:///index.html");
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    // 5. Windows Event Execution Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Discord_RunCallbacks(); // Tick Discord event loop
    }
    return (int)msg.wParam;
}
