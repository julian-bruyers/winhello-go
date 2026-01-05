//go:build !windows

package winhello

func Authenticate(promptMsg string) (bool, error) {
	return false, ErrOsNotSupported
}
