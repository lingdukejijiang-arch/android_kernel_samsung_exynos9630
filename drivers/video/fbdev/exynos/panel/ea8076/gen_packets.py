#!/usr/bin/env python3
# Translate the 4G EA8076 standalone driver (dpu20) DSI byte tables
# into exynos9630 unified-framework command tables.
# Output: ea8076_a51_poc_tables.h (three complete cmdtbl arrays:
#         unlock / lock / erase_all), consumed by
#         ea8076_a51x_panel_poc.h via nested SEQINFO nodes.
import os, re

here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, 'ea8076_a51_poc_old.h')).read()

def parse_table(name):
    m = re.search(r'static const u8 %s\[\] = \{(.*?)\};' % name, src, re.S)
    body = m.group(1)
    nums = [int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]+)', body)]
    # table format is a (B0 xx) / (E1 yy zz) packet sequence
    pkts, i = [], 0
    while i < len(nums):
        if nums[i] == 0xB0:
            pkts.append(nums[i:i+2]); i += 2
        elif nums[i] == 0xE1:
            pkts.append(nums[i:i+3]); i += 3
        else:
            raise ValueError('%s: unknown byte %02x at %d' % (name, nums[i], i))
    return pkts

def emit_packets(pkt_list, prefix, indent='\t'):
    out = []
    for n, p in enumerate(pkt_list):
        arr = ', '.join('0x%02X' % b for b in p)
        out.append('%sstatic u8 %s_%03d[] = { %s };' % (indent, prefix, n, arr))
    out.append('')
    for n, p in enumerate(pkt_list):
        out.append('%sstatic DEFINE_STATIC_PACKET(%s_%03d_pkt, DSI_PKT_TYPE_WR, %s_%03d, 0);'
                   % (indent, prefix, n, prefix, n))
    return '\n'.join(out)

def pktrefs(pkt_list, prefix, indent='\t'):
    return ['%s&PKTINFO(%s_%03d_pkt),' % (indent, prefix, n) for n in range(len(pkt_list))]

tables = {
    'FLASH_UNLOCK_TABLE': 'ea8076_poc_flash_unlock',
    'ERASE_TABLE_00': 'ea8076_poc_erase_00',
    'ERASE_TABLE_01': 'ea8076_poc_erase_01',
    'ERASE_TABLE_02': 'ea8076_poc_erase_02',
    'ERASE_TABLE_03': 'ea8076_poc_erase_03',
    'ERASE_TABLE_04': 'ea8076_poc_erase_04',
    'FLASH_LOCK_TABLE': 'ea8076_poc_flash_lock',
}

parsed = {}
out = []
out.append('/*')
out.append(' * ea8076_a51_poc_tables.h')
out.append(' *')
out.append(' * AUTO-GENERATED from ea8076_a51_poc.h (A51 4G dpu20 standalone driver).')
out.append(' * Same EA8076 DDI on A51 5G; command bytes are identical.')
out.append(' * Do not edit by hand; regenerate with gen_packets.py.')
out.append(' */')
out.append('')
out.append('/* erase-bank wait: dpu20 driver sleeps 1200ms after each erase table */')
out.append('static DEFINE_PANEL_MDELAY(ea8076_poc_wait_1200ms, 1200);')
out.append('')

for t, prefix in tables.items():
    pkts = parse_table(t)
    parsed[t] = (pkts, prefix)
    out.append('/* ---- %s (%d packets) ---- */' % (t, len(pkts)))
    out.append(emit_packets(pkts, prefix))
    out.append('')

def emit_cmdtbl(name, entries):
    out.append('static void *%s[] = {' % name)
    out.extend(entries)
    out.append('};')
    out.append('')

# unlock cmdtbl
unlock_pkts, unlock_pfx = parsed['FLASH_UNLOCK_TABLE']
emit_cmdtbl('ea8076_a51x_poc_unlock_cmdtbl', pktrefs(unlock_pkts, unlock_pfx))

# lock cmdtbl
lock_pkts, lock_pfx = parsed['FLASH_LOCK_TABLE']
emit_cmdtbl('ea8076_a51x_poc_lock_cmdtbl', pktrefs(lock_pkts, lock_pfx))

# erase-all cmdtbl: t00 -> 1200ms -> t01 -> ... -> t04 -> 1200ms (1:1 dpu20)
erase_entries = []
for i in range(5):
    pkts, pfx = parsed['ERASE_TABLE_%02d' % i]
    erase_entries.extend(pktrefs(pkts, pfx))
    erase_entries.append('\t&DLYINFO(ea8076_poc_wait_1200ms),')
emit_cmdtbl('ea8076_a51x_poc_erase_all_cmdtbl', erase_entries)

with open(os.path.join(here, 'ea8076_a51_poc_tables.h'), 'w') as f:
    f.write('\n'.join(out))

total = sum(len(v[0]) for v in parsed.values())
print('tables:', {t: len(v[0]) for t, v in parsed.items()})
print('total packets:', total)
print('erase_all_cmdtbl entries:', len(erase_entries))
