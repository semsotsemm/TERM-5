from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import ast,hashlib,json,re,shlex
from docx import Document
import pdfplumber

root=Path(__file__).resolve().parents[1]
instruction=(root/'Инструкция_ЛР5.md').read_text(encoding='utf-8')
assert not [(i,c) for i,c in enumerate(instruction) if ord(c)<32 and c not in '\n\r\t']
for line in instruction.splitlines():
    if line.startswith('python3 -c '): ast.parse(shlex.split(line)[2])
blocks=re.findall(r'```bash\n(.*?)```',instruction,re.S)
(root/'.work/instruction-bash-syntax.sh').write_text('\n\n'.join(blocks),encoding='utf-8',newline='\n')
doc=Document(root/'Отчет_ЛР5.docx')
figs=re.findall(r'Рисунок (\d+\.\d+) –','\n'.join(p.text for p in doc.paragraphs))
assert len(figs)==37 and len(set(figs))==37
for figure in figs: assert figure in instruction,figure
sections=doc.sections
assert len(sections)==1
s=sections[0]
assert all(abs(a-b)<.1 for a,b in zip([s.page_width.mm,s.page_height.mm,s.left_margin.mm,s.right_margin.mm,s.top_margin.mm,s.bottom_margin.mm],[210,297,30,15,20,20]))
with pdfplumber.open(root/'.work/report.pdf') as pdf:
    outside=[]; wrongfont=[]; wrongsize=[]
    for i,p in enumerate(pdf.pages):
        for c in p.chars:
            if c['text'].isspace(): continue
            if 'TimesNewRoman' not in c['fontname']: wrongfont.append((i+1,c['text'],c['fontname']))
            if abs(c['size']-14)>.2 and abs(c['size']-12)>.2: wrongsize.append((i+1,c['text'],c['size']))
            if c['x0']<80 or c['x1']>p.width-25 or c['top']<20 or c['bottom']>p.height-50:
                outside.append((i+1,c['text'],c['x0'],c['top']))
    assert not wrongfont,wrongfont[:10]
    assert not wrongsize,wrongsize[:10]
    assert not outside,outside[:10]
for reference,expected in [
    (root.parent/'Laboratory_03'/'Отчет_ЛР3.docx','908F422ACD6CD76B3E78A88A60D85EC739103D2DDDD8A8F36E1ECF17AB2F6B71'),
    (root.parent/'ТребованияОформления.doc','807DEC92A1A6B48A15C2917DC71BDF35E8FAD45F5CE4D3E3B011EB8FEB22499C')]:
    assert hashlib.sha256(reference.read_bytes()).hexdigest().upper()==expected
files=[]
for folder in ['native','simulation']:
    for p in (root/folder).rglob('*'):
        if p.is_file() and '__pycache__' not in p.parts and p.suffix not in ('.obj','.pyc'):
            files.append(p)
files += [root/'Инструкция_ЛР5.md',root/'README_комплекта.md']
with ZipFile(root/'Lab05-kit.zip','w',ZIP_DEFLATED) as z:
    for p in files: z.write(p,p.relative_to(root).as_posix())
with ZipFile(root/'Lab05-kit.zip','r') as z:
    assert z.testzip() is None
manifest={p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(root/'.work/final-manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: fonts, font sizes, margins, figure coverage, instruction Python syntax, original hashes and archive CRC')
print('Archive files:',len(files),'size:',(root/'Lab05-kit.zip').stat().st_size)
