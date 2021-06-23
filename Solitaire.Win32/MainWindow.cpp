#include "pch.h"
#include "Solitaire.Core.h"
#include "MainWindow.h"
#include <Windowsx.h>

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics;
}

const std::wstring MainWindow::ClassName = L"Solitaire.Win32.MainWindow";

void MainWindow::RegisterWindowClass()
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIconW(instance, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.lpszClassName = ClassName.c_str();
    wcex.hIconSm = LoadIconW(instance, IDI_APPLICATION);
    winrt::check_bool(RegisterClassExW(&wcex));
}

MainWindow::MainWindow(
    std::wstring const& titleString,
    std::shared_ptr<ISolitaire> const& game,
    winrt::Windows::Graphics::SizeInt32 const& windowSize)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    m_game = game;

    winrt::check_bool(CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, ClassName.c_str(), titleString.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowSize.Width, windowSize.Height, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);
}

LRESULT MainWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    if (WM_DESTROY == message)
    {
        PostQuitMessage(0);
        return 0;
    }

    switch (message)
    {
    case WM_MOUSEMOVE:
    {
        auto rawX = GET_X_LPARAM(lparam);
        auto rawY = GET_Y_LPARAM(lparam);
        winrt::float2 point = { (float)rawX, (float)rawY };
        m_game->OnPointerMoved(point);
    }
    break;
    case WM_SIZE:
    case WM_SIZING:
    {
        auto windowSize = GetWindowSize();
        m_game->OnParentSizeChanged({ (float)windowSize.Width, (float)windowSize.Height });
    }
    break;
    case WM_LBUTTONDOWN:
    {
        auto rawX = GET_X_LPARAM(lparam);
        auto rawY = GET_Y_LPARAM(lparam);
        winrt::float2 point = { (float)rawX, (float)rawY };
        m_game->OnPointerPressed(point, false, false);
    }
        break;
    case WM_LBUTTONUP:
    {
        auto rawX = GET_X_LPARAM(lparam);
        auto rawY = GET_Y_LPARAM(lparam);
        winrt::float2 point = { (float)rawX, (float)rawY };
        m_game->OnPointerReleased(point, false, false);
    }
        break;
    case WM_KEYUP:
    {
        auto isControlDown = GetKeyState(static_cast<int>(winrt::Windows::System::VirtualKey::Control)) != 0;
        auto key = static_cast<winrt::Windows::System::VirtualKey>(wparam);
        m_game->OnKeyUp(key, isControlDown);
    }
        break;
    }

    return base_type::MessageHandler(message, wparam, lparam);
}

winrt::Windows::Graphics::SizeInt32 MainWindow::GetWindowSize()
{
    RECT rect = {};
    winrt::check_bool(GetClientRect(m_window, &rect));
    auto windowWidth = rect.right - rect.left;
    auto windowHeight = rect.bottom - rect.top;
    winrt::SizeInt32 windowSize = { windowWidth, windowHeight };
    return windowSize;
}