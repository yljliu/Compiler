// AST NODE DEFINITIONS


// Base class for all AST nodes. Each concrete node also implements
// toString() (used in place of Python's auto-generated dataclass repr).
class Node {
public:
    virtual ~Node() = default;
    virtual std::string toString() const = 0;
};

using NodePtr = std::shared_ptr<Node>;

class Num : public Node {
public:
    explicit Num(std::string value) : value(std::move(value)) {}
    std::string value;

    std::string toString() const override {
        return "Num(value='" + value + "')";
    }
};

class Var : public Node {
public:
    explicit Var(std::string name) : name(std::move(name)) {}
    std::string name;

    std::string toString() const override {
        return "Var(name='" + name + "')";
    }
};

class BinOp : public Node {
public:
    BinOp(std::string op, NodePtr left, NodePtr right)
        : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
    std::string op;
    NodePtr left;
    NodePtr right;

    std::string toString() const override {
        return "BinOp(op='" + op + "', left=" + left->toString() +
               ", right=" + right->toString() + ")";
    }
};

class UnaryOp : public Node {
public:
    UnaryOp(std::string op, NodePtr operand)
        : op(std::move(op)), operand(std::move(operand)) {}
    std::string op;
    NodePtr operand;

    std::string toString() const override {
        return "UnaryOp(op='" + op + "', operand=" + operand->toString() + ")";
    }
};

class Assign : public Node {
public:
    Assign(std::string name, NodePtr expr)
        : name(std::move(name)), expr(std::move(expr)) {}
    std::string name;
    NodePtr expr;

    std::string toString() const override {
        return "Assign(name='" + name + "', expr=" + expr->toString() + ")";
    }
};

class Print : public Node {
public:
    explicit Print(NodePtr expr) : expr(std::move(expr)) {}
    NodePtr expr;

    std::string toString() const override {
        return "Print(expr=" + expr->toString() + ")";
    }
};

class Block : public Node {
public:
    std::vector<NodePtr> statements;

    std::string toString() const override {
        std::ostringstream oss;
        oss << "Block(statements=[";
        for (std::size_t i = 0; i < statements.size(); ++i) {
            if (i) oss << ", ";
            oss << statements[i]->toString();
        }
        oss << "])";
        return oss.str();
    }
};

using BlockPtr = std::shared_ptr<Block>;

class If : public Node {
public:
    If(NodePtr cond, BlockPtr then_block, BlockPtr else_block = nullptr)
        : cond(std::move(cond)), then_block(std::move(then_block)),
          else_block(std::move(else_block)) {}
    NodePtr cond;
    BlockPtr then_block;
    BlockPtr else_block; // nullptr means "no else", mirroring Python's None

    std::string toString() const override {
        std::ostringstream oss;
        oss << "If(cond=" << cond->toString()
            << ", then_block=" << then_block->toString()
            << ", else_block=" << (else_block ? else_block->toString() : "None") << ")";
        return oss.str();
    }
};

class While : public Node {
public:
    While(NodePtr cond, BlockPtr body) : cond(std::move(cond)), body(std::move(body)) {}
    NodePtr cond;
    BlockPtr body;

    std::string toString() const override {
        return "While(cond=" + cond->toString() + ", body=" + body->toString() + ")";
    }
};

class Program : public Node {
public:
    std::vector<NodePtr> statements;

    std::string toString() const override {
        std::ostringstream oss;
        oss << "Program(statements=[";
        for (std::size_t i = 0; i < statements.size(); ++i) {
            if (i) oss << ", ";
            oss << statements[i]->toString();
        }
        oss << "])";
        return oss.str();
    }
};