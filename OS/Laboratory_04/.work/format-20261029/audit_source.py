from pathlib import Path
from docx import Document
from PIL import Image,ImageDraw
from io import BytesIO
import json,re
from docx.oxml.ns import qn
R=Path(__file__).resolve().parents[2];W=Path(__file__).resolve().parent
paths=[p for p in R.glob('*.docx') if 'Отч' in p.name and not p.name.startswith('~')]
assert len(paths)==1,paths
src=paths[0];d=Document(src);items=[];images=[];tables=[]
for i,e in enumerate(d.element.body):
 t=''.join(e.xpath('.//w:t/text()'));im=[]
 if e.tag==qn('w:tbl'):
  table=[[''.join(c.xpath('.//w:t/text()')) for c in row.xpath('./w:tc')] for row in e.xpath('./w:tr')]
  tables.append({'body':i,'cells':table});t='TABLE '+json.dumps(table,ensure_ascii=False)
 for b in e.xpath('.//a:blip'):
  rid=b.get(qn('r:embed'))
  if not rid:continue
  part=d.part.related_parts[rid];dest=W/(f'img{len(images)+1:02d}'+Path(part.partname).suffix);dest.write_bytes(part.blob)
  z={'n':len(images)+1,'body':i,'path':str(dest),'size':Image.open(dest).size};images.append(z);im.append(z['n'])
 fmt=[]
 for r in e.xpath('.//w:r'):
  txt=''.join(r.xpath('.//w:t/text()'))
  if txt:fmt.append({'text':txt[:90],'font':r.xpath('./w:rPr/w:rFonts/@w:ascii'),'sz':r.xpath('./w:rPr/w:sz/@w:val')})
 items.append({'body':i,'text':t,'style':e.xpath('./w:pPr/w:pStyle/@w:val'),'ppr':str(e.xpath('./w:pPr')[0].xml) if e.xpath('./w:pPr') else '', 'images':im,'runs':fmt})
(W/'inventory.json').write_text(json.dumps(items,ensure_ascii=False,indent=2),encoding='utf8')
(W/'images.json').write_text(json.dumps(images,ensure_ascii=False,indent=2),encoding='utf8')
(W/'text.txt').write_text('\n'.join(f"[{x['body']}] {'IMAGE'+str(x['images']) if x['images'] else ''} {' '.join(x['style'])} {x['text']}" for x in items),encoding='utf8')
(W/'source.json').write_text(json.dumps({'path':str(src),'tables':tables,'styles':[(s.style_id,s.name) for s in d.styles],'sections':[{'width':s.page_width.mm,'height':s.page_height.mm,'left':s.left_margin.mm,'right':s.right_margin.mm,'top':s.top_margin.mm,'bottom':s.bottom_margin.mm} for s in d.sections]},ensure_ascii=False,indent=2),encoding='utf8')
print('SOURCE',repr(src.name),'BODY',len(items),'IMAGES',len(images),'TABLES',len(tables),'PARAGRAPHS',len(d.paragraphs))
