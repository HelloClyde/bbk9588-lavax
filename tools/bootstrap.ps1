[CmdletBinding()]
param([switch]$SkipToolchainSetup)

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$lock = Import-PowerShellDataFile (Join-Path $root 'deps.lock.psd1')
$sdk = Join-Path $root '.deps\sdk'

if (-not (Test-Path -LiteralPath (Join-Path $sdk '.git'))) {
    New-Item -ItemType Directory -Force -Path (Split-Path $sdk) | Out-Null
    & git clone $lock.BdaSdk.Url $sdk
    if ($LASTEXITCODE -ne 0) { throw 'BDA SDK clone failed' }
}
& git -C $sdk fetch --all --tags --prune
if ($LASTEXITCODE -ne 0) { throw 'BDA SDK fetch failed' }
& git -C $sdk checkout --detach $lock.BdaSdk.Commit
if ($LASTEXITCODE -ne 0) { throw 'BDA SDK checkout failed' }

if (-not $SkipToolchainSetup) {
    & (Join-Path $sdk 'scripts\setup_toolchain.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'MIPS toolchain setup failed' }
}
Write-Host "SDK ready: $sdk"
