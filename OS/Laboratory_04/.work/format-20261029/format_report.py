from pathlib import Path
from io import BytesIO
from copy import deepcopy
from zipfile import ZipFile
from collections import Counter
import json,re,shutil,hashlib
from docx import Document
from docx.text.paragraph import Paragraph
from docx.shared import Pt,Mm,RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT,WD_CELL_VERTICAL_ALIGNMENT
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from PIL import Image

W=Path(__file__).resolve().parent;R=W.parents[1]
SRC=R/'Отчёт_ЛР4.docx';BACK=W/'Отчёт_ЛР4 — до оформления.docx';OUT=R/'Отчёт_ЛР4 — оформление.docx'
if not BACK.exists():shutil.copy2(SRC,BACK)
d=Document(BACK);body=list(d.element.body)
def add_before(e,text):
 el=OxmlElement('w:p');e.addprevious(el);p=Paragraph(el,d._body);p.text=text;return p
def add_after(e,text):
 el=OxmlElement('w:p');e.addnext(el);p=Paragraph(el,d._body);p.text=text;return p
def textof(e):return ''.join(e.xpath('.//w:t/text()'))
def bodyfmt(p):
 p.style='Normal';f=p.paragraph_format
 f.alignment=WD_ALIGN_PARAGRAPH.JUSTIFY;f.first_line_indent=Mm(12.5);f.left_indent=Mm(0);f.right_indent=Mm(0)
 f.space_before=Pt(0);f.space_after=Pt(0);f.line_spacing=1;f.page_break_before=False;f.keep_with_next=False;f.keep_together=True;f.widow_control=True
 for r in p.runs:
  r.font.name='Times New Roman';r.font.size=Pt(14);r.font.color.rgb=RGBColor(0,0,0)
def heading_level(p):
 if p.style.name=='Heading 1' or p.style.name=='Report unnumbered':return 1
 if p.style.name in ['Heading 2','Report appendix subsection']:return 2
 if p.style.name=='Heading 3':return 3
 return 0

# Preserve paragraphs and inline mathematics; normalize only body formatting.
for e in body[64:]:
 if e.tag!=qn('w:p'):continue
 p=Paragraph(e,d._body);lev=heading_level(p)
 if lev:
  f=p.paragraph_format;f.page_break_before=lev==1;f.keep_together=True;f.keep_with_next=True;f.widow_control=True
  f.space_before=Pt(0 if lev==1 else 18);f.space_after=Pt(18 if lev==1 else 12)
  f.line_spacing=1;f.first_line_indent=Mm(0);f.left_indent=Mm(0);f.right_indent=Mm(0);f.alignment=WD_ALIGN_PARAGRAPH.LEFT
  for r in p.runs:r.font.name='Times New Roman';r.font.size=Pt(14);r.font.bold=True;r.font.color.rgb=RGBColor(0,0,0)
 elif not e.xpath('.//w:drawing') and not p.text.strip().startswith(('Рисунок ','Таблица ','Листинг ')):
  bodyfmt(p)
 # Clear hard breaks from text paragraphs; headings control chapter pagination.
 for br in e.xpath('.//w:br[@w:type="page"]'):br.getparent().remove(br)
 # Numbered body prose can inherit bullets after pasted edits.
 if not lev:
  for num in e.xpath('./w:pPr/w:numPr'):num.getparent().remove(num)

# Materialize the existing chapter/subchapter numbering to remove a duplicate 2.8.
chapter=0;sub=0
for e in body[64:]:
 if e.tag!=qn('w:p'):continue
 p=Paragraph(e,d._body)
 if p.style.name=='Heading 1':chapter+=1;sub=0;value=f'{chapter} '+re.sub(r'^\d+(?:\.\d+)*\s+','',p.text)
 elif p.style.name=='Heading 2':sub+=1;value=f'{chapter}.{sub} '+re.sub(r'^\d+(?:\.\d+)*\s+','',p.text)
 else:continue
 p.text=value
 np=OxmlElement('w:numPr');id=OxmlElement('w:numId');id.set(qn('w:val'),'0');np.append(id)
 for old in p._p.xpath('./w:pPr/w:numPr'):old.getparent().remove(old)
 p._p.get_or_add_pPr().append(np)
 for r in p.runs:r.font.name='Times New Roman';r.font.size=Pt(14);r.font.bold=True

for name in ['Heading 1','Heading 2','Heading 3','Report unnumbered','Report appendix subsection']:
 s=d.styles[name];s.font.name='Times New Roman';s.font.size=Pt(14);s.font.bold=True;s.font.color.rgb=RGBColor(0,0,0)
 s.paragraph_format.page_break_before=name in ['Heading 1','Report unnumbered']
 s.paragraph_format.keep_with_next=True;s.paragraph_format.keep_together=True

# Separate screenshots from captions without changing a single screenshot byte.
figures=[]
for i,e in enumerate(body):
 bs=e.xpath('.//a:blip')
 if not bs:continue
 assert len(bs)==1
 blob=d.part.related_parts[bs[0].get(qn('r:embed'))].blob
 p=Paragraph(e,d._body);caption=p.text.strip()
 if caption:
  cp=add_after(e,caption)
 else:
  ce=e.getnext()
  while ce is not None and ce.tag==qn('w:p') and not textof(ce).strip():
   following=ce.getnext();ce.getparent().remove(ce);ce=following
  assert ce is not None and ce.tag==qn('w:p')
  cp=Paragraph(ce,d._body);caption=cp.text.strip();cp.text=caption
 assert caption.startswith('Рисунок '),(i,caption)
 p.clear();p.style='Normal';cp.style='Normal'
 w,h=Image.open(BytesIO(blob)).size
 width=min(165,135*w/h)
 p.add_run().add_picture(BytesIO(blob),width=Mm(width))
 for x in [p,cp]:
  f=x.paragraph_format;f.alignment=WD_ALIGN_PARAGRAPH.CENTER;f.first_line_indent=Mm(0);f.left_indent=Mm(0);f.right_indent=Mm(0)
  f.page_break_before=False;f.keep_with_next=True;f.keep_together=True;f.line_spacing=1;f.space_before=Pt(0);f.space_after=Pt(0)
  for r in x.runs:r.font.name='Times New Roman';r.font.size=Pt(14);r.font.bold=False;r.font.color.rgb=RGBColor(0,0,0)
 p.paragraph_format.space_before=Pt(14)
 cp.paragraph_format.space_after=Pt(14)
 # A dedicated lead prevents a chain joining two complete figure blocks.
 if i==126:
  add_before(e,'Состояние потоков после принудительного завершения второго дочернего потока показано на рисунке 2.8.')
 if i==234:
  add_before(e,'Сводные показатели времени и полноты результата приведены на рисунке 5.5.')
 lead=e.getprevious()
 while lead is not None and lead.tag==qn('w:p') and not textof(lead).strip():
  prev=lead.getprevious();lead.getparent().remove(lead);lead=prev
 assert lead is not None and lead.tag==qn('w:p')
 lp=Paragraph(lead,d._body);lp.paragraph_format.keep_with_next=True;lp.paragraph_format.keep_together=True;lp.paragraph_format.space_after=Pt(0)
 ae=cp._p.getnext()
 while ae is not None and ae.tag==qn('w:p') and not textof(ae).strip():
  nx=ae.getnext();ae.getparent().remove(ae);ae=nx
 assert ae is not None and ae.tag==qn('w:p')
 ap=Paragraph(ae,d._body);assert ap.text.strip() and not heading_level(ap)
 ap.paragraph_format.keep_together=True
 # Keep_with_next for the next figure's separate lead is set in its own pass.
 figures.append({'caption':caption,'sha256':hashlib.sha256(blob).hexdigest(),'width_mm':width})

def borders(t):
 pr=t._tbl.tblPr
 for old in pr.findall(qn('w:tblBorders')):pr.remove(old)
 bs=OxmlElement('w:tblBorders')
 for side in ['top','left','bottom','right','insideH','insideV']:
  b=OxmlElement('w:'+side);b.set(qn('w:val'),'single');b.set(qn('w:sz'),'4');b.set(qn('w:color'),'000000');bs.append(b)
 pr.append(bs)
 # Explicit cell borders prevent inherited no-border table styles from hiding edges.
 for row in t.rows:
  for c in row.cells:
   tcpr=c._tc.get_or_add_tcPr()
   for old in tcpr.findall(qn('w:tcBorders')):tcpr.remove(old)
def padding(t,mm):
 pr=t._tbl.tblPr
 for old in pr.findall(qn('w:tblCellMar')):pr.remove(old)
 mar=OxmlElement('w:tblCellMar')
 for side in ['top','bottom','left','right']:
  a=OxmlElement('w:'+side);a.set(qn('w:w'),str(int(Mm(mm).twips)));a.set(qn('w:type'),'dxa');mar.append(a)
 pr.append(mar)

listing_tokens=[]
data_widths={0:[36,49,40,40],1:[15,50,50,50],3:[29,56,43,37],4:[15,50,50,50],5:[29,56,43,37]}
for ti,t in enumerate(d.tables):
 if ti==2:continue # Preserve the native editable mathematical formula and numbering.
 t.autofit=False;t.alignment=WD_TABLE_ALIGNMENT.CENTER
 pr=t._tbl.tblPr
 for old in pr.findall(qn('w:tblInd')):pr.remove(old)
 tw=pr.find(qn('w:tblW'));tw.set(qn('w:type'),'dxa');tw.set(qn('w:w'),str(Mm(165).twips))
 borders(t)
 listing=len(t.rows)==1 and len(t.columns)==1
 if listing:
  assert ti>=6
  oldcode=t.cell(0,0).text;listing_tokens.append(re.sub(r'\s+','',oldcode))
  # Normalize indentation while preserving all non-whitespace code tokens.
  depth=0;lines=[]
  for line in oldcode.splitlines():
   s=line.strip()
   if not s:lines.append('');continue
   before=max(0,depth-(1 if s.startswith('}') else 0))
   lines.append('    '*before+s)
   depth=max(0,depth+s.count('{')-s.count('}'))
  code='\n'.join(lines)
  assert re.sub(r'\s+','',code)==listing_tokens[-1]
  cell=t.cell(0,0);cell.text=code;cell.width=Mm(165);t.columns[0].width=Mm(165);cell.vertical_alignment=WD_CELL_VERTICAL_ALIGNMENT.TOP
  padding(t,3)
  for p in cell.paragraphs:
   bodyfmt(p);f=p.paragraph_format;f.first_line_indent=Mm(0);f.alignment=WD_ALIGN_PARAGRAPH.LEFT
   f.keep_with_next=True;f.keep_together=True;f.space_before=Pt(0);f.space_after=Pt(0)
  rp=t.rows[0]._tr.get_or_add_trPr()
  if rp.find(qn('w:cantSplit')) is None:rp.append(OxmlElement('w:cantSplit'))
  # Label, bordered code cell, source and analysis form one page block.
  cap=Paragraph(t._tbl.getprevious(),d._body)
  assert cap.text.startswith('Листинг ')
  if ti==7:
   lp=add_before(cap._p,'Отбор потоков по идентификатору процесса и закрытие снимка показаны в листинге А.2.')
   lp.paragraph_format.keep_with_next=True;lp.paragraph_format.keep_together=True
  elif ti==9:
   lp=add_before(cap._p,'В листинге А.4 приведены структура локальных данных, создание ключа TLS и функция освобождения буфера при завершении потока.')
   lp.paragraph_format.keep_with_next=True;lp.paragraph_format.keep_together=True
  cap.paragraph_format.keep_with_next=True;cap.paragraph_format.keep_together=True;cap.paragraph_format.first_line_indent=Mm(0)
  cap.paragraph_format.space_before=Pt(14);cap.paragraph_format.space_after=Pt(6);cap.alignment=WD_ALIGN_PARAGRAPH.LEFT
  after=Paragraph(t._tbl.getnext(),d._body);after.paragraph_format.space_before=Pt(14);after.paragraph_format.keep_with_next=True;after.paragraph_format.keep_together=True
  explanation=after._p.getnext()
  if explanation is not None and explanation.tag==qn('w:p'):Paragraph(explanation,d._body).paragraph_format.keep_together=True
 else:
  padding(t,1.2)
  widths=data_widths[ti]
  for col,w in zip(t.columns,widths):col.width=Mm(w)
  for ri,row in enumerate(t.rows):
   row.height=None;rp=row._tr.get_or_add_trPr()
   if rp.find(qn('w:cantSplit')) is None:rp.append(OxmlElement('w:cantSplit'))
   for cell,w in zip(row.cells,widths):
    cell.width=Mm(w);cell.vertical_alignment=WD_CELL_VERTICAL_ALIGNMENT.CENTER
    for p in cell.paragraphs:
     p.style='Normal';f=p.paragraph_format;f.alignment=WD_ALIGN_PARAGRAPH.CENTER;f.first_line_indent=Mm(0)
     f.left_indent=Mm(0);f.right_indent=Mm(0);f.space_before=Pt(0);f.space_after=Pt(0);f.line_spacing=1
     f.keep_with_next=True;f.keep_together=True
     for r in p.runs:r.font.name='Times New Roman';r.font.size=Pt(12 if ri==0 else 14);r.font.bold=ri==0
  caption=Paragraph(t._tbl.getprevious(),d._body)
  assert caption.text.startswith('Таблица ')
  f=caption.paragraph_format;f.keep_with_next=True;f.keep_together=True;f.first_line_indent=Mm(0)
  f.space_before=Pt(14);f.space_after=Pt(6);f.alignment=WD_ALIGN_PARAGRAPH.LEFT
  if ti==1:
   add_before(caption._p,'Медианы полного времени выполнения для каждого числа исполнителей представлены в таблице 3.1.')
  lead=Paragraph(caption._p.getprevious(),d._body);lead.paragraph_format.keep_with_next=True;lead.paragraph_format.keep_together=True
  ae=t._tbl.getnext()
  while ae is not None and ae.tag==qn('w:p') and not textof(ae).strip():
   nx=ae.getnext();ae.getparent().remove(ae);ae=nx
  if ti==0:
   after=add_after(t._tbl,'После 40-й итерации оба дочерних потока приостановлены. Первый получает возможность продолжить выполнение на 70-й итерации, а второй — после окончания главного цикла.')
  elif ti==3:
   after=add_after(t._tbl,'При увеличении блока от 1000 до 5000 членов число переключений в выбранных запусках уменьшилось с 17 962 782 до 3 749 430. Это согласуется с более редкой передачей управления между волокнами.')
  elif ti==5:
   after=add_after(t._tbl,'Среди проверенных размеров блока наибольшая медианная производительность получена для 5000 членов. Для этого размера в выбранном запуске зарегистрировано 3 518 342 переключения.')
  else:after=Paragraph(ae,d._body)
  after.paragraph_format.space_before=Pt(14);after.paragraph_format.keep_together=True

# Add prose between appendix headings and the code listings that follow.
appendix_leads={284:'В листингах А.3 и А.4 показаны доступ к промежуточному буферу WinAPI TLS и однократная инициализация ключа TLS в Linux.',293:'В листинге А.5 приведён расчёт частичной суммы и возврат управления главному волокну.',298:'В листинге А.6 показаны настройка стеков рабочих контекстов и последовательное переключение к ним из главного контекста.'}
for idx,text in appendix_leads.items():
 p=add_after(body[idx],text);p.paragraph_format.keep_with_next=True;p.paragraph_format.keep_together=True
for p in list(d.paragraphs):
 if heading_level(p):
  nxt=p._p.getnext()
  if nxt is not None and nxt.tag==qn('w:p'):
   np=Paragraph(nxt,d._body)
   if heading_level(np):p.paragraph_format.space_after=Pt(12 if heading_level(p)==1 else 6);np.paragraph_format.space_before=Pt(0)

# Apply body defaults to newly inserted explanatory paragraphs.
for p in d.paragraphs:
 if p._p in body:continue
 if p.text.startswith(('Рисунок ','Таблица ','Листинг ')):continue
 keep=p.paragraph_format.keep_with_next;sp=p.paragraph_format.space_before
 bodyfmt(p);p.paragraph_format.keep_with_next=keep;p.paragraph_format.space_before=sp

# Remove empty body layout paragraphs without touching the title or contents.
intro=next(p._p for p in d.paragraphs if p.text=='Введение' and p.style.name=='Report unnumbered')
seen=False
for e in list(d.element.body):
 if e is intro:seen=True
 if not seen or e.tag!=qn('w:p'):continue
 p=Paragraph(e,d._body)
 if not p.text.strip() and not e.xpath('.//w:drawing|.//m:oMath|.//w:fldChar|.//w:instrText'):e.getparent().remove(e)
for s in d.sections:
 s.page_width=Mm(210);s.page_height=Mm(297);s.left_margin=Mm(30);s.right_margin=Mm(15);s.top_margin=Mm(20);s.bottom_margin=Mm(20)
assert len(d.inline_shapes)==25 and len(d.tables)==12
assert len(listing_tokens)==6
d.save(OUT)
with ZipFile(BACK) as z:a=Counter(hashlib.sha256(z.read(n)).hexdigest() for n in z.namelist() if n.startswith('word/media/'))
with ZipFile(OUT) as z:b=Counter(hashlib.sha256(z.read(n)).hexdigest() for n in z.namelist() if n.startswith('word/media/'))
assert a==b
(W/'changes.json').write_text(json.dumps({'source':str(SRC),'output':str(OUT),'figures':figures,'code_tokens':listing_tokens,'screenshots_unchanged':True},ensure_ascii=False,indent=2),encoding='utf8')
print('Formatted 25 figures, 5 data tables, 6 listing tables; preserved native equation.')
print(OUT)
