#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""读取 Myapp 的 Benchmark.txt，绘制性能分析柱状图 (SVG)，零依赖、无需 matplotlib。
用法:
    python perf_visualize.py                 # 读当前目录 Benchmark.txt -> perf_chart.svg
    python perf_visualize.py Benchmark.txt out.svg
"""
import sys
import os


def parse(path):
    d = {}
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if ':' in line:
                k, v = line.split(':', 1)
                try:
                    d[k.strip()] = float(v.strip())
                except ValueError:
                    pass
    return d


def build_svg(d):
    build = d.get('build_ms', 0.0)
    canon = d.get('canonical_ms', 0.0)
    strt = d.get('tostr_ms', 0.0)
    total = d.get('total_time_ms', build + canon + strt)
    got = int(d.get('total_problems', 0))

    W, H = 680, 380
    items = [
        ("buildTree\n构造表达式树", build, "#4e79a7"),
        ("canonical\n规范化去重", canon, "#59a14f"),
        ("toStr\n格式化输出", strt, "#e15759"),
    ]
    maxv = max([build, canon, strt, 1.0])
    baseY = 300
    maxH = 220
    bw = 120
    xs = [70, 270, 470]

    parts = []
    parts.append('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %d %d" '
                 'font-family="Segoe UI, Microsoft YaHei, Arial, sans-serif">' % (W, H))
    parts.append('<rect width="%d" height="%d" fill="#ffffff"/>' % (W, H))
    title = "Myapp 性能分析（共 %d 题，总计 %.1f ms）" % (got, total)
    parts.append('<text x="340" y="34" text-anchor="middle" font-size="20" '
                 'font-weight="bold" fill="#222">%s</text>' % title)
    parts.append('<line x1="50" y1="%d" x2="630" y2="%d" stroke="#888" stroke-width="1"/>' % (baseY, baseY))

    for (label, val, color), x in zip(items, xs):
        h = int(val / maxv * maxH) if maxv > 0 else 0
        y = baseY - h
        parts.append('<rect x="%d" y="%d" width="%d" height="%d" fill="%s"/>' % (x, y, bw, h, color))
        pct = (val / total * 100) if total > 0 else 0
        parts.append('<text x="%d" y="%d" text-anchor="middle" font-size="14" fill="#222">%.2f ms</text>'
                     % (x + bw // 2, y - 8, val))
        parts.append('<text x="%d" y="%d" text-anchor="middle" font-size="13" fill="#666">%.1f%%</text>'
                     % (x + bw // 2, y - 24, pct))
        ll = label.split('\n')
        parts.append('<text x="%d" y="%d" text-anchor="middle" font-size="13" fill="#222">%s</text>'
                     % (x + bw // 2, baseY + 22, ll[0]))
        if len(ll) > 1:
            parts.append('<text x="%d" y="%d" text-anchor="middle" font-size="13" fill="#222">%s</text>'
                         % (x + bw // 2, baseY + 40, ll[1]))

    hot = max(items, key=lambda it: it[1])
    hot_label = hot[0].split('\n')[0]
    hot_pct = (hot[1] / total * 100) if total > 0 else 0
    parts.append('<text x="340" y="362" text-anchor="middle" font-size="14" fill="#b00020">'
                 '消耗最大的函数阶段: %s（%.2f ms, %.1f%%）</text>' % (hot_label, hot[1], hot_pct))
    parts.append('</svg>')
    return ''.join(parts)


def main():
    bp = sys.argv[1] if len(sys.argv) > 1 else 'Benchmark.txt'
    out = sys.argv[2] if len(sys.argv) > 2 else 'perf_chart.svg'
    if not os.path.exists(bp):
        print("找不到 %s。" % bp)
        print("请先运行: Myapp.exe -b -r 100 -n 200000  生成它，再执行本脚本。")
        sys.exit(1)
    d = parse(bp)
    svg = build_svg(d)
    with open(out, 'w', encoding='utf-8') as f:
        f.write(svg)
    print("已生成性能分析图: %s" % out)


if __name__ == '__main__':
    main()
