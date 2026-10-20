#!/usr/bin/env python3
"""Turn the KARDS textNotifications DataTable export into docs/reasons.json.

The `Reason` output of CanPlayFromHand / CanBeTargetted / CanOtherCardBeTargetted is a row key of
this table (for example "enemy_unit"; Chinese texts live in tools/data/reasons-zh.json); `reasonParam1/2` fill the {0} placeholders of its text.
"""
import json, sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
source = Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'tools' / 'data' / 'textNotifications.json'
table = json.loads(source.read_text(encoding='utf-8-sig'))[0]
zh = json.loads((root / 'tools' / 'data' / 'reasons-zh.json').read_text(encoding='utf-8'))
reasons = []
seen = set()
for row in table['Rows'].values():
    for entries in row.values():
        for entry in entries:
            key = entry['Key']
            if key in seen: continue
            seen.add(key)
            text = (entry['Value'].get('SourceString') or entry['Value'].get('LocalizedString') or '').strip()
            reasons.append({'key': key, 'text': text, 'zh': zh.get(key, ''), 'params': text.count('{0}') + text.count('{1}')})
out = root / 'docs' / 'reasons.json'
lines = ',\n'.join('  ' + json.dumps(r, ensure_ascii=False) for r in reasons)
out.write_text('{\n "source": "textNotifications",\n "reasons": [\n' + lines + '\n ]\n}\n', encoding='utf-8')
print(f'Generated {len(reasons)} reasons')
