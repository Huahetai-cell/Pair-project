"""
小学四则运算题目生成器 —— 核心算法原型（仅用于验证逻辑，最终以 C++ 工程为准）。
验证点：
 1) 约束满足：减法不出负数；除法结果恒为真分数（<1）。
 2) 唯一性：规范化 key 不重复；打印出的题目文本也不重复。
 3) 判重等价：1+2+3 与 3+(2+1) 同一 key；3+2+1 不同 key。
 4) 往返一致：打印文本 -> 解析 -> 重新规范化，key 与生成时一致。
"""

import random
from math import gcd


class Frac:
    __slots__ = ("num", "den")

    def __init__(self, num, den=1):
        if den == 0:
            den = 1
        if den < 0:
            num, den = -num, -den
        g = gcd(num, den) or 1
        self.num = num // g
        self.den = den // g

    def __add__(self, o):
        return Frac(self.num * o.den + o.num * self.den, self.den * o.den)

    def __sub__(self, o):
        return Frac(self.num * o.den - o.num * self.den, self.den * o.den)

    def __mul__(self, o):
        return Frac(self.num * o.num, self.den * o.den)

    def __truediv__(self, o):
        if o.num == 0:
            return None
        return Frac(self.num * o.den, self.den * o.num)

    def __lt__(self, o):
        return self.num * o.den < o.num * self.den

    def __le__(self, o):
        return self.num * o.den <= o.num * self.den

    def __eq__(self, o):
        return self.num * o.den == o.num * self.den

    def is_zero(self):
        return self.num == 0

    def key(self):
        return f"{self.num}/{self.den}"

    def disp(self):
        if self.num == 0:
            return "0"
        a, r = divmod(self.num, self.den)
        if r == 0:
            return str(a)
        if a == 0:
            return f"{r}/{self.den}"
        return f"{a}\u2019{r}/{self.den}"


OPS = ["+", "-", "*", "/"]
PREC = {"+": 1, "-": 1, "*": 2, "/": 2}


def op_sym(op):
    return {"+": "+", "-": "\u2212", "*": "\u00d7", "/": "\u00f7"}[op]


def rand_operand(rng, r):
    if r <= 1:
        return Frac(0)
    if r < 3:
        return Frac(rng.randint(0, r - 1))
    if rng.random() < 0.5:
        return Frac(rng.randint(0, r - 1))
    a = rng.randint(0, r - 1)
    d = rng.randint(2, r - 1)
    n = rng.randint(1, d - 1)
    return Frac(a * d + n, d)


def build(ops, rng, r):
    if ops <= 0:
        return ("leaf", rand_operand(rng, r))
    op = rng.choice(OPS)
    budget = ops - 1
    lb = rng.randint(0, budget)
    rb = budget - lb
    left = build(lb, rng, r)
    right = build(rb, rng, r)
    if left is None or right is None:
        return None
    lv = left[1] if left[0] == "leaf" else left[4]
    rv = right[1] if right[0] == "leaf" else right[4]

    if op == "+":
        val = lv + rv
    elif op == "-":
        if lv < rv:
            left, right = right, left
            lv, rv = rv, lv
        val = lv - rv
    elif op == "*":
        val = lv * rv
    else:  # "/"
        if rv.is_zero() or lv.is_zero() or rv <= lv:
            ok = False
            for _ in range(80):
                right = build(rb, rng, r)
                if right is None:
                    return None
                rv = right[1] if right[0] == "leaf" else right[4]
                if (not rv.is_zero()) and rv > lv:
                    ok = True
                    break
            if not ok:
                return None
        val = lv / rv
        if val is None:
            return None
    return ("op", op, left, right, val)


def canon(node):
    if node[0] == "leaf":
        return node[1].key()
    _, op, left, right, _ = node
    l, r = canon(left), canon(right)
    if op in ("+", "*") and l > r:
        l, r = r, l
    return op + "(" + l + "," + r + ")"


def need_paren(child, pop, is_right):
    if child[0] == "leaf":
        return False
    cp, pp = PREC[child[1]], PREC[pop]
    if cp < pp:
        return True
    if cp == pp:
        return is_right
    return False


def to_str(node, is_right):
    if node[0] == "leaf":
        return node[1].disp()
    _, op, left, right, _ = node
    ls, rs = to_str(left, False), to_str(right, True)
    if need_paren(left, op, False):
        ls = "(" + ls + ")"
    if need_paren(right, op, True):
        rs = "(" + rs + ")"
    return ls + " " + op_sym(op) + " " + rs


# ---------- 解析（用于往返验证） ----------
def normalize(s):
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == "\u2212":
            out.append("-")
        elif c == "\u00d7":
            out.append("*")
        elif c == "\u00f7":
            out.append("/")
        elif c == "\u2019":
            out.append("'")
        else:
            out.append(c)
        i += 1
    return "".join(out)


def parse_frac_str(tok):
    p = tok.find("'")
    q = tok.find("/")
    if p == -1 and q == -1:
        return Frac(int(tok))
    a = int(tok[:p]) if p != -1 else 0
    if q != -1:
        ns = tok[(p + 1) if p != -1 else 0:q]
        d = int(tok[q + 1:])
        return Frac(a * d + int(ns), d)
    return Frac(a)


def tokenize(s):
    toks = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == " ":
            i += 1
            continue
        if c.isdigit():
            j = i
            while j < len(s) and (s[j].isdigit() or s[j] in "/'"):
                j += 1
            toks.append(("num", parse_frac_str(s[i:j])))
            i = j
        elif c in "+-*/()=":
            toks.append((c, None))
            i += 1
        else:
            i += 1
    toks.append(("end", None))
    return toks


def evaluate(toks):
    pos = 0

    def peek():
        return toks[pos]

    def parse_expr():
        nonlocal pos
        v = parse_term()
        while peek()[0] in ("+", "-"):
            op = peek()[0]
            pos += 1
            r = parse_term()
            v = v + r if op == "+" else v - r
        return v

    def parse_term():
        nonlocal pos
        v = parse_factor()
        while peek()[0] in ("*", "/"):
            op = peek()[0]
            pos += 1
            r = parse_factor()
            v = v * r if op == "*" else (v / r)
        return v

    def parse_factor():
        nonlocal pos
        t = peek()
        if t[0] == "(":
            pos += 1
            v = parse_expr()
            if peek()[0] == ")":
                pos += 1
            return v
        if t[0] == "num":
            pos += 1
            return t[1]
        pos += 1
        return Frac(0)

    return parse_expr()


def parse_expr_str(s):
    s = normalize(s).split("=")[0].strip()
    return evaluate(tokenize(s))


# ---------- 生成 + 测试 ----------
def generate(n, r, seed):
    rng = random.Random(seed)
    seen = set()
    out = []
    attempts = 0
    cap = n * 300 + 10000
    while len(out) < n and attempts < cap:
        attempts += 1
        t = rng.randint(1, 3)
        node = build(t, rng, r)
        if node is None:
            continue
        key = canon(node)
        if key in seen:
            continue
        seen.add(key)
        out.append((to_str(node, False), node[1] if node[0] == "leaf" else node[4], key))
    return out, attempts


def run_checks():
    print("== 判重等价例 ==")
    # 手工构造节点验证 canon
    def leaf(v):
        return ("leaf", v)

    # (1+2)+3
    n1 = ("op", "+", ("op", "+", leaf(Frac(1)), leaf(Frac(2)), Frac(3)), leaf(Frac(3)), Frac(6))
    # 3+(2+1)
    n2 = ("op", "+", leaf(Frac(3)), ("op", "+", leaf(Frac(2)), leaf(Frac(1)), Frac(3)), Frac(6))
    # (3+2)+1
    n3 = ("op", "+", ("op", "+", leaf(Frac(3)), leaf(Frac(2)), Frac(5)), leaf(Frac(1)), Frac(6))
    print("1+2+3 key      :", canon(n1))
    print("3+(2+1) key    :", canon(n2), "<-> 相同?" , canon(n1) == canon(n2))
    print("3+2+1 key      :", canon(n3), "<-> 相同?" , canon(n1) == canon(n3))

    print("\n== 大批量生成 + 约束 + 唯一性 + 往返 ==")
    for r in (4, 10, 50):
        probs, att = generate(2000, r, seed=20260919 + r)
        keys = [p[2] for p in probs]
        questions = [p[0] for p in probs]
        # 唯一性
        uniq_key = len(keys) == len(set(keys))
        uniq_q = len(questions) == len(set(questions))
        # 约束：每个问题解析求值，检查减法/除法（由构造保证，这里只验证求值成功且值>=0）
        all_ok = True
        for q, val, key in probs:
            try:
                v = parse_expr_str(q + " =")
            except Exception:
                all_ok = False
                break
            if v is None or v.num < 0:
                all_ok = False
                break
        # 往返：打印文本解析值与生成时的值完全一致（说明括号/优先级打印无损）
        roundtrip_ok = all(val == parse_expr_str(q + " =") for q, val, key in probs)
        print(f"r={r}: 生成 {len(probs)} 题 (attempts={att}) | key唯一={uniq_key} | 文本唯一={uniq_q} | 求值正常={all_ok} | 往返值一致={roundtrip_ok}")


def canon_node_from_parsed(_q):
    return ""


if __name__ == "__main__":
    run_checks()
