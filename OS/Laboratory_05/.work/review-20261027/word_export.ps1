param([string]$Source,[string]$Pdf,[switch]$Update)
$ErrorActionPreference='Stop'
$w=New-Object -ComObject Word.Application
$w.Visible=$false
$w.DisplayAlerts=0
$w.AutomationSecurity=3
try {
 $d=$w.Documents.Open($Source,$false,(-not $Update.IsPresent),$false)
 if($Update){$d.Fields.Update()|Out-Null;foreach($t in $d.TablesOfContents){$t.Update()};$d.Repaginate();foreach($t in $d.TablesOfContents){$t.UpdatePageNumbers()};$d.Save()}
 $d.Repaginate()
 $d.ExportAsFixedFormat($Pdf,17)
 $items=@()
 foreach($p in $d.Paragraphs){
  $r=$p.Range.Duplicate
  $items+= [pscustomobject]@{Start=$r.Start;End=$r.End;Page=$r.Information(3);Text=$r.Text;Images=$r.InlineShapes.Count;Style=$r.Style.NameLocal}
 }
 $items|ConvertTo-Json -Depth 4|Set-Content -LiteralPath ($Pdf+'.paragraphs.json') -Encoding UTF8
 Write-Output ('Pages='+$d.ComputeStatistics(2))
 $d.Close(0)
}finally{$w.Quit(0)}
