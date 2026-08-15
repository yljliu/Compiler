// 3. PARSER (recursive descent)

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};
 
class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos_(0) {}
 
    // -- entry point ---------------------------------------------------------
    ProgramPtr parse() {
        auto program = std::make_shared<Program>();
        while (!check({"EOF"})) {
            program->statements.push_back(statement());
        }
        return program;
    }
 
private:
    std::vector<Token> tokens_;
    std::size_t pos_;
 
    // -- helpers -----------------------------------------------------------
    const Token& current() const { return tokens_[pos_]; }
 
    Token advance() {
        Token tok = tokens_[pos_];
        if (tok.type != "EOF") {
            ++pos_;
        }
        return tok;
    }
 
    Token expect(const std::string& ttype) {
        const Token& tok = current();
        if (tok.type != ttype) {
            std::ostringstream oss;
            oss << "Expected " << ttype << " but got " << tok.type
                << " (\"" << tok.value << "\") at pos " << tok.pos;
            throw ParseError(oss.str());
        }
        return advance();
    }
 
    bool check(std::initializer_list<const char*> ttypes) const {
        const std::string& cur = current().type;
        for (const char* t : ttypes) {
            if (cur == t) return true;
        }
        return false;
    }
 
    // -- statements ----------------------------------------------------------
    BlockPtr block() {
        expect("LBRACE");
        auto blk = std::make_shared<Block>();
        while (!check({"RBRACE"})) {
            blk->statements.push_back(statement());
        }
        expect("RBRACE");
        return blk;
    }
 
    NodePtr statement() {
        const Token& tok = current();
 
        if (tok.type == "IF") return ifStmt();
        if (tok.type == "WHILE") return whileStmt();
        if (tok.type == "PRINT") return printStmt();
        if (tok.type == "ID") return assignStmt();
 
        std::ostringstream oss;
        oss << "Unexpected token " << tok.type << " (\"" << tok.value
            << "\") at pos " << tok.pos;
        throw ParseError(oss.str());
    }
 
    NodePtr assignStmt() {
        Token name_tok = expect("ID");
        expect("ASSIGN");
        NodePtr expr = this->expr();
        expect("SEMI");
        return std::make_shared<Assign>(name_tok.value, expr);
    }
 
    NodePtr ifStmt() {
        expect("IF");
        expect("LPAREN");
        NodePtr cond = expr();
        expect("RPAREN");
        BlockPtr then_block = block();
        BlockPtr else_block = nullptr;
        if (check({"ELSE"})) {
            advance();
            else_block = block();
        }
        return std::make_shared<If>(cond, then_block, else_block);
    }
 
    NodePtr whileStmt() {
        expect("WHILE");
        expect("LPAREN");
        NodePtr cond = expr();
        expect("RPAREN");
        BlockPtr body = block();
        return std::make_shared<While>(cond, body);
    }
 
    NodePtr printStmt() {
        expect("PRINT");
        expect("LPAREN");
        NodePtr e = expr();
        expect("RPAREN");
        expect("SEMI");
        return std::make_shared<Print>(e);
    }
 
    // -- expressions (precedence climbing) -----------------------------------
    NodePtr expr() { return comparison(); }
 
    NodePtr comparison() {
        NodePtr node = term();
        while (check({"LT", "GT", "LE", "GE", "EQ", "NE"})) {
            Token op_tok = advance();
            NodePtr right = term();
            node = std::make_shared<BinOp>(op_tok.value, node, right);
        }
        return node;
    }
 
    NodePtr term() {
        NodePtr node = factor();
        while (check({"PLUS", "MINUS"})) {
            Token op_tok = advance();
            NodePtr right = factor();
            node = std::make_shared<BinOp>(op_tok.value, node, right);
        }
        return node;
    }
 
    NodePtr factor() {
        NodePtr node = unary();
        while (check({"STAR", "SLASH"})) {
            Token op_tok = advance();
            NodePtr right = unary();
            node = std::make_shared<BinOp>(op_tok.value, node, right);
        }
        return node;
    }
 
    NodePtr unary() {
        if (check({"MINUS"})) {
            advance();
            NodePtr operand = unary();
            return std::make_shared<UnaryOp>("-", operand);
        }
        return primary();
    }
 
    NodePtr primary() {
        const Token& tok = current();
        if (tok.type == "NUMBER") {
            advance();
            return std::make_shared<Num>(tok.value);
        }
        if (tok.type == "ID") {
            advance();
            return std::make_shared<Var>(tok.value);
        }
        if (tok.type == "LPAREN") {
            advance();
            NodePtr node = expr();
            expect("RPAREN");
            return node;
        }
        std::ostringstream oss;
        oss << "Unexpected token " << tok.type << " (\"" << tok.value
            << "\") at pos " << tok.pos;
        throw ParseError(oss.str());
    }
};