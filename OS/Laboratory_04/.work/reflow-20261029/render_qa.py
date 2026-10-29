from pathlib import Path
import json,re
import pypdfium2 as pdfium
import pdfplumber
from PIL import Image,ImageDraw
W=Path(__file__).resolve().parent;path=W/'final.pdf'
out=W/'pages';out.mkdir(exist_ok=True)
pd=pdfium.PdfDocument(path)
for i,page in enumerate(pd):page.render(scale=1.7).to_pil().save(out/f'page-{i+1:02}.png')
for first in range(0,len(pd),4):
 canvas=Image.new('RGB',(1500,2100),'#ccc');draw=ImageDraw.Draw(canvas)
 for j in range(first,min(first+4,len(pd))):
  im=Image.open(out/f'page-{j+1:02}.png');im.thumbnail((730,1010));x=((j-first)%2)*750;y=((j-first)//2)*1050
  canvas.paste(im,(x+10,y+27));draw.text((x+10,y+8),'PAGE '+str(j+1),fill='black')
 canvas.save(W/f'sheet-{first//4+1:02}.jpg')
inventory=[];issues=[]
with pdfplumber.open(path) as pdf:
 for n,page in enumerate(pdf.pages,1):
  text=page.extract_text() or '';lines=text.splitlines();words=[x for x in page.extract_words() if x['top']>50]
  tables=page.find_tables();tableboxes=[t.bbox for t in tables]
  for k,img in enumerate(page.images):
   before=[x for x in words if x['bottom']<img['top']-1]
   after=[x for x in words if x['top']>img['bottom']+1]
   if not before:issues.append({'page':n,'kind':'No text before image'})
   below=' '.join(x['text'] for x in after)
   if not re.search(r'Рисунок\s+\d+\.\d+',below):issues.append({'page':n,'kind':'No caption after image'})
   # Substantive analysis must follow the final caption on each page.
  if page.images:
   caps=[i for i,l in enumerate(lines) if re.match('Рисунок ',l)]
   if not caps or len(lines)-caps[-1]<=2:issues.append({'page':n,'kind':'No prose after final caption'})
  if lines and lines[-1].startswith(('Рисунок ','Таблица ','Листинг ')):issues.append({'page':n,'kind':'Page ends in caption'})
  for box in tableboxes:
   b=[w for w in words if w['bottom']<box[1]-1]
   a=[w for w in words if w['top']>box[3]+1]
   if not b:issues.append({'page':n,'kind':'No text before table','bbox':box})
   if not a:issues.append({'page':n,'kind':'No text after table','bbox':box})
  start=lines[1] if lines and lines[0].isdigit() and len(lines)>1 else (lines[0] if lines else '')
  if start.startswith(('Рисунок ','Таблица ','Листинг ')):issues.append({'page':n,'kind':'Page starts in caption'})
  inventory.append({'page':n,'start':lines[:3],'end':lines[-4:],'images':len(page.images),'tables':len(tables),'text':text})
(W/'page-audit.json').write_text(json.dumps({'pages':inventory,'issues':issues},ensure_ascii=False,indent=2),encoding='utf8')
(W/'final-text.txt').write_text('\n\n'.join(f"PAGE {p['page']}\n{p['text']}" for p in inventory),encoding='utf8')
print('PAGES',len(pd),'IMAGES',sum(x['images'] for x in inventory),'ISSUES',json.dumps(issues,ensure_ascii=False))
for p in inventory:print(p['page'],' | '.join(p['start'][1:2] if p['start'][0].isdigit() else p['start'][:1]),'IMAGES',p['images'],'TABLES',p['tables'],'END',' | '.join(p['end'][-2:]))
