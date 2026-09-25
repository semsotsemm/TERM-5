$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
& 'D:\Applications\Docker\resources\bin\docker.exe' compose -f (Join-Path $PSScriptRoot 'compose.yaml') stop
if ($LASTEXITCODE -ne 0) {
    Write-Host 'Could not stop Oracle. Check Docker Desktop.' -ForegroundColor Red
    Read-Host 'Press Enter to close'
    exit 1
}
Write-Host 'Oracle stopped. All database files are preserved.'
