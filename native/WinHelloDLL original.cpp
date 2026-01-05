// ============================================================================
// WinHelloDLL.cpp
// ----------------------------------------------------------------------------
// Implements a bridge between the Windows Runtime (WinRT) "Windows Hello" API
// and a plain C-interface usable by Go (cgo).
//
// Compilation Requirements:
//   - Standard: C++17 (/std:c++17) for WinRT language projection support.
//   - Exception Handling: standard (/EHsc).
//   - Runtime Library: Static Multi-threaded (/MT) recommended for portability.
// ============================================================================

#include <cstdint>
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Security.Credentials.UI.h>

// Link against the 'windowsapp' library to resolve WinRT symbols
#pragma comment(lib, "windowsapp")

#define AUTH_SUCCESS             1
#define AUTH_FAILED_OR_CANCELLED 0
#define AUTH_NOT_AVAILABLE      -1
#define AUTH_ERROR_INTERNAL     -2

namespace winhello {
    using namespace winrt;
    using namespace Windows::Security::Credentials::UI;

    static int Authenticate(const std::wstring& message) {
        // Check if Windows Hello is available (installed and configured)
        if ( UserConsentVerifier::CheckAvailabilityAsync().get() != UserConsentVerifierAvailability::Available) {
            return AUTH_NOT_AVAILABLE;
        }

        // Request verification via Windows Hello
        if (UserConsentVerifier::RequestVerificationAsync(message).get() == UserConsentVerificationResult::Verified) {
            return AUTH_SUCCESS;
        }

        return AUTH_FAILED_OR_CANCELLED;
    }
}

/**
 * Public C-Interface
 * This function acts as the "Exception Boundary". No C++ exceptions must ever
 * escape this function, as they would crash the calling Go runtime (undefined behavior).
 */
extern "C" __declspec(dllexport) int32_t AuthenticateUser(const wchar_t* promptMessage) {
    try {
        // Ensure thread is COM-initialized
        winrt::init_apartment();

        std::wstring msg = (promptMessage && *promptMessage) ? promptMessage : L"User authentication";
        return winhello::Authenticate(msg);

    // Wildcard catch to prevent any C++ exception from crossing dll boundary (Go runtime safety)
    } catch (...) {
        return AUTH_ERROR_INTERNAL;
    }
}
