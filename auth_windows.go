//go:build windows

package winhello

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sync"
	"syscall"
	"unsafe"
)

//go:generate go run make.go
var (
	lazyDLL    *syscall.LazyDLL
	procAuth   *syscall.LazyProc
	setupError error
	once       sync.Once
)

func setup() error {
	once.Do(func() {
		fileName := fmt.Sprintf("winhello_%d.dll", os.Getpid())
		tempDir := os.TempDir()
		dllPath := filepath.Join(tempDir, fileName)

		if err := os.WriteFile(dllPath, dllBytes, 0600); err != nil {
			setupError = fmt.Errorf("%w: Failed to write temp dll: %v", ErrDLLLoad, err)
			return
		}

		lazyDLL = syscall.NewLazyDLL(dllPath)
		procAuth = lazyDLL.NewProc("AuthenticateUser")
	})

	return setupError
}

func Authenticate(promptMsg string) (bool, error) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	if err := setup(); err != nil {
		return false, err
	}

	if err := procAuth.Find(); err != nil {
		return false, fmt.Errorf("%w: procedure not found: %v", ErrDLLLoad, err)
	}

	messagePtr, err := syscall.UTF16PtrFromString(promptMsg)
	if err != nil {
		return false, fmt.Errorf("invalid string prompMsg string: %v", err)
	}

	rawResult, _, _ := procAuth.Call(uintptr(unsafe.Pointer(messagePtr)))
	result := int32(rawResult)

	switch result {
	case 1:
		return true, nil
	case 0:
		return false, ErrUserCanceled
	case -1:
		return false, ErrNotAvailable
	case -2:
		return false, ErrInternal
	default:
		return false, ErrUnknownDllReturnVal
	}
}
