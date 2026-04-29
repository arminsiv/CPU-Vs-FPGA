$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$VitisHls = "D:\Softwares\Xilinx\Vitis_HLS\2024.2\bin\vitis_hls.bat"
$Vivado = "D:\Softwares\Xilinx\Vivado\2024.2\bin\vivado.bat"
$ToolPaths = @(
    "D:\Softwares\Xilinx\Vivado\2024.2\gnuwin\bin",
    "D:\Softwares\Xilinx\Vitis\2024.2\tps\win64\msys64\usr\bin",
    "D:\Softwares\Xilinx\Vitis\2024.2\tps\win64\msys64\mingw64\bin"
)

if (-not (Test-Path $VitisHls)) {
    throw "Vitis HLS 2024.2 was not found at $VitisHls"
}

if (-not (Test-Path $Vivado)) {
    throw "Vivado 2024.2 was not found at $Vivado"
}

$env:PATH = (($ToolPaths | Where-Object { Test-Path $_ }) -join ";") + ";" + $env:PATH

$WorkDrive = "X:"
$WorkRoot = "$WorkDrive\"
$CmdExe = "$env:SystemRoot\System32\cmd.exe"

& $CmdExe /c "subst $WorkDrive /d" | Out-Null
& $CmdExe /c "subst $WorkDrive `"$ScriptDir`""
if ($LASTEXITCODE -ne 0) {
    throw "Failed to create temporary $WorkDrive mapping for $ScriptDir"
}

Push-Location $WorkRoot
try {
    & $VitisHls -f build_hls.tcl
    if ($LASTEXITCODE -ne 0) {
        throw "Vitis HLS failed with exit code $LASTEXITCODE"
    }

    & $Vivado -mode batch -source build_vivado.tcl
    if ($LASTEXITCODE -ne 0) {
        throw "Vivado failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
    & $CmdExe /c "subst $WorkDrive /d" | Out-Null
}
