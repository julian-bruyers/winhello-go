# winhello-go: Windows Hello Authentication for Go

A Go library providing seamless Windows Hello biometric authentication (fingerprint, facial recognition, PIN) on Windows systems.

## Overview

**winhello-go** bridges Go with the Windows Hello APIs through a native C++ DLL. Use biometric authentication in your Go applications without dealing with complex WinRT APIs directly.

### Features

- Native Windows Hello support (fingerprint, facial recognition, PIN)
- Supports AMD64 and ARM64 architectures
- Automatic DLL extraction and loading from embedded binaries
- Comprehensive error handling with specific error types
- Thread-safe implementation
- Static linking for maximum portability

## Installation

Add winhello-go to your Go project:

```bash
go get github.com/julian-bruyers/winhello-go
```

## Usage Example

```go
package main

import (
    "fmt"
    "github.com/julian-bruyers/winhello-go"
)

func main() {
    if ok, _ := winhello.Authenticate("Please verify your identity"); ok {
        fmt.Println("Welcome back!")
    } else {
        fmt.Println("Authentication failed")
    }
}
```

**With error handling:**

```go
package main

import (
    "fmt"
    "log"
    "github.com/julian-bruyers/winhello-go"
)

func main() {
    ok, err := winhello.Authenticate("Please verify your identity")
    
    if err != nil {
        log.Printf("Authentication error: %v\n", err)
        return
    }
    
    if ok {
        fmt.Println("Welcome back!")
    } else {
        fmt.Println("Authentication failed")
    }
}
```

## API

### Authenticate(promptMsg string) (bool, error)

Prompts the user to authenticate using Windows Hello.

**Parameters:**
- `promptMsg`: The message displayed to the user during authentication

**Returns:**
- `bool`: `true` if authentication was successful, `false` otherwise
- `error`: An error if something went wrong, `nil` on success

**Example:**
```go
if ok, _ := winhello.Authenticate("Verify your identity"); ok {
    // User authenticated successfully
    processRequest()
}
```

## Error Handling

The library returns specific error types for different failure scenarios:

| Error | Description |
|-------|-------------|
| `ErrOsNotSupported` | Called on non-Windows systems |
| `ErrArchNotSupported` | Unsupported CPU architecture (only amd64 and ARM64 supported) |
| `ErrNotAvailable` | Windows Hello not configured or available on the system |
| `ErrUserCanceled` | User canceled the authentication prompt |
| `ErrDLLLoad` | Failed to load the native DLL |
| `ErrInternal` | Internal Windows Hello error |

In most cases, you can ignore the error by using the blank identifier (`_`) and focus on the boolean result. Only use error handling if you need to differentiate between different failure modes.

## System Requirements

- Windows 10 or later (Windows 11 recommended)
- Go 1.25 or later
- AMD64 or ARM64 processor
- Windows Hello configured and enabled

## Development and Compilation

### Prerequisites

1. **Go 1.25+**
   - Download: https://golang.org/dl/

2. **Visual Studio 2022 or 2019** with C++ Build Tools
   - Download: https://visualstudio.microsoft.com/downloads/
   - Workload: "Desktop development with C++"
   - Components:
     - MSVC v143 (or newer) compiler
     - Windows 11 SDK
     - For ARM64: ARM64 Build Tools (`MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools (latest)`)

3. **Windows 10 SDK** or newer
   - Automatically installed with Visual Studio C++ Tools

### Installation Steps

1. **Install Visual Studio Build Tools:**
   - Download the installer from https://visualstudio.microsoft.com/downloads/
   - Select the "Desktop development with C++" workload
   - Ensure "MSVC v143 compiler" and "Windows 11 SDK" are checked
   - Complete the installation

2. **Verify Installation:**
   ```cmd
   where vcvarsall.bat
   ```
   If found, the installation was successful.

3. **Clone the Repository:**
   ```bash
   cd C:\path\to\your\projects
   git clone https://github.com/julian-bruyers/winhello-go.git
   cd winhello-go
   ```

4. **Compile the Native DLL:**
   ```bash
   go run make.go
   ```
   
   This compiles:
   - `winhello_amd64.dll` (for 64-bit systems)
   - `winhello_arm64.dll` (for ARM64 systems)
   
   The DLLs are automatically embedded into Go binaries.

5. **Run Tests (if available):**
   ```bash
   go test ./...
   ```

6. **Build Example Application:**
   ```bash
   go build -o example.exe example_main.go
   ```

## Project Structure

```
winhello-go/
├── make.go                    # Build script for C++ DLL compilation
├── native/
│   └── WinHelloDLL.cpp        # C++ implementation using WinRT APIs
├── auth_windows.go            # Windows-specific Go bindings
├── auth_stub.go               # Stub for non-Windows systems
├── errors.go                  # Error type definitions
├── dll_amd64.go               # Embedded DLL for amd64
├── dll_arm64.go               # Embedded DLL for ARM64
├── LICENSE                    # MIT License
└── README.md                  # This file
```

## Troubleshooting

### Build fails with "vcvarsall.bat not found"
- Ensure Visual Studio Build Tools are installed
- Verify that the installation includes C++ tools
- Check that the path matches your Visual Studio version (2019/2022)

### Authentication returns `ErrNotAvailable`
- Windows Hello may not be configured
- Open Settings → Accounts → Sign-in options
- Ensure at least one biometric or PIN method is enabled
- Some systems may require hardware support (TPM, camera, etc.)

### DLL fails to load at runtime
- Ensure the application is running on Windows
- Verify the target architecture (amd64 vs ARM64) matches your system
- Check that the go:generate directive ran during the build
- Clean and rebuild: `go clean && go build`

## Technical Details

The library works as follows:

1. Pre-compiled native DLLs are embedded in the Go binary
2. The DLL is extracted to a temporary directory at runtime
3. The DLL is loaded and the `AuthenticateUser` function is called
4. The C++ function returns an integer result code
5. The result code is converted to a bool and error for the Go caller

The native C++ code interfaces with Windows WinRT APIs and ensures all exceptions are caught before crossing the C++/Go boundary to maintain safety and stability.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

Copyright (c) 2026 Julian Bruyers

## Contributing

Contributions are welcome! Please report issues or submit pull requests to:

https://github.com/julian-bruyers/winhello-go
