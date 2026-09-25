param([switch]$DatabaseOnly)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$docker = 'D:\Applications\Docker\resources\bin\docker.exe'
$dockerDesktop = 'D:\Applications\Docker\Docker Desktop.exe'
$sqlDeveloper = Join-Path $env:LOCALAPPDATA 'Programs\Oracle\sqldeveloper\sqldeveloper.exe'

function Test-DockerReady {
    $previousPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        & $docker info --format '{{.ServerVersion}}' 2>$null | Out-Null
        return ($LASTEXITCODE -eq 0)
    } finally {
        $ErrorActionPreference = $previousPreference
    }
}

try {
    Write-Host 'Starting Oracle lab (AAR)...'
    if (-not (Test-DockerReady)) {
        Start-Process -FilePath $dockerDesktop -WindowStyle Hidden
        $deadline = (Get-Date).AddMinutes(4)
        do {
            Start-Sleep -Seconds 3
            if ((Get-Date) -gt $deadline) {
                throw 'Docker Desktop did not start. Open Docker Desktop and check its status.'
            }
        } while (-not (Test-DockerReady))
    }

    & $docker compose -f (Join-Path $PSScriptRoot 'compose.yaml') up -d --wait --wait-timeout 600
    if ($LASTEXITCODE -ne 0) {
        throw 'Oracle failed to start. Run docker logs aar-oracle for details.'
    }

    Write-Host 'Oracle is ready: localhost:1521/FREEPDB1.'
    if (-not $DatabaseOnly) {
        if (-not (Test-Path -LiteralPath $sqlDeveloper)) {
            throw "SQL Developer not found: $sqlDeveloper"
        }
        Start-Process -FilePath $sqlDeveloper -WorkingDirectory (Split-Path $sqlDeveloper) -WindowStyle Normal
    }
} catch {
    Write-Host $_ -ForegroundColor Red
    Read-Host 'Press Enter to close'
    exit 1
}
