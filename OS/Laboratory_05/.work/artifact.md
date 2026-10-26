# Report design contract

Reference: D:/WorkSpace/TERM-5/OS/Laboratory_03/Отчет_ЛР3.docx
SHA256: 908F422ACD6CD76B3E78A88A60D85EC739103D2DDDD8A8F36E1ECF17AB2F6B71
43 rendered pages, one A4 portrait section. Render: reference.pdf and reference-pages/.
Rules: ../..//ТребованияОформления.doc, SHA256 807DEC92A1A6B48A15C2917DC71BDF35E8FAD45F5CE4D3E3B011EB8FEB22499C.

Reuse reference package and its styles, body rebuilt for the new lab. Preserve styles,
theme, font table, numbering, section geometry, first-page behavior and header/footer
relationships. Remove old report body, image relationships/media and orphan appendix
listing content; they are content slots, not design. Rebuild TOC for new headings.
Title page university/faculty/department and author details are authoritative.
Replace lab number, report title, all technical body, bibliography and appendices.

Page system: A4 210 x 297 mm, left 30/right 15/top 20/bottom 20 mm. Header 10 mm;
PAGE upper right, no title-page number. Adjust right header paragraph indent -5 mm
to satisfy the explicit 10 mm rule. No decorative frame (incomplete source rule).
Normal: Times New Roman 14 pt, black, single, justified, first indent 12.5 mm.
Headings: TNR14 bold black, section page-break-before, 18 pt after; subsection
18 before/12 after, first indent 0, left aligned. Title block centered TNR14;
author block right aligned. Title uses Word Title style with no border and 14 pt.
Table widths <=165 mm, TNR14 body/12 headers, unshaded black borders as reference.
Captions TNR14, no first indent, table caption left, figure caption centered.
Figure component: reference paragraph + replaceable screenshot placeholder +
caption + explanatory paragraph, all kept on same page. Placeholder max 65 mm
high, no fake screenshot. Each main section begins with prose, not a table/image.
Numerical observations from VMs remain explicitly empty; deterministic simulation
results may be populated only from tested bundled programs, with seed/model noted.

Source structure: cover, TOC, introduction, theory, Windows, Linux, algorithms,
MQ, conclusion, sources, appendices. New report follows this hierarchy, not old
Lab3 program names. Full code is delivered in native/ and simulation/, appendix
uses short explanatory excerpts to avoid a 100-page duplicate source dump.
All source package parts inventoried by .work/package_inventory.json before build.
Untouched reference remains byte-for-byte identical; paragraph-level edits and
new fields are editable. Final Word refresh may change cached field contents only.
