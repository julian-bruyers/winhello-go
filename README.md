<!--  README badges  -->
<p>
  <a href="#platform-support"><img src="https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white" align="right" height="20"></a>
  <a href="#installation"><img src="https://img.shields.io/badge/Go-00ADD8?logo=go&logoColor=white&labelColor=2D3748&color=2D3748" align="right" height="20" hspace="5"></a>
  <a href="https://github.com/julian-bruyers/winhello-go/releases"><img src="https://img.shields.io/github/v/release/julian-bruyers/winhello-go?label=Latest&labelColor=2D3748&color=003087" height="20"></a>
  <a href="https://github.com/julian-bruyers/winhello-go/blob/main/LICENSE"><img src="https://img.shields.io/github/license/julian-bruyers/winhello-go?&label=License&logo=opensourceinitiative&logoColor=ffffff&labelColor=2D3748&color=2D3748" height="20" hspace="5"></a><a href="https://goreportcard.com/report/github.com/julian-bruyers/winhello-go"><img src="https://goreportcard.com/badge/github.com/julian-bruyers/winhello-go" height="20"></a>
</p>

# winhello-go: Windows Hello Authentication for Go

A **lightweight**, **zero external dependencies** and **simple to use** Go library providing Windows Hello biometric authentication (fingerprint, facial recognition, PIN) on Windows systems.

## Overview
**winhello-go** bridges Go with the Windows Hello APIs through a native C++ DLL. Use biometric authentication in your Go applications without dealing with complex WinRT APIs directly.

### Features
- Native Windows Hello support (fingerprint, facial recognition, PIN)
- Supports AMD64 and ARM64 architectures
- Thread-safe implementation
- No external dependencies
- Extremely easy to use

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
    if isAuthenticated, _ := winhello.Authenticate("Verify your identity for winhello-go test"); isAuthenticated {
        fmt.Println("Authentication successful!")
    } else {
        fmt.Println("Authentication failed!")
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
	isAuthenticated, err := winhello.Authenticate("Verify your identity for winhello-go test")

	if err != nil {
		log.Fatal(err)
	}

	if isAuthenticated {
		fmt.Println("Authentication successful!")
	} else {
		fmt.Println("Authentication failed!")
	}
}
```

## API / Usage

**winhello.Authenticate(promptMsg string) (bool, error)**
Prompts the user to authenticate using Windows Hello.

_Parameters:_
- `promptMsg`: The message displayed to the user during authentication

_Returns:_
- `bool`: `true` if authentication was successful, `false` otherwise
- `error`: An error if something went wrong, `nil` on success

| Error | Description |
|-------|-------------|
| `ErrOsNotSupported` | Called on non-Windows systems |
| `ErrArchNotSupported` | Unsupported CPU architecture (only AMD64 and ARM64 supported) |
| `ErrNotAvailable` | Windows Hello not configured or available on the system |
| `ErrUserCanceled` | User canceled the authentication prompt |
| `ErrDLLLoad` | Failed to load the native DLL |
| `ErrInternal` | Internal Windows Hello DLL error |
| `ErrUnknownDllReturnVal` | The embedded winhello_arch.dll returned an unhadled "status" code |

## System Requirements
- Windows 10 or later (Windows 11 recommended)
- Go 1.25 or later (earlier versions likely work aswell)
- AMD64 or ARM64 processor
- Windows Hello configured and enabled

## Development and Building
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

4. **Clone the Repository:**
   ```bash
   cd C:\path\to\your\projects
   git clone https://github.com/julian-bruyers/winhello-go.git
   cd winhello-go
   ```

5. **Compile the Native DLL's:**
   ```bash
   go run make.go
   ```
   
   This compiles:
   - `winhello_amd64.dll` (for 64-bit systems)
   - `winhello_arm64.dll` (for ARM64 systems)
   
   The DLLs are automatically embedded into Go binaries.

6. **Build Example Application:**
   ```bash
   go run examples\main.go
   ```

## Project Structure

```
winhello-go/
├── examples/
│   └── main.go                # Example application
├── native/
│   └── WinHelloDLL.cpp        # C++ implementation using WinRT APIs
├── auth_windows.go            # Windows-specific Go bindings
├── auth_stub.go               # Stub for non-Windows systems
├── dll_amd64.go               # Go embedding logic for amd64
├── dll_arm64.go               # Go embedding logic for ARM64
├── errors.go                  # Error type definitions
├── go.mod                     # Go module definition
├── make.go                    # Build script for C++ DLL compilation
├── winhello_amd64.dll         # Pre-compiled DLL for amd64 (embedded)
├── winhello_arm64.dll         # Pre-compiled DLL for ARM64 (embedded)
├── LICENSE                    # MIT License
└── README.md                  # This file
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

Copyright (c) 2026 Julian Bruyers
