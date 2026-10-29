from pathlib import Path
import json,re
from PIL import Image,ImageDraw
import pypdfium2 as pdfium
import pdfplumber
W=Path(__file__).resolve().parent
PDF=W/'final.pdf'
out=W/'pages';out.mkdir(exist_ok=True)
pdoc=pdfium.PdfDocument(PDF)
for i,p in enumerate(pdoc):p.render(scale=1.7).to_pil().save(out/f'page-{i+1:02}.png')
for first in range(0,len(pdoc),4):
 canvas=Image.new('RGB',(1500,2100),'#ccc');draw=ImageDraw.Draw(canvas)
 for j in range(first,min(first+4,len(pdoc))):
  im=Image.open(out/f'page-{j+1:02}.png');im.thumbnail((730,1010));x=((j-first)%2)*750;y=((j-first)//2)*1050
  canvas.paste(im,(x+10,y+27));draw.text((x+10,y+8),f'PAGE {j+1}',fill='black')
 canvas.save(W/f'pages-{first//4+1:02}.jpg')
info=[];issues=[]
with pdfplumber.open(PDF) as pdf:
 for no,p in enumerate(pdf.pages,1):
  text=p.extract_text() or '';words=p.extract_words();body=[x for x in words if x['top']>50]
  lines=text.splitlines()
  figs=re.findall(r'Рисунок\s+(\d+\.\d+)',text)
  row={'page':no,'start':lines[:3],'end':lines[-4:],'images':len(p.images),'figures':figs,'text':text}
  for im in p.images:
   before=[x for x in body if x['bottom']<im['top']-1]
   after=[x for x in body if x['top']>im['bottom']+1]
   if not before:issues.append({'page':no,'error':'No body text before image'})
   # Caption plus substantive explanation must follow the image on the same page.
   at=' '.join(x['text'] for x in after)
   if not after or not re.search(r'Рисунок\s+\d+\.\d+\s+[–−-]',at):issues.append({'page':no,'error':'No same-page caption below image'})
   if after:
    maxcaption=next((i for i,l in enumerate(lines) if re.match(r'Рисунок\s+\d+\.\d+',l)),None)
    if maxcaption is not None and len(lines)-maxcaption<=2:issues.append({'page':no,'error':'No substantive text after caption'})
  if lines and re.match(r'Рисунок\s+\d+\.\d+',lines[-1]):issues.append({'page':no,'error':'Page ends in caption'})
  info.append(row)
(W/'page-audit.json').write_text(json.dumps({'pages':info,'issues':issues},ensure_ascii=False,indent=2),encoding='utf8')
(W/'final-text.txt').write_text('\n\n'.join(f"PAGE {x['page']}\n{x['text']}" for x in info),encoding='utf8')
print('PAGES',len(info),'IMAGES',sum(x['images'] for x in info),'ISSUES',json.dumps(issues,ensure_ascii=False))
for x in info:print(x['page'],' | '.join(x['start'][:2]),'IMAGES',x['images'],'END',' | '.join(x['end'][-2:]))
