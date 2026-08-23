[CmdletBinding()]
param(
    [string]$Source,
    [string]$Output
)

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$build = Join-Path $root 'build'
$staging = Join-Path $build 'lavaxos-package'
$lock = Import-PowerShellDataFile (Join-Path $root 'deps.lock.psd1')
$upstream = $lock.LavaXVm

function Invoke-Git([string[]]$Arguments) {
    & git @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Git command failed ($LASTEXITCODE): git $($Arguments -join ' ')"
    }
}

function Remove-VerifiedStagingDirectory([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) { return }
    $resolved = [IO.Path]::GetFullPath($Path)
    $buildPrefix = [IO.Path]::GetFullPath($build).TrimEnd('\') + '\'
    if (-not $resolved.StartsWith($buildPrefix, [StringComparison]::OrdinalIgnoreCase) -or
        [IO.Path]::GetFileName($resolved) -ne 'lavaxos-package') {
        throw "Refusing to remove unexpected staging path: $resolved"
    }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}

if (-not $Source) {
    $checkout = Join-Path $root '.deps\lavaxos-package'
    if (-not (Test-Path -LiteralPath (Join-Path $checkout '.git'))) {
        New-Item -ItemType Directory -Force -Path (Split-Path $checkout) | Out-Null
        Invoke-Git @('clone', $upstream.Url, $checkout)
    }
    Invoke-Git @('-C', $checkout, 'fetch', '--all', '--tags', '--prune')
    Invoke-Git @('-C', $checkout, 'checkout', '--detach', $upstream.Commit)
    $Source = Join-Path $checkout 'LavaXOS'
    $licenseSource = Join-Path $checkout 'LICENSE'
} else {
    $Source = [IO.Path]::GetFullPath($Source)
    $licenseSource = Join-Path (Split-Path $Source) 'LICENSE'
}

if (-not (Test-Path -LiteralPath (Join-Path $Source 'System\Shell.sys') -PathType Leaf)) {
    throw "Invalid LavaXOS source: System\Shell.sys not found under $Source"
}
if (-not (Test-Path -LiteralPath $licenseSource -PathType Leaf)) {
    throw "Upstream license not found: $licenseSource"
}

if (-not $Output) { $Output = Join-Path $build 'LavaXOS.zip' }
$Output = [IO.Path]::GetFullPath($Output)
New-Item -ItemType Directory -Force -Path $build | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path $Output) | Out-Null
Remove-VerifiedStagingDirectory $staging
New-Item -ItemType Directory -Force -Path $staging | Out-Null

$runtimeStaging = Join-Path $staging 'LavaXOS'
Copy-Item -LiteralPath $Source -Destination $runtimeStaging -Recurse
$ndsDirectory = Join-Path $runtimeStaging '_NDS'
if (Test-Path -LiteralPath $ndsDirectory) {
    Remove-Item -LiteralPath $ndsDirectory -Recurse -Force
}
$ndsFiles = @(Get-ChildItem -LiteralPath $runtimeStaging -Recurse -File -Filter '*.nds')
if ($ndsFiles.Count -ne 0) {
    throw "NDS files remain in runtime staging: $($ndsFiles.FullName -join ', ')"
}

Copy-Item -LiteralPath $licenseSource -Destination (Join-Path $staging 'LICENSE')
@(
    'LavaXOS runtime package for BBK 9588',
    '',
    "Source: $($upstream.Url)",
    "Revision: $($upstream.Commit)",
    'License: GNU GPL v2; see LICENSE in this archive.',
    'Packaging change: LavaXOS/_NDS and all .nds files are excluded.'
) | Set-Content -LiteralPath (Join-Path $staging 'SOURCE.txt') -Encoding utf8

if (Test-Path -LiteralPath $Output) {
    Remove-Item -LiteralPath $Output -Force
}
Compress-Archive -LiteralPath @(
    $runtimeStaging,
    (Join-Path $staging 'LICENSE'),
    (Join-Path $staging 'SOURCE.txt')
) -DestinationPath $Output -CompressionLevel Optimal

Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead($Output)
try {
    $entries = @($archive.Entries | ForEach-Object { $_.FullName.Replace('\', '/') })
    if ($entries -notcontains 'LavaXOS/System/Shell.sys') {
        throw 'Packaged runtime is missing LavaXOS/System/Shell.sys'
    }
    $forbidden = @($entries | Where-Object {
        $_ -match '(^|/)_NDS(/|$)' -or $_ -match '(?i)\.nds$'
    })
    if ($forbidden.Count -ne 0) {
        throw "Packaged runtime contains NDS content: $($forbidden -join ', ')"
    }
    Write-Host "Runtime files: $($entries.Count)"
} finally {
    $archive.Dispose()
}

Remove-VerifiedStagingDirectory $staging
Write-Host "LavaXOS: $Output"
