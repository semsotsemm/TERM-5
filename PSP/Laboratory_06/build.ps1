param([string]$VisualStudioPath = '', [switch]$WithTests)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
if (-not $VisualStudioPath) {
    $locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $locator) {
        $instances = & $locator -all -products '*' -format json | ConvertFrom-Json
        $VisualStudioPath = ($instances | Select-Object -First 1).installationPath
    }
}
if (-not $VisualStudioPath) { throw 'Visual Studio C++ tools not found. Pass -VisualStudioPath.' }
$toolset = Get-ChildItem -LiteralPath (Join-Path $VisualStudioPath 'VC\Tools\MSVC') -Directory |
    Sort-Object Name -Descending | Select-Object -First 1
$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
$sdk = Get-ChildItem -LiteralPath (Join-Path $sdkRoot 'Include') -Directory |
    Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'um\Windows.h') } |
    Sort-Object Name -Descending | Select-Object -First 1
if (-not $toolset -or -not $sdk) { throw 'Install the C++ toolset and Windows SDK.' }
$compilerDir = Join-Path $toolset.FullName 'bin\Hostx64\x64'
$compiler = Join-Path $compilerDir 'cl.exe'
$oldInclude = $env:INCLUDE; $oldLib = $env:LIB; $oldPath = $env:PATH
try {
    $env:PATH = "$compilerDir;$oldPath"
    $env:INCLUDE = @((Join-Path $toolset.FullName 'include'),
        (Join-Path $sdk.FullName 'ucrt'), (Join-Path $sdk.FullName 'shared'),
        (Join-Path $sdk.FullName 'um')) -join ';'
    $env:LIB = @((Join-Path $toolset.FullName 'lib\x64'),
        (Join-Path $sdkRoot "Lib\$($sdk.Name)\ucrt\x64"),
        (Join-Path $sdkRoot "Lib\$($sdk.Name)\um\x64")) -join ';'
    $outputDir = Join-Path $projectRoot 'bin'
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    foreach ($name in @('ServerNP', 'ClientNP', 'ClientNPt', 'ClientNPct')) {
        $source = Join-Path $projectRoot "ServerNP\$name\$name.cpp"
        & $compiler /nologo /std:c++20 /EHsc /W4 /WX /utf-8 /Zi /Od /MDd `
            "/Fo$outputDir\$name.obj" "/Fd$outputDir\$name.pdb" "/Fe$outputDir\$name.exe" $source
        if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
    }
    if ($WithTests) {
        & $compiler /nologo /std:c++20 /EHsc /W4 /WX /utf-8 /Zi /Od /MDd `
            "/Fo$outputDir\PipeTests.obj" "/Fd$outputDir\PipeTests.pdb" `
            "/Fe$outputDir\PipeTests.exe" (Join-Path $projectRoot 'tests\PipeTests.cpp')
        if ($LASTEXITCODE -ne 0) { throw 'Build failed: PipeTests' }
    }
    Write-Output "Build succeeded: $outputDir"
}
finally { $env:INCLUDE = $oldInclude; $env:LIB = $oldLib; $env:PATH = $oldPath }
