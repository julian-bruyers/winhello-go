// WinHelloDLL.cpp
// C++17, /EHsc, link with windowsapp.lib
//
// Notes:
// - Creates a hidden dummy owner window and centers it on the primary screen (best-effort).
// - Shows Windows Hello prompt bound to that HWND via interop.
// - Waits with a message pump to avoid STA deadlocks.
// - Cleans up the dummy window automatically.

#include <cstdint>
#include <windows.h>

#include <inspectable.h>  // IInspectable
#include <winstring.h>    // HSTRING
#include <unknwn.h>
#include <mutex>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Security.Credentials.UI.h>

#pragma comment(lib, "windowsapp")
#pragma comment(lib, "user32")

#define AUTH_SUCCESS             1
#define AUTH_FAILED_OR_CANCELLED 0
#define AUTH_NOT_AVAILABLE      -1
#define AUTH_ERROR_INTERNAL     -2

// Interop interface to bind Windows Hello to a specific HWND.
struct __declspec(uuid("39E050C3-4E74-441A-8DC0-B81104DF949C"))
IUserConsentVerifierInterop : ::IInspectable {
  virtual HRESULT __stdcall RequestVerificationForWindowAsync(
    HWND appWindow,
    HSTRING message,
    REFIID riid,
    void** asyncOperation) = 0;
};

static LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      return 0;
    default:
      return DefWindowProcW(hwnd, msg, wp, lp);
  }
}

static void centerWindowOnPrimaryScreen(HWND hwnd) {
  if (!hwnd) {
    return;
  }

  RECT rc{};
  if (!GetWindowRect(hwnd, &rc)) {
    return;
  }

  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top;

  int sw = GetSystemMetrics(SM_CXSCREEN);
  int sh = GetSystemMetrics(SM_CYSCREEN);

  int x = (sw - w) / 2;
  int y = (sh - h) / 2;

  SetWindowPos(hwnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

static HWND createHiddenOwnerWindow(HINSTANCE instance) {
  const wchar_t* kClassName = L"WinHelloDummyOwnerWindowClass";

  static std::once_flag registered;
  std::call_once(registered, [&]() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = dummyWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);
  });

  // WS_EX_TOOLWINDOW: prevents Alt-Tab presence.
  // No WS_VISIBLE: window stays hidden.
  // Give it a reasonable size so centering has meaning (even though it stays hidden).
  HWND hwnd = CreateWindowExW(
    WS_EX_TOOLWINDOW,
    kClassName,
    L"",
    WS_OVERLAPPED,
    0, 0, 300, 200,
    nullptr,
    nullptr,
    instance,
    nullptr);

  if (hwnd) {
    centerWindowOnPrimaryScreen(hwnd);
  }

  return hwnd;
}

template <typename T>
static T syncWaitWithPump(winrt::Windows::Foundation::IAsyncOperation<T> const& op) {
  winrt::handle done{ CreateEventW(nullptr, TRUE, FALSE, nullptr) };
  if (!done) {
    throw winrt::hresult_error(HRESULT_FROM_WIN32(GetLastError()));
  }

  op.Completed([h = done.get()](auto&&, auto&&) noexcept {
    SetEvent(h);
  });

  HANDLE handles[1] = { done.get() };

  while (true) {
    DWORD r = MsgWaitForMultipleObjectsEx(
      1, handles, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

    if (r == WAIT_OBJECT_0) {
      break;
    }

    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }

  return op.GetResults();
}

static int authenticateWithOwnerHwnd(HWND owner_hwnd, const std::wstring& message) {
  using namespace winrt;
  using namespace Windows::Security::Credentials::UI;

  auto availability = syncWaitWithPump(UserConsentVerifier::CheckAvailabilityAsync());
  if (availability != UserConsentVerifierAvailability::Available) {
    return AUTH_NOT_AVAILABLE;
  }

  // Bind prompt to HWND via interop.
  auto interop = get_activation_factory<UserConsentVerifier, IUserConsentVerifierInterop>();

  winrt::hstring hmsg{ message };

  winrt::Windows::Foundation::IAsyncOperation<UserConsentVerificationResult> op{ nullptr };
  winrt::check_hresult(interop->RequestVerificationForWindowAsync(
    owner_hwnd,
    reinterpret_cast<HSTRING>(winrt::get_abi(hmsg)),
    winrt::guid_of<winrt::Windows::Foundation::IAsyncOperation<UserConsentVerificationResult>>(),
    winrt::put_abi(op)));

  auto result = syncWaitWithPump(op);
  return (result == UserConsentVerificationResult::Verified)
           ? AUTH_SUCCESS
           : AUTH_FAILED_OR_CANCELLED;
}

extern "C" __declspec(dllexport) int32_t AuthenticateUser(const wchar_t* promptMessage) {
  try {
    // STA is important for UI-related WinRT calls.
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    std::wstring msg = (promptMessage && *promptMessage) ? promptMessage : L"User authentication";

    HINSTANCE instance = GetModuleHandleW(nullptr);
    HWND owner = createHiddenOwnerWindow(instance);
    if (!owner) {
      return AUTH_ERROR_INTERNAL;
    }

    // Helps in some focus/foreground cases (may still be ignored by Windows).
    AllowSetForegroundWindow(ASFW_ANY);

    int result = authenticateWithOwnerHwnd(owner, msg);

    DestroyWindow(owner);

    // Drain remaining messages related to window destruction.
    MSG m;
    while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&m);
      DispatchMessageW(&m);
    }

    return result;
  } catch (...) {
    return AUTH_ERROR_INTERNAL;
  }
}
