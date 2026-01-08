//go:build !windows

package winhello

func Available() (bool) {
	return false
}

func Authenticate(promptMsg string) (bool, error) {
	return false, ErrOsNotSupported
}
