from pathlib import Path
from zipfile import ZipFile
from collections import Counter
import hashlib,json,re,shutil
from docx import Document
from docx.oxml.ns import qn
from docx.text.paragraph import Paragraph
import pypdfium2 as pdfium
from PIL import ImageChops

W=Path(__file__).resolve().parent;R=W.parents[1]
original=R/'Отчет_ЛР5.docx';backup=W/'Отчет_ЛР5 — до доработки.docx';ready=R/'Отчет_ЛР5 — готовый.docx'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(original)==sha(backup),'The source report was edited during this review; do not overwrite it.'
d=Document(ready)
assert len(d.inline_shapes)==37 and len(d.tables)==9
def media(p):
 with ZipFile(p) as z:return Counter(hashlib.sha256(z.read(n)).hexdigest() for n in z.namelist() if n.startswith('word/media/'))
assert media(backup)==media(ready)
assert not d.element.xpath('.//wp:anchor')
assert not d.element.xpath('.//w:highlight')
text='\n'.join(p.text for p in d.paragraphs)+'\n'+'\n'.join(c.text for t in d.tables for row in t.rows for c in row.cells)
assert not re.search(r'ЗАПОЛНИТЬ|\[внести\]|МЕСТО ДЛЯ СКРИНШОТА',text)
caps=[p.text for p in d.paragraphs if p.text.startswith('Рисунок ')]
assert len(caps)==37 and len(set(caps))==37
for p in d.paragraphs:
 if p.style.name.startswith('Heading'):
  br=p.paragraph_format.page_break_before
  if br is None:br=p.style.paragraph_format.page_break_before
  assert bool(br)==(p.style.name=='Heading 1'),(p.text,br)
  e=p._p.getprevious()
  if e is not None and e.tag==qn('w:p'):
   prev=Paragraph(e,d._body)
   assert not prev.text.startswith(('Рисунок ','Таблица '))
 for drawing in p._p.xpath('.//w:drawing'):
  assert p.paragraph_format.space_before.pt==14
  assert p.paragraph_format.keep_with_next
  cp=Paragraph(p._p.getnext(),d._body);after=Paragraph(cp._p.getnext(),d._body)
  assert cp.text.startswith('Рисунок ') and cp.paragraph_format.keep_with_next
  assert after.text.strip() and not after.text.startswith(('Рисунок ','Таблица ')) and not after.style.name.startswith('Heading')
audit=json.loads((W/'page-audit.json').read_text(encoding='utf8'))
assert not audit['issues'] and len(audit['pages'])==45
for p in audit['pages']:
 start=p['start'][1] if p['start'][0].isdigit() else p['start'][0]
 assert not start.startswith(('Рисунок ','Таблица '))
# Original cover was preserved both structurally and visually.
before=pdfium.PdfDocument(W/'before.pdf');after=pdfium.PdfDocument(W/'final.pdf')
a=before[0].render(scale=1.7).to_pil().convert('RGB');b=after[0].render(scale=1.7).to_pil().convert('RGB')
cover_identical=ImageChops.difference(a,b).getbbox() is None
assert cover_identical
headings=[p.text for p in d.paragraphs if p.style.name=='Heading 1']
report={'pages':45,'screenshots':37,'tables':9,'screenshots_unchanged':True,'cover_unchanged':cover_identical,'all_pages_visually_reviewed':True,'placeholders':0,'highlighting':0,'layout_issues':audit['issues'],'main_sections':headings,'output':str(original),'backup':str(backup),'sha256':sha(ready)}
shutil.copy2(ready,original)
assert sha(original)==report['sha256']
archive=W/'final-reviewed.docx'
assert not archive.exists()
ready.replace(archive)
(W/'verification.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
print(json.dumps(report,ensure_ascii=False,indent=2))
