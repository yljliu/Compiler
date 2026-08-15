# mini_compiler

A tiny compiler front-end for a toy imperative language, written in C++17.

I put this how a compiler works: a hand-written lexer, a recursive-descent parser, a proper
AST, and a code generator that spits out three-address code (the kind of
intermediate representation real compilers use before they get anywhere near
machine code).

It's a front-end: it takes source text all the way down to IR, which is the
natural handoff point for an interpreter or a backend down the line.

## What the language looks like

```c
x = 10;
y = 0;
while (x > 0) {
    if (x == 5) {
        y = y + 100;
    } else {
        y = y + x;
    }
    x = x - 1;
}
print(y);
```

Numbers, identifiers, `+ - * /`, comparisons (`< > <= >= == !=`), `if`/`else`,
`while`, `print`, and `// comments`.

## The pipeline

```
source text -> Lexer -> tokens -> Parser -> AST -> IRGenerator -> IR
```

1. **Lexer** — turns raw source text into a flat list of tokens
   (`NUMBER`, `ID`, `IF`, `PLUS`, `LBRACE`, ...). Handles whitespace,
   `//` comments, integers and simple decimals, and picks up multi-char
   operators like `<=` and `==` before falling back to single characters.

2. **Parser** — classic recursive descent, one function per grammar rule.
   Expressions are parsed with precedence climbing
   (`comparison → term → factor → unary → primary`), so `2 + 3 * 4` comes
   out the way you'd hope.

3. **AST** — a small hierarchy of node types (`Num`, `Var`, `BinOp`,
   `UnaryOp`, `Assign`, `Print`, `Block`, `If`, `While`, `Program`), all
   built on top of a polymorphic `Node` base class and held together with
   `shared_ptr`.

4. **IRGenerator** — walks the AST and emits three-address code, the kind
   with one operation per line and temporaries for intermediate values:

   ```
   x = 10
   y = 0
   LABEL L1
   t1 = x > 0
   IF_FALSE t1 GOTO L2
   t2 = x == 5
   IF_FALSE t2 GOTO L3
   t3 = y + 100
   y = t3
   GOTO L4
   LABEL L3
   t4 = y + x
   y = t4
   LABEL L4
   t5 = x - 1
   x = t5
   GOTO L1
   LABEL L2
   PRINT y
   ```

## Building and running

You'll need a C++17 compiler.

```bash
g++ -std=c++17 -O2 -o mini_compiler mini_compiler.cpp
./mini_compiler
```


## Errors

Bad input throws exceptions rather than crashing or silently doing the wrong
thing:

- `LexError` — an unrecognized character.
- `ParseError` — a token where the grammar didn't expect one, with the
  token type, value, and source position included.

`main()` catches both and prints a message instead of letting the exception
propagate, so you get a readable error instead of a stack trace.
