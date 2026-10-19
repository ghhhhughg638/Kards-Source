#!/usr/bin/env python3
"""Generate the static, provenance-preserving KARDS node catalog from UHT headers."""
import json, re, sys
from pathlib import Path
root = Path(__file__).resolve().parents[1]

def balanced_end(s, start):
    depth = 0
    for i in range(start, len(s)):
        if s[i] == '(': depth += 1
        elif s[i] == ')':
            depth -= 1
            if depth == 0: return i
    raise ValueError('Unbalanced declaration')

def split_params(s):
    depth = 0; start = 0; result = []
    for i, c in enumerate(s):
        if c in '<(': depth += 1
        if c in '>)': depth -= 1
        if c == ',' and depth == 0:
            result.append(s[start:i].strip()); start = i+1
    if s[start:].strip(): result.append(s[start:].strip())
    return result

def category(name, source, pure):
    if source == 'BaseCardObject' and name.startswith('On'): return 'events'
    if pure: return 'queries'
    if re.search('Damage|Combat|Attack|Defense|Destroy|Kill|Repair', name, re.I): return 'combat'
    if re.search('Give|Remove|Buff|Suppress|Pin|Keyword|Ability|Effect', name, re.I): return 'effects'
    if re.search('Draw|Deck|Hand|Spawn|Card|Shuffle|Discard', name, re.I): return 'cards'
    if re.search('Kredit|Turn|Side|Round', name, re.I): return 'match'
    return 'utilities'

nodes=[]; enums={}; classes={}; properties=[]
for path in sorted(root.glob('*/Public/*.h')):
    text=path.read_text(encoding='utf-8-sig')
    for m in re.finditer(r'enum\s+class\s+(\w+)[^{]*\{([^}]+)', text):
        values=[]
        for entry in m[2].split(','):
            entry=re.sub(r'//[^\n]*','',entry).strip()
            v=re.match(r'(\w+)', entry)
            if v and '_MAX' not in v[1] and 'UMETA(Hidden)' not in entry: values.append(v[1])
        enums[m[1]]=values
    class_matches=list(re.finditer(r'class\s+(?:\w+_API\s+)?([UA]\w+)\s*:\s*public\s+(\w+)', text))
    for c in class_matches: classes[c[1]]={'parent':c[2], 'source':str(path.relative_to(root))}
    if not class_matches: continue
    source=path.stem.replace('Stub','').replace('EditorShim','')
    for m in re.finditer(r'\bUFUNCTION\s*\(',text):
        enclosing=[c for c in class_matches if c.start()<m.start()]
        if not enclosing: continue
        cls=enclosing[-1][1]
        try: end=balanced_end(text, text.index('(',m.start()))
        except ValueError:
            print(f'warning: skipping unbalanced UFUNCTION in {path.relative_to(root)}', file=sys.stderr); continue
        flags=text[m.start():end+1]
        semi=text.find(';',end)
        if semi < 0: continue
        sig=text[end+1:semi].strip()
        is_static=bool(re.search(r'\bstatic\b',sig))
        sig=re.sub(r'//[^\n]*','',sig)
        sig=re.sub(r'\b(?:static|virtual|KARDS_API|BLUEPRINTJSON_API)\b','',sig).strip()
        decl=re.match(r'([\w:<>, *&]+?)\s+(\w+)\s*\((.*)\)\s*(?:const\s*)?(?:override\s*)?$',sig,re.S)
        if not decl: continue
        ret,name,params=decl.groups()
        pure='BlueprintPure' in flags
        event=source=='BaseCardObject' and name.startswith('On')
        pins=[]
        if not pure:
            if not event: pins.append({'id':'exec-in','name':'执行','type':'exec','direction':'in'})
            pins.append({'id':'exec-out','name':'然后','type':'exec','direction':'out'})
        if not is_static and not event:
            pins.append({'id':'target','name':'Target','type':cls+'*','direction':'in'})
        for param in split_params(params):
            refinput=bool(re.search(r'UPARAM\s*\(\s*[Rr]ef\s*\)',param))
            param=re.sub(r'UPARAM\s*\([^)]*\)','',param).strip().split('=')[0].strip()
            p=re.match(r'(.+?)[ \t]*\b(\w+)$',param)
            if not p: continue
            typ,pname=p.groups(); typ=typ.strip()
            out='&' in typ and not typ.startswith('const ') and not refinput
            direction='out' if event or out else 'in'
            clean=typ.replace('const ','').replace('&','').strip()
            pins.append({'id':pname,'name':pname,'type':clean,'direction':direction})
        if ret.strip()!='void': pins.append({'id':'return','name':'Return Value','type':ret.strip(),'direction':'out'})
        nodes.append({'id':f'{cls}.{name}','name':name,'class':cls,'category':category(name,source,pure),'pure':pure,'event':event,'static':is_static,'nativeEvent':'BlueprintNativeEvent' in flags,'implementable':'BlueprintImplementableEvent' in flags,'kind':'event' if event else 'pure' if pure else 'function','source':str(path.relative_to(root)),'signature':re.sub(r'\s+',' ',sig),'pins':pins})
    for m in re.finditer(r'\bUPROPERTY\s*\(',text):
        enclosing=[c for c in class_matches if c.start()<m.start()]
        if not enclosing: continue
        cls=enclosing[-1][1]
        try: end=balanced_end(text,text.index('(',m.start()))
        except ValueError:
            print(f'warning: skipping unbalanced UPROPERTY in {path.relative_to(root)}', file=sys.stderr); continue
        flags=text[m.start():end+1]
        if not any(f in flags for f in ['BlueprintReadWrite','BlueprintReadOnly','BlueprintAssignable']): continue
        semi=text.find(';',end)
        sig=re.sub(r'//[^\n]*','',text[end+1:semi]).strip().split('=')[0].strip()
        prop=re.match(r'([\w:<>, *&]+?)\s+(\w+)$',sig,re.S)
        if not prop: continue
        typ,name=prop.groups();typ=re.sub(r'\s+',' ',typ.strip())
        properties.append({'class':cls,'name':name,'type':typ,'writable':'BlueprintReadWrite' in flags,'source':str(path.relative_to(root))})
result={'version':1,'repository':'ghhhhughg638/KardsSource','nodes':nodes,'enums':enums,'classes':classes,'properties':properties}
(root/'docs/catalog.json').write_text(json.dumps(result,ensure_ascii=False,separators=(',',':'))+'\n',encoding='utf-8')
print(f'Generated {len(nodes)} functions and {len(enums)} enums, {len(classes)} classes, {len(properties)} properties')
