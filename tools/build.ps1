[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$SkipToolchainSetup
)

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$build = Join-Path $root 'build'
$objectsRoot = Join-Path $build 'obj'
$sdk = if ($env:BDA_SDK_ROOT) {
    [IO.Path]::GetFullPath($env:BDA_SDK_ROOT)
} elseif (Test-Path -LiteralPath (Join-Path $root '.deps\sdk\sdk\include\bda_sdk.h')) {
    Join-Path $root '.deps\sdk'
} else {
    Join-Path $root '.deps\sdk'
}

if (-not (Test-Path -LiteralPath (Join-Path $sdk 'sdk\include\bda_sdk.h'))) {
    throw 'BDA SDK not found. Run tools\bootstrap.ps1 or set BDA_SDK_ROOT.'
}

function Find-ToolPrefix {
    if ($env:BDA_TOOLCHAIN_PREFIX) { return $env:BDA_TOOLCHAIN_PREFIX }
    $candidates = @(
        (Join-Path $sdk '.toolchain\bin\mipsel-none-elf-'),
        (Get-ChildItem (Join-Path $sdk '.toolchain') -Directory `
            -Filter 'g++-mipsel-none-elf-*' -ErrorAction SilentlyContinue |
            ForEach-Object { Join-Path $_.FullName 'bin\mipsel-none-elf-' })
    )
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path -LiteralPath ($candidate + 'gcc.exe'))) {
            return $candidate
        }
    }
    return $null
}

$prefix = Find-ToolPrefix
if (-not $prefix -and -not $SkipToolchainSetup) {
    & (Join-Path $sdk 'scripts\setup_toolchain.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'MIPS toolchain setup failed' }
    $prefix = Find-ToolPrefix
}
if (-not $prefix) { throw 'MIPS toolchain not found' }

function Resolve-Tool([string]$name) {
    foreach ($suffix in @('.exe', '')) {
        $candidate = $prefix + $name + $suffix
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }
    throw "Tool not found: $name"
}

function Invoke-Checked([string]$executable, [string[]]$arguments) {
    & $executable @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed ($LASTEXITCODE): $executable $($arguments -join ' ')"
    }
}

if ($Clean -and (Test-Path -LiteralPath $build)) {
    $resolved = [IO.Path]::GetFullPath($build)
    $prefixPath = $root.TrimEnd('\') + '\'
    if (-not $resolved.StartsWith($prefixPath, [StringComparison]::OrdinalIgnoreCase) -or
        [IO.Path]::GetFileName($resolved) -ne 'build') {
        throw "Refusing to clean unexpected path: $resolved"
    }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $objectsRoot | Out-Null

$gcc = Resolve-Tool 'gcc'
$objcopy = Resolve-Tool 'objcopy'
$objdump = Resolve-Tool 'objdump'
$python = (Get-Command python -ErrorAction Stop).Source
$sources = @(
    'src\runtime\entry.S',
    'src\runtime\startup.c',
    'src\libc\runtime.c',
    'src\libc\filesystem.c',
    'src\libc\math_extra.c',
    'src\platform\bbk9588_platform.c',
    'src\platform\hardware_bbk9588.c',
    'src\platform\lcd_bbk9588.c',
    'src\platform\vm_entry.c',
    'src\vm\lava.c',
    'src\vm\file.c',
    'src\vm\font.c',
    'src\vm\myctype.c',
    'src\vm\pinyin.c',
    'src\vm\py2gb.c'
)
$common = @(
    '-EL','-march=mips32','-msoft-float','-mno-abicalls','-G0','-fno-pic',
    '-O2','-ffreestanding','-fno-builtin','-ffunction-sections',
    '-fdata-sections','-fsigned-char','-fno-strict-aliasing',
    '-Wno-unused-function','-Wno-pointer-to-int-cast','-Wno-int-to-pointer-cast',
    '-I',(Join-Path $root 'src\libc\include'),
    '-I',(Join-Path $root 'src\platform'),
    '-I',(Join-Path $root 'src\vm'),
    '-I',(Join-Path $sdk 'sdk\include')
)

$objects = @()
foreach ($relative in $sources) {
    $source = Join-Path $root $relative
    $objectName = ($relative -replace '[\\/:]', '_') -replace '\.(c|S)$', '.o'
    $object = Join-Path $objectsRoot $objectName
    $language = if ($relative.EndsWith('.S')) {
        @('-x','assembler-with-cpp')
    } else {
        @('-std=gnu11')
    }
    Write-Host "CC $relative"
    Invoke-Checked $gcc @($common + $language + @('-c',$source,'-o',$object))
    $objects += $object
}

$elf = Join-Path $build 'LavaX-9588.elf'
$raw = Join-Path $build 'LavaX-9588.bin'
$map = Join-Path $build 'LavaX-9588.map'
$dump = Join-Path $build 'LavaX-9588.dump.txt'
$bda = Join-Path $build 'LavaX.bda'
$linker = Join-Path $root 'linker\bda.ld'
$icon = Join-Path $root 'assets\lavax-icon.png'
if (-not (Test-Path -LiteralPath $icon -PathType Leaf)) {
    throw "Icon not found: $icon"
}
$linkArgs = @(
    '-EL','-march=mips32','-msoft-float','-mno-abicalls','-G0','-fno-pic',
    '-nostdlib','-Wl,--build-id=none','-Wl,--gc-sections',
    "-Wl,-T,$linker","-Wl,-Map,$map",'-o',$elf
)
Write-Host 'LD LavaX-9588.elf'
Invoke-Checked $gcc ($linkArgs + $objects + @('-lgcc'))
Invoke-Checked $objcopy @('-O','binary',$elf,$raw)
& $objdump -d -h $elf | Out-File -LiteralPath $dump -Encoding ascii
if ($LASTEXITCODE -ne 0) { throw 'objdump failed' }
Invoke-Checked $python @(
    '-X','utf8','-s',(Join-Path $PSScriptRoot 'pack-prelinked.py'),$raw,
    '--sdk',$sdk,'--output',$bda,'--title','LavaX','--icon',$icon
)
Write-Host "ELF: $elf"
Write-Host "BDA: $bda"
