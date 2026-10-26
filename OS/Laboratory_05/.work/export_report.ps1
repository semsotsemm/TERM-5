$ErrorActionPreference = 'Stop'
$root = 'D:\WorkSpace\TERM-5\OS\Laboratory_05'
$word = New-Object -ComObject Word.Application
$word.Visible = $false
$word.DisplayAlerts = 0
$word.AutomationSecurity = 3
try {
    $doc = $word.Documents.Open((Join-Path $root 'Отчет_ЛР5.docx'), $false, $false, $false)
    $doc.Content.Font.Name = 'Times New Roman'
    $doc.Fields.Update() | Out-Null
    foreach ($toc in $doc.TablesOfContents) { $toc.Update() }
    $doc.Repaginate()
    foreach ($toc in $doc.TablesOfContents) { $toc.UpdatePageNumbers() }
    $doc.Save()
    $doc.ExportAsFixedFormat((Join-Path $root '.work\report.pdf'),17)
    Write-Output "Pages=$($doc.ComputeStatistics(2))"
    $doc.Close(0)
} finally { $word.Quit(0) }
