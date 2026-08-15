//Lexer.cpp

// C++ port of the Python lexer front-end  for a small imperative toy language with variables, arithmetic, comparisons, if/else, while, and print.

// Pipeline (this file implements the Lexer stage only)
//     source text -> Lexer -> tokens

// Language grammar (EBNF-ish):
//
//     program    := statement*
//     block      := '{' statement* '}'
//     statement  := assign_stmt
//                 | if_stmt
//                 | while_stmt
//                 | print_stmt
//     assign_stmt:= ID '=' expr ';'
//     if_stmt    := 'if' '(' expr ')' block ('else' block)?
//     while_stmt := 'while' '(' expr ')' block
//     print_stmt := 'print' '(' expr ')' ';'
//
//     expr       := comparison
//     comparison := term (('<'|'>'|'<='|'>='|'=='|'!=') term)*
//     term       := factor (('+'|'-') factor)*
//     factor     := unary (('*'|'/') unary)*
//     unary      := '-' unary | primary
//     primary    := NUMBER | ID | '(' expr ')'
//
// Build:
//     g++ -std=c++17 -O2 -o mini_compiler mini_compiler.cpp
//
 
#include <array>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
 
// ---------------------------------------------------------------------------
// 1. LEXER
// ---------------------------------------------------------------------------
 
namespace mini_compiler {
 
// Reserved words that get their own token type instead of ID.
static const std::unordered_set<std::string> KEYWORDS = {
    "if", "else", "while", "print"
};
 
// Multi-character operators must be checked before single-character ones,
// so this table is ordered the same way as the Python SYMBOLS list.
struct SymbolEntry {
    const char* text;
    const char* type;
};
 
static const std::array<SymbolEntry, 15> SYMBOLS = {{
    {"<=", "LE"}, {">=", "GE"}, {"==", "EQ"}, {"!=", "NE"},
    {"+", "PLUS"}, {"-", "MINUS"}, {"*", "STAR"}, {"/", "SLASH"},
    {"=", "ASSIGN"}, {"<", "LT"}, {">", "GT"},
    {"(", "LPAREN"}, {")", "RPAREN"},
    {"{", "LBRACE"}, {"}", "RBRACE"},
}};
// SEMI is handled separately below since it has no multi-char collision risk,
// but for fidelity with the original table we still include it here:
static const SymbolEntry SEMI_ENTRY = {";", "SEMI"};
 
struct Token {
    std::string type;
    std::string value;
    std::size_t pos;
 
    std::string repr() const {
        std::ostringstream oss;
        oss << "Token(" << type << ", \"" << value << "\")";
        return oss.str();
    }
};
 
class LexError : public std::runtime_error {
public:
    explicit LexError(const std::string& msg) : std::runtime_error(msg) {}
};
 
class Lexer {
public:
    explicit Lexer(std::string text)
        : text_(std::move(text)), pos_(0), n_(text_.size()) {}
 
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
 
        while (pos_ < n_) {
            char ch = text_[pos_];
 
            // whitespace
            if (std::isspace(static_cast<unsigned char>(ch))) {
                ++pos_;
                continue;
            }
 
            // comments: // to end of line
            if (peekMatches("//")) {
                while (pos_ < n_ && text_[pos_] != '\n') {
                    ++pos_;
                }
                continue;
            }
 
            // numbers (integers and simple decimals)
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                std::size_t start = pos_;
                while (pos_ < n_ && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                    ++pos_;
                }
                if (pos_ < n_ && text_[pos_] == '.' &&
                    pos_ + 1 < n_ && std::isdigit(static_cast<unsigned char>(text_[pos_ + 1]))) {
                    ++pos_;
                    while (pos_ < n_ && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                        ++pos_;
                    }
                }
                tokens.push_back(Token{"NUMBER", text_.substr(start, pos_ - start), start});
                continue;
            }
 
            // identifiers / keywords
            if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
                std::size_t start = pos_;
                while (pos_ < n_ &&
                       (std::isalnum(static_cast<unsigned char>(text_[pos_])) || text_[pos_] == '_')) {
                    ++pos_;
                }
                std::string word = text_.substr(start, pos_ - start);
                std::string ttype = KEYWORDS.count(word) ? toUpper(word) : "ID";
                tokens.push_back(Token{ttype, word, start});
                continue;
            }
 
            // symbols / operators
            bool matched = false;
            for (const auto& sym : SYMBOLS) {
                if (peekMatches(sym.text)) {
                    tokens.push_back(Token{sym.type, sym.text, pos_});
                    pos_ += std::string(sym.text).size();
                    matched = true;
                    break;
                }
            }
            if (!matched && peekMatches(SEMI_ENTRY.text)) {
                tokens.push_back(Token{SEMI_ENTRY.type, SEMI_ENTRY.text, pos_});
                pos_ += 1;
                matched = true;
            }
            if (matched) {
                continue;
            }
 
            std::ostringstream oss;
            oss << "Unexpected character '" << ch << "' at position " << pos_;
            throw LexError(oss.str());
        }
 
        tokens.push_back(Token{"EOF", "", pos_});
        return tokens;
    }
 
private:
    std::string text_;
    std::size_t pos_;
    std::size_t n_;
 
    bool peekMatches(const std::string& s) const {
        if (pos_ + s.size() > n_) return false;
        return text_.compare(pos_, s.size(), s) == 0;
    }
 
    static std::string toUpper(std::string s) {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }
};
 
 

 