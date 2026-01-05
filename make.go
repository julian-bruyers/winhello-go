//go:build ignore

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
)

const (
	sourceFile = "native\\WinHelloDLL.cpp"
	cppStd     = "/std:c++17"
	optimize   = "/O2"
	flags      = "/EHsc"
	linkStatic = "/MT"
)

func main() {
	if runtime.GOOS != "windows" {
		fmt.Println("This library can only be compiled on Windows systems!")
		os.Exit(1)
	}

	vsPath, err := findVsDevCmd()
	if err != nil {
		fmt.Printf("ERROR: %v\n", err)
		fmt.Println("Please install Visual Studio C++ Build Tools or adjust the path in the script.")
		os.Exit(1)
	}
	fmt.Printf("Visual Studio Tools found: %s\n", vsPath)

	fmt.Println("\n--- Building x64 DLL ---")
	if err := compile(vsPath, "x64", "winhello_amd64.dll"); err != nil {
		fmt.Printf("Error building x64: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("\n--- Building ARM64 DLL ---")
	if err := compile(vsPath, "x64_arm64", "winhello_arm64.dll"); err != nil {
		fmt.Printf("Error building ARM64: %v\n", err)
		fmt.Println("Hint: Have you installed the 'ARM64 build tools' in the VS Installer?")
		os.Exit(1)
	}

	cleanUp()
	fmt.Println("\nBuild complete!")
}

func compile(vsBatPath, arch, outFile string) error {
	// Befehl zusammenbauen mit allen Flags
	innerCmd := fmt.Sprintf(`call "%s" %s && cl /nologo /LD %s %s %s %s "%s" /Fe:"%s"`,
		vsBatPath, arch, optimize, flags, cppStd, linkStatic, sourceFile, outFile)

	cmd := exec.Command("cmd")
	// SysProcAttr verhindert, dass Go die Anführungszeichen kaputt macht
	cmd.SysProcAttr = &syscall.SysProcAttr{
		CmdLine: fmt.Sprintf(`cmd /C "%s"`, innerCmd),
	}
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	return cmd.Run()
}

func findVsDevCmd() (string, error) {
	if vsInstall := os.Getenv("VSINSTALLDIR"); vsInstall != "" {
		candidate := filepath.Join(vsInstall, "VC", "Auxiliary", "Build", "vcvarsall.bat")
		if _, err := os.Stat(candidate); err == nil {
			return candidate, nil
		}
	}

	possiblePaths := []string{
		`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat`,
		`C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat`,
		`C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat`,
		`C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat`,
		`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat`,
	}

	for _, p := range possiblePaths {
		if _, err := os.Stat(p); err == nil {
			return p, nil
		}
	}

	vswhere := filepath.Join(os.Getenv("ProgramFiles(x86)"), "Microsoft Visual Studio", "Installer", "vswhere.exe")
	if _, err := os.Stat(vswhere); err == nil {
		out, err := exec.Command(vswhere, "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath").Output()
		if err == nil {
			installPath := strings.TrimSpace(string(out))
			if installPath != "" {
				candidate := filepath.Join(installPath, "VC", "Auxiliary", "Build", "vcvarsall.bat")
				if _, err := os.Stat(candidate); err == nil {
					return candidate, nil
				}
			}
		}
	}

	return "", fmt.Errorf("vcvarsall.bat not found")
}

func cleanUp() {
	exts := []string{"*.obj", "*.lib", "*.exp"}
	for _, ext := range exts {
		files, _ := filepath.Glob(ext)
		for _, f := range files {
			os.Remove(f)
		}
	}
}
