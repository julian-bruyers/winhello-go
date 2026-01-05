//go:build windows && amd64

package winhello

import _ "embed"

//go:embed winhello_amd64.dll
var dllBytes []byte
