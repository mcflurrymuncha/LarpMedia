#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include "WebView2.h"
#include "discord_rpc.h"

using namespace Microsoft::WRL;

HWND hWnd;
static TCHAR szWindowClass[] = _T("LarpMediaApp");
static TCHAR szTitle[] = _T("LarpMedia Player Engine");

// Standard WRL ComPtr declarations replacing WIL
ComPtr<ICoreWebView2Controller> webviewController;
ComPtr<ICoreWebView2> webviewWindow;

void UpdateDiscordTrackPresence(const char* trackName, const char* durationStr) {
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    
    discordPresence.activityType = 2; // 2 = Listening status
    discordPresence.state = trackName;
    discordPresence.details = "Processing Harmonics";
    discordPresence.largeImageKey = "larpmedia_purple";
    discordPresence.largeImageText = "LarpMedia Player v1.0";
    discordPresence.instance = 1;
    
    Discord_UpdatePresence(&discordPresence);
}

void InitializeDiscordPresence() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize("1515011986021421218", &handlers, 1, NULL);
    UpdateDiscordTrackPresence("Idle / Deck Empty", "0:00");
}

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
    InitializeDiscordPresence();

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

    if (!RegisterClassEx(&wcex)) return 1;

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
                        1300, 850, NULL, NULL, hInstance, NULL);

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

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

                        webviewWindow->SetVirtualHostNameToFolderMapping(
                            L"larpmedia.local", L".", COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW
                        );
                        
                        webviewWindow->Navigate(L"https://larpmedia.local/index.html");
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Discord_RunCallbacks();
    }
    return (int)msg.wParam;
}
