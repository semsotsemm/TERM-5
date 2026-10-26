$ErrorActionPreference = 'Stop'
$work = 'D:\WorkSpace\TERM-5\OS\Laboratory_05\.work'
$word = New-Object -ComObject Word.Application
$word.Visible = $false
$word.DisplayAlerts = 0
$word.AutomationSecurity = 3
try {
    foreach ($item in @(
        @('D:\WorkSpace\TERM-5\OS\ТребованияОформления.doc', 'rules'),
        @('D:\WorkSpace\TERM-5\OS\Laboratory_03\Отчет_ЛР3.docx', 'reference')
    )) {
        Write-Output "Opening $($item[1])"
        $doc = $word.Documents.Open($item[0], $false, $true, $false)
        [IO.File]::WriteAllText((Join-Path $work "$($item[1]).txt"), $doc.Content.Text)
        Write-Output "Text extracted $($item[1])"
        $doc.ExportAsFixedFormat((Join-Path $work "$($item[1]).pdf"), 17)
        Write-Output "PDF exported $($item[1])"
        $doc.Close(0)
    }
} finally {
    $word.Quit(0)
}
