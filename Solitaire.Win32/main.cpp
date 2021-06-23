#include "pch.h"
#include "Solitaire.Core.h"
#include "MainWindow.h"
#include <shellapi.h>

// https://docs.microsoft.com/en-us/dotnet/api/system.io.filenotfoundexception?view=net-5.0
#define COR_E_FILENOTFOUND 0x80070002

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Graphics;
    using namespace Windows::Storage;
}

namespace util
{
    using namespace robmikh::common::desktop;
}

winrt::hstring GetAssetsPath();
winrt::IAsyncOperation<winrt::StorageFolder> GetAssetsFolderAsync(std::optional<winrt::hstring> const& assetsPath);

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // Initialize COM
    winrt::init_apartment();
    
    // Register our window classes
    MainWindow::RegisterWindowClass();

    // Get command line arguments
    auto argc = 0;
    auto argv = winrt::check_pointer(CommandLineToArgvW(GetCommandLineW(), &argc));
    std::vector<std::wstring> args(argv + 1, argv + argc);

    // Try and get the assets directory
    winrt::StorageFolder folder{ nullptr };
    std::optional<winrt::hstring> assetsPath = std::nullopt;
    if (args.size() > 0)
    {
        assetsPath = std::optional(winrt::hstring(args[0]));
    }
    try
    {
        folder = GetAssetsFolderAsync(assetsPath).get();
    }
    catch (winrt::hresult_error const& error)
    {
        if (error.code() != COR_E_FILENOTFOUND)
        {
            throw;
        }
    }
    if (folder == nullptr)
    {
        MessageBoxW(nullptr,
            L"Could not find 'Assets' directory!",
            L"Solitaire.Win32",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    // Create the DispatcherQueue that the compositor needs to run
    auto controller = util::CreateDispatcherQueueControllerForCurrentThread();

    // Initialize Composition
    auto compositor = winrt::Compositor();
    auto root = compositor.CreateContainerVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });

    // Create our game
    winrt::SizeInt32 windowSize = { 800, 600 };
    auto game = CreateSolitaireAsync(root, winrt::float2{ (float)windowSize.Width, (float)windowSize.Height }, folder).get();

    // Create our main window
    auto window = MainWindow(L"Solitaire", game, windowSize);

    // Hookup our visual tree to the window
    auto target = window.CreateWindowTarget(compositor);
    target.Root(root);

    // Message pump
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

winrt::hstring GetAssetsPath()
{
    auto currentPath = std::filesystem::current_path();
    currentPath.append(L"Assets");
    return winrt::hstring(currentPath.wstring());
}

winrt::IAsyncOperation<winrt::StorageFolder> GetAssetsFolderAsync(std::optional<winrt::hstring> const& assetsPath)
{
    co_return co_await winrt::StorageFolder::GetFolderFromPathAsync(assetsPath.value_or(GetAssetsPath()));
}