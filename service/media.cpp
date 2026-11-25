// Runs in the signed-in user's session, where Windows exposes the current media player.
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

#include <cstddef>
#include <vector>

using winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager;

namespace {
constexpr USHORT kTransportPage = 0xFFDA;
constexpr USHORT kTransportCollection = 0xA2D0;

void ApplyTransport(const BYTE *report)
{
    if (report[0] != 2 || (report[1] != 0xB0 && report[1] != 0xB1))
        return; // releases and unrelated reports

    auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    auto session = manager.GetCurrentSession();
    if (!session)
        return;
    // Await in arrival order. No cached playback-state guess, and no toggle fallback.
    if (report[1] == 0xB0)
        session.TryPlayAsync().get();
    else
        session.TryPauseAsync().get();
}

void ReadTransport(HRAWINPUT input)
{
    UINT size = 0;
    constexpr UINT dataOffset = offsetof(RAWINPUT, data.hid.bRawData);
    if (GetRawInputData(input, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0 ||
        size < dataOffset || size > 65536)
        return;
    std::vector<BYTE> bytes(size);
    if (GetRawInputData(input, RID_INPUT, bytes.data(), &size, sizeof(RAWINPUTHEADER)) != bytes.size())
        return;
    auto raw = reinterpret_cast<const RAWINPUT *>(bytes.data());
    if (raw->header.dwType != RIM_TYPEHID)
        return;
    auto &hid = raw->data.hid;
    if (hid.dwSizeHid != 3 || hid.dwCount > (bytes.size() - dataOffset) / 3)
        return;
    for (DWORD i = 0; i < hid.dwCount; ++i)
        ApplyTransport(hid.bRawData + i * 3);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_INPUT) {
        try {
            ReadTransport(reinterpret_cast<HRAWINPUT>(lp));
        } catch (const winrt::hresult_error &e) {
            OutputDebugStringW(e.message().c_str()); // player closed or media broker unavailable
        } catch (const std::bad_alloc &) {
            OutputDebugStringW(L"LdacMedia: could not allocate input buffer");
        }
    }
    return DefWindowProcW(window, message, wp, lp);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    winrt::handle singleton{CreateMutexW(nullptr, FALSE, L"Local\\LdacA2dp.Xm6.Media")};
    if (!singleton)
        return 1;
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"LdacA2dpMediaInput";
    if (!RegisterClassW(&wc))
        return 1;
    HWND window = CreateWindowExW(0, wc.lpszClassName, L"", 0, 0, 0, 0, 0,
                                   HWND_MESSAGE, nullptr, instance, nullptr);
    RAWINPUTDEVICE device{kTransportPage, kTransportCollection, RIDEV_INPUTSINK, window};
    if (!window || !RegisterRawInputDevices(&device, 1, sizeof(device)))
        return 1;
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
        DispatchMessageW(&msg);
    return 0;
}
