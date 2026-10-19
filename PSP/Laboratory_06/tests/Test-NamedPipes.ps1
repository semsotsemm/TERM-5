param([switch]$IncludeSmbLoopback)
$ErrorActionPreference = 'Stop'
$labRoot = Split-Path -Parent $PSScriptRoot
$binaryDir = Join-Path $labRoot 'bin'
$logDir = Join-Path $labRoot ('test-results\' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $logDir -Force | Out-Null
$script:checks = @()

function Start-TestProcess([string]$Name, [string]$Label, [string[]]$Arguments = @()) {
    $options = @{
        FilePath = Join-Path $binaryDir "$Name.exe"
        WorkingDirectory = $binaryDir
        WindowStyle = 'Hidden'
        PassThru = $true
        RedirectStandardOutput = Join-Path $logDir "$Label.out.txt"
        RedirectStandardError = Join-Path $logDir "$Label.err.txt"
    }
    if ($Arguments.Count) { $options.ArgumentList = $Arguments }
    return Start-Process @options
}

function Confirm-Process($Process, [string]$Label, [int]$ExpectedExit = 0, [int]$Replies = 0) {
    if (-not $Process.WaitForExit(20000)) {
        Stop-Process -Id $Process.Id -Force
        throw "Timed out: $Label"
    }
    $Process.Refresh()
    $stdout = Get-Content -Raw -Encoding utf8 -LiteralPath (Join-Path $logDir "$Label.out.txt")
    $stderr = Get-Content -Raw -Encoding utf8 -LiteralPath (Join-Path $logDir "$Label.err.txt")
    if ($Process.ExitCode -ne $ExpectedExit) { throw "$Label exit=$($Process.ExitCode): $stderr" }
    if ($Replies -gt 0 -and ([regex]::Matches($stdout, 'Сообщение получено!')).Count -ne $Replies) {
        throw "Wrong reply count: $Label : $stdout"
    }
    $script:checks += [pscustomobject]@{ Test = $Label; Passed = $true; ExitCode = $Process.ExitCode }
    Write-Output "PASS $Label"
}

$serverProcess = $null
try {
    $serverProcess = Start-TestProcess 'ServerNP' 'server'
    Start-Sleep -Milliseconds 500
    if ($serverProcess.HasExited) {
        throw ('Could not start own server: ' + (Get-Content -Raw -Encoding utf8 -LiteralPath (Join-Path $logDir 'server.err.txt')))
    }
    Confirm-Process (Start-TestProcess 'ClientNP' 'read-write' @('.')) 'read-write' 0 3
    Confirm-Process (Start-TestProcess 'ClientNPt' 'transact' @('.')) 'transact' 0 1
    Confirm-Process (Start-TestProcess 'ClientNPct' 'call' @('.')) 'call' 0 1
    Confirm-Process (Start-TestProcess 'PipeTests' 'boundaries-and-dacl') 'boundaries-and-dacl'

    # Второй клиент должен дождаться занятого экземпляра, а не завершиться с 231.
    $busy = Start-TestProcess 'ClientNP' 'busy-owner' @('.')
    Start-Sleep -Milliseconds 300
    $waiter = Start-TestProcess 'ClientNPt' 'busy-waiter' @('.')
    Confirm-Process $busy 'busy-owner' 0 3
    Confirm-Process $waiter 'busy-waiter' 0 1
    $busy = Start-TestProcess 'ClientNP' 'call-busy-owner' @('.')
    Start-Sleep -Milliseconds 300
    $waiter = Start-TestProcess 'ClientNPct' 'call-busy-waiter' @('.')
    Confirm-Process $busy 'call-busy-owner' 0 3
    Confirm-Process $waiter 'call-busy-waiter' 0 1

    Confirm-Process (Start-TestProcess 'ServerNP' 'duplicate-server') 'duplicate-server' 1
    if ($serverProcess.HasExited) { throw 'Original server died after duplicate start' }

    if ($IncludeSmbLoopback) {
        foreach ($name in @('ClientNP', 'ClientNPt', 'ClientNPct')) {
            $replies = if ($name -eq 'ClientNP') { 3 } else { 1 }
            $label = "smb-$name"
            Confirm-Process (Start-TestProcess $name $label @($env:COMPUTERNAME)) $label 0 $replies
        }
    }
}
finally {
    # Останавливаем только тот сервер, который запустил этот тест.
    if ($serverProcess -and -not $serverProcess.HasExited) { Stop-Process -Id $serverProcess.Id -Force }
}

foreach ($name in @('ClientNP', 'ClientNPt', 'ClientNPct')) {
    $label = "no-server-$name"
    Confirm-Process (Start-TestProcess $name $label @('.')) $label 1
}
$script:checks | ConvertTo-Json | Set-Content -Encoding utf8 -LiteralPath (Join-Path $logDir 'results.json')
Write-Output "ALL TESTS PASSED. Logs: $logDir"
