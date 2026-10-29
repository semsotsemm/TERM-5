param([string]$Source,[string]$Output)
$ErrorActionPreference='Stop'
$wordApp=New-Object -ComObject Word.Application
$wordApp.Visible=$false
$wordApp.DisplayAlerts=0
$wordApp.AutomationSecurity=3
try {
 $doc=$wordApp.Documents.Open($Source,$false,$true,$false)
 $doc.Repaginate()
 $records=@()
 foreach($table in $doc.Tables){
  $first=$doc.Range($table.Range.Start,$table.Range.Start)
  $last=$doc.Range($table.Range.End-1,$table.Range.End-1)
  $records += [pscustomobject]@{Start=$table.Range.Start;End=$table.Range.End;FirstPage=$first.Information(3);LastPage=$last.Information(3);Rows=$table.Rows.Count;Columns=$table.Columns.Count}
 }
 $records|ConvertTo-Json -Depth 4|Set-Content -LiteralPath $Output -Encoding UTF8
 if(@($records|Where-Object {$_.FirstPage -ne $_.LastPage}).Count){throw 'A table is split across pages'}
 Write-Output ('Verified tables='+$records.Count)
 $doc.Close(0)
}finally{$wordApp.Quit(0)}
