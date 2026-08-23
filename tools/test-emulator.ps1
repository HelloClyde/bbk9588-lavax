[CmdletBinding()]
param(
    [string]$EmulatorRoot = $env:BBK9588_EMULATOR_ROOT,
    [int]$Port = 8021,
    [switch]$ResetImage,
    [switch]$NoAutoLaunch,
    [switch]$NoOpenBrowser
)

$ErrorActionPreference = 'Stop'
if (-not $EmulatorRoot) {
    throw 'Pass -EmulatorRoot or set BBK9588_EMULATOR_ROOT.'
}
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$sdk = if ($env:BDA_SDK_ROOT) {
    [IO.Path]::GetFullPath($env:BDA_SDK_ROOT)
} elseif (Test-Path -LiteralPath (Join-Path $root '.deps\sdk\scripts\test_bda_in_emulator.ps1')) {
    Join-Path $root '.deps\sdk'
} else {
    Join-Path $root '.deps\sdk'
}
$bda = Join-Path $root 'build\LavaX.bda'
$helper = Join-Path $sdk 'scripts\test_bda_in_emulator.ps1'
if (-not (Test-Path -LiteralPath $bda)) {
    & (Join-Path $PSScriptRoot 'build.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
}
if (-not (Test-Path -LiteralPath $helper)) { throw "SDK helper not found: $helper" }
$parameters = @{
    Bda = $bda
    EmulatorRoot = $EmulatorRoot
    Port = $Port
    NoOpenBrowser = $NoOpenBrowser
}
if ($ResetImage) { $parameters.ResetImage = $true }
if ($NoAutoLaunch) { $parameters.NoAutoLaunch = $true }
& $helper @parameters
if ($LASTEXITCODE -ne 0) { throw 'Emulator deployment failed' }
