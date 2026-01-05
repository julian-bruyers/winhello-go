package winhello

import "errors"

var (
	ErrOsNotSupported      = errors.New("winhello for Go is only available for Windows systems")
	ErrArchNotSupported    = errors.New("winhello is only supported for amd64 and ARM architectures")
	ErrNotAvailable        = errors.New("Windows Hello is not available or configured on this system")
	ErrUserCanceled        = errors.New("the user canceled the authentification")
	ErrDLLLoad             = errors.New("failed to load dll")
	ErrInternal            = errors.New("internal winhello error")
	ErrUnknownDllReturnVal = errors.New("the winhello_arch.dll returned an unknown value")
)
