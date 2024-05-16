struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr *expr;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMinus {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd *, NodeBinExprMinus *, NodeBinExprMulti *, NodeBinExprDiv *> var; // var can be any of the types specified inside the variant<> similar to enum
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen *> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtPrint {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt *> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr *expr;
    NodeScope *scope;
    std::optional<NodeIfPred *> pred;
};

struct NodeIfPredElse {
    NodeScope *scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
};

struct NodeStmtIf {
    NodeExpr *expr;
    NodeScope *scope;
    std::optional<NodeIfPred *> pred;
};

struct NodeStmtWhile {
    NodeExpr *expr;
    NodeScope *scope;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeStmtPrint *, NodeScope *, NodeStmtIf *, NodeStmtAssign *, NodeStmtWhile *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts;
};