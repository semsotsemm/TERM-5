Attribute VB_Name = "GOST_ReportHotkeys"
Option Explicit

' Formatting requirements taken from the local report-formatting rules:
' A4; margins 30/15/20/20 mm; Times New Roman 14 pt;
' single spacing; first-line indent 12.5 mm.

Private Const STYLE_SECTION As String = "GOST Section"
Private Const STYLE_SUBSECTION As String = "GOST Subsection"
Private Const STYLE_BODY As String = "GOST Body"
Private Const STYLE_FIGURE As String = "GOST Figure Caption"

' Run once for every report. Creates styles, sets page parameters
' and adds page numbers in the upper-right header.
Public Sub GOST_SetupCurrentDocument()
    If Documents.Count = 0 Then Exit Sub

    Application.ScreenUpdating = False
    EnsureGOSTStyles ActiveDocument
    ApplyGOSTPageSetup ActiveDocument
    Application.ScreenUpdating = True

    MsgBox "The current document has been configured.", _
           vbInformation, "GOST report formatting"
End Sub

' Run once after importing this module into Normal.dotm.
Public Sub GOST_InstallHotkeys()
    CustomizationContext = NormalTemplate

    BindMacro "GOST_SetupCurrentDocument", _
              BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyG)
    BindMacro "GOST_FormatSection", _
              BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyZ)
    BindMacro "GOST_FormatSubsection", _
              BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyX)
    BindMacro "GOST_FormatBody", _
              BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyT)
    BindMacro "GOST_FormatFigureCaption", _
              BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyR)

    NormalTemplate.Save

    MsgBox "Hotkeys installed:" & vbCrLf & vbCrLf & _
           "Ctrl+Alt+G - configure current document" & vbCrLf & _
           "Ctrl+Alt+Z - section heading" & vbCrLf & _
           "Ctrl+Alt+X - subsection heading" & vbCrLf & _
           "Ctrl+Alt+T - body text" & vbCrLf & _
           "Ctrl+Alt+R - figure caption", _
           vbInformation, "GOST report formatting"
End Sub

Public Sub GOST_FormatSection()
    ApplyGOSTStyle STYLE_SECTION
End Sub

Public Sub GOST_FormatSubsection()
    ApplyGOSTStyle STYLE_SUBSECTION
End Sub

Public Sub GOST_FormatBody()
    ApplyGOSTStyle STYLE_BODY
End Sub

Public Sub GOST_FormatFigureCaption()
    ApplyGOSTStyle STYLE_FIGURE
End Sub

Public Sub GOST_RemoveHotkeys()
    CustomizationContext = NormalTemplate

    ClearKey BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyG)
    ClearKey BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyZ)
    ClearKey BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyX)
    ClearKey BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyT)
    ClearKey BuildKeyCode(wdKeyControl, wdKeyAlt, wdKeyR)

    NormalTemplate.Save
    MsgBox "Hotkeys removed.", vbInformation, "GOST report formatting"
End Sub

Private Sub ApplyGOSTStyle(ByVal styleName As String)
    Dim target As Range

    If Documents.Count = 0 Then Exit Sub

    EnsureGOSTStyles ActiveDocument
    ' Selection.Range.Paragraphs has no Range property in some Word versions.
    ' Expand the selected fragment to complete paragraphs explicitly.
    Set target = Selection.Range.Duplicate
    target.Start = target.Paragraphs.First.Range.Start
    target.End = target.Paragraphs.Last.Range.End

    ' Remove accidental direct formatting and apply the whole paragraph style.
    target.Font.Reset
    target.ParagraphFormat.Reset
    target.Style = ActiveDocument.Styles(styleName)
End Sub

Private Sub EnsureGOSTStyles(ByVal doc As Document)
    Dim normalName As String
    Dim s As Style

    normalName = doc.Styles(wdStyleNormal).NameLocal

    Set s = GetOrCreateParagraphStyle(doc, STYLE_BODY)
    With s
        .BaseStyle = normalName
        .AutomaticallyUpdate = False
        .Font.Name = "Times New Roman"
        .Font.Size = 14
        .Font.Bold = False
        .Font.Italic = False
        .Font.AllCaps = False
        .Font.Color = wdColorAutomatic
        With .ParagraphFormat
            .Alignment = wdAlignParagraphJustify
            .LeftIndent = 0
            .RightIndent = 0
            .FirstLineIndent = CentimetersToPoints(1.25)
            .SpaceBefore = 0
            .SpaceAfter = 0
            .LineSpacingRule = wdLineSpaceSingle
            .KeepWithNext = False
            .KeepTogether = False
            .WidowControl = True
            .PageBreakBefore = False
            .OutlineLevel = wdOutlineLevelBodyText
        End With
        .NextParagraphStyle = STYLE_BODY
    End With

    Set s = GetOrCreateParagraphStyle(doc, STYLE_SECTION)
    With s
        .BaseStyle = normalName
        .AutomaticallyUpdate = False
        .Font.Name = "Times New Roman"
        .Font.Size = 14
        .Font.Bold = True
        .Font.Italic = False
        .Font.AllCaps = False
        .Font.Color = wdColorAutomatic
        With .ParagraphFormat
            .Alignment = wdAlignParagraphJustify
            .LeftIndent = 0
            .RightIndent = 0
            .FirstLineIndent = 0
            .SpaceBefore = 0
            .SpaceAfter = 18
            .LineSpacingRule = wdLineSpaceSingle
            .KeepWithNext = True
            .KeepTogether = True
            .WidowControl = True
            .PageBreakBefore = True
            .OutlineLevel = wdOutlineLevel1
        End With
        .NextParagraphStyle = STYLE_BODY
    End With

    Set s = GetOrCreateParagraphStyle(doc, STYLE_SUBSECTION)
    With s
        .BaseStyle = normalName
        .AutomaticallyUpdate = False
        .Font.Name = "Times New Roman"
        .Font.Size = 14
        .Font.Bold = True
        .Font.Italic = False
        .Font.AllCaps = False
        .Font.Color = wdColorAutomatic
        With .ParagraphFormat
            .Alignment = wdAlignParagraphJustify
            .LeftIndent = 0
            .RightIndent = 0
            .FirstLineIndent = 0
            .SpaceBefore = 18
            .SpaceAfter = 12
            .LineSpacingRule = wdLineSpaceSingle
            .KeepWithNext = True
            .KeepTogether = True
            .WidowControl = True
            .PageBreakBefore = False
            .OutlineLevel = wdOutlineLevel2
        End With
        .NextParagraphStyle = STYLE_BODY
    End With

    Set s = GetOrCreateParagraphStyle(doc, STYLE_FIGURE)
    With s
        .BaseStyle = normalName
        .AutomaticallyUpdate = False
        .Font.Name = "Times New Roman"
        .Font.Size = 14
        .Font.Bold = False
        .Font.Italic = False
        .Font.AllCaps = False
        .Font.Color = wdColorAutomatic
        With .ParagraphFormat
            .Alignment = wdAlignParagraphCenter
            .LeftIndent = 0
            .RightIndent = 0
            .FirstLineIndent = 0
            .SpaceBefore = 0
            .SpaceAfter = 14
            .LineSpacingRule = wdLineSpaceSingle
            .KeepWithNext = True
            .KeepTogether = True
            .WidowControl = True
            .PageBreakBefore = False
            .OutlineLevel = wdOutlineLevelBodyText
        End With
        .NextParagraphStyle = STYLE_BODY
    End With
End Sub

Private Sub ApplyGOSTPageSetup(ByVal doc As Document)
    Dim sec As Section
    Dim hdr As HeaderFooter
    Dim i As Long

    For i = 1 To doc.Sections.Count
        Set sec = doc.Sections(i)

        With sec.PageSetup
            .PaperSize = wdPaperA4
            .Orientation = wdOrientPortrait
            .TopMargin = CentimetersToPoints(2)
            .BottomMargin = CentimetersToPoints(2)
            .LeftMargin = CentimetersToPoints(3)
            .RightMargin = CentimetersToPoints(1.5)
            .HeaderDistance = CentimetersToPoints(1)
            .FooterDistance = CentimetersToPoints(1)
            .DifferentFirstPageHeaderFooter = (i = 1)
        End With

        Set hdr = sec.Headers(wdHeaderFooterPrimary)
        hdr.Range.ParagraphFormat.Alignment = wdAlignParagraphRight
        hdr.Range.Font.Name = "Times New Roman"
        hdr.Range.Font.Size = 14

        If hdr.PageNumbers.Count = 0 Then
            hdr.PageNumbers.Add _
                PageNumberAlignment:=wdAlignPageNumberRight, _
                FirstPage:=False
        End If

        hdr.PageNumbers.RestartNumberingAtSection = False
    Next i
End Sub

Private Function GetOrCreateParagraphStyle( _
    ByVal doc As Document, _
    ByVal styleName As String) As Style

    Dim s As Style

    On Error Resume Next
    Set s = doc.Styles(styleName)
    On Error GoTo 0

    If s Is Nothing Then
        Set s = doc.Styles.Add( _
            Name:=styleName, _
            Type:=wdStyleTypeParagraph)
    End If

    Set GetOrCreateParagraphStyle = s
End Function

Private Sub BindMacro(ByVal macroName As String, ByVal keyCode As Long)
    ClearKey keyCode
    KeyBindings.Add _
        KeyCategory:=wdKeyCategoryMacro, _
        Command:=macroName, _
        KeyCode:=keyCode
End Sub

Private Sub ClearKey(ByVal keyCode As Long)
    On Error Resume Next
    FindKey(keyCode).Clear
    On Error GoTo 0
End Sub

