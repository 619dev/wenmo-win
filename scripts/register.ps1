param(
    [ValidateSet("x64", "arm64")][string]$Architecture = $(if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "arm64" } else { "x64" }),
    [switch]$Unregister
)
$ErrorActionPreference = "Stop"
$dll = Join-Path $PSScriptRoot "..\out\build\windows-$Architecture\bin\Release\WenmoTsf.dll"
if (-not (Test-Path $dll)) { throw "找不到 $dll；请先执行 cmake --build --preset windows-$Architecture-release" }
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw "注册 TSF 需要以管理员身份运行 PowerShell。" }
$regsvr = Join-Path $env:WINDIR "System32\regsvr32.exe"
& $regsvr /s $(if ($Unregister) { "/u" }) $dll
if ($LASTEXITCODE -ne 0) { throw "regsvr32 失败，退出码 $LASTEXITCODE" }
Write-Host $(if ($Unregister) { "已注销问墨输入法。" } else { "已注册问墨输入法；请注销并重新登录后在语言设置中启用。" })
