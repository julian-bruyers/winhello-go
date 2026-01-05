//go:build windows && arm64

package winhello

import _ "embed"

//go:embed winhello_arm64.dll
var dllBytes []byte
