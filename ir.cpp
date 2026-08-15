// IR GENERATOR 

 
/// Walks the AST and emits a flat list of three-address-code instructions
/// as strings. Uses temporaries (t1, t2, ...) for intermediate values and
/// labels (L1, L2, ...) for control flow.
class IRGenerator {
public:
    // -- public entry point --------------------------------------------------
    std::vector<std::string> generate(const ProgramPtr& program) {
        for (const NodePtr& stmt : program->statements) {
            genStmt(stmt);
        }
        return code_;
    }
 
private:
    std::vector<std::string> code_;
    int temp_count_ = 0;
    int label_count_ = 0;
 
    std::string newTemp() {
        ++temp_count_;
        return "t" + std::to_string(temp_count_);
    }
 
    std::string newLabel() {
        ++label_count_;
        return "L" + std::to_string(label_count_);
    }
 
    void emit(const std::string& instr) { code_.push_back(instr); }
 
    // -- statements ------------------------------------------------------
    void genStmt(const NodePtr& node) {
        if (auto n = std::dynamic_pointer_cast<Assign>(node)) {
            std::string value = genExpr(n->expr);
            emit(n->name + " = " + value);
            return;
        }
 
        if (auto n = std::dynamic_pointer_cast<Print>(node)) {
            std::string value = genExpr(n->expr);
            emit("PRINT " + value);
            return;
        }
 
        if (auto n = std::dynamic_pointer_cast<If>(node)) {
            std::string cond = genExpr(n->cond);
            std::string else_label = newLabel();
            std::string end_label = n->else_block ? newLabel() : else_label;
 
            emit("IF_FALSE " + cond + " GOTO " + else_label);
            for (const NodePtr& s : n->then_block->statements) {
                genStmt(s);
            }
 
            if (n->else_block) {
                emit("GOTO " + end_label);
                emit("LABEL " + else_label);
                for (const NodePtr& s : n->else_block->statements) {
                    genStmt(s);
                }
                emit("LABEL " + end_label);
            } else {
                emit("LABEL " + else_label);
            }
            return;
        }
 
        if (auto n = std::dynamic_pointer_cast<While>(node)) {
            std::string start_label = newLabel();
            std::string end_label = newLabel();
            emit("LABEL " + start_label);
            std::string cond = genExpr(n->cond);
            emit("IF_FALSE " + cond + " GOTO " + end_label);
            for (const NodePtr& s : n->body->statements) {
                genStmt(s);
            }
            emit("GOTO " + start_label);
            emit("LABEL " + end_label);
            return;
        }
 
        throw std::runtime_error("Unknown statement node: " + node->toString());
    }
 
    // -- expressions -----------------------------------------------------
    /// Returns the name (temp/var/literal) that holds the expression's value.
    std::string genExpr(const NodePtr& node) {
        if (auto n = std::dynamic_pointer_cast<Num>(node)) {
            return n->value;
        }
 
        if (auto n = std::dynamic_pointer_cast<Var>(node)) {
            return n->name;
        }
 
        if (auto n = std::dynamic_pointer_cast<UnaryOp>(node)) {
            std::string operand = genExpr(n->operand);
            std::string t = newTemp();
            emit(t + " = " + n->op + operand);
            return t;
        }
 
        if (auto n = std::dynamic_pointer_cast<BinOp>(node)) {
            std::string left = genExpr(n->left);
            std::string right = genExpr(n->right);
            std::string t = newTemp();
            emit(t + " = " + left + " " + n->op + " " + right);
            return t;
        }
 
        throw std::runtime_error("Unknown expression node: " + node->toString());
    }
};
 

//CONVENIENCE DRIVER

#include "lexer.cpp"
#include "parser.cpp"
#include "node.cpp"
struct CompileResult {
    std::vector<Token> tokens;
    ProgramPtr ast;
    std::vector<std::string> ir;
};
 
/// Runs the full pipeline and returns (tokens, ast, ir_lines).
CompileResult compile_source(const std::string& source, bool verbose = false) {
    std::vector<Token> tokens = Lexer(source).tokenize();
    ProgramPtr ast = Parser(tokens).parse();
    std::vector<std::string> ir = IRGenerator().generate(ast);
 
    if (verbose) {
        std::cout << "=== TOKENS ===\n";
        for (const Token& t : tokens) {
            std::cout << "  " << t.repr() << "\n";
        }
        std::cout << "\n=== AST ===\n";
        std::cout << "  " << ast->toString() << "\n";
        std::cout << "\n=== IR (three-address code) ===\n";
        for (const std::string& line : ir) {
            std::cout << "  " << line << "\n";
        }
    }
 
    return CompileResult{std::move(tokens), ast, std::move(ir)};
}