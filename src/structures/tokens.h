enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    print,
    plus,
    multi,
    minus,
    div,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
    while_
};

std::string to_string(const TokenType type) {
    switch (type) {
        case TokenType::exit:
            return "`exit`";
        case TokenType::int_lit:
            return "int literal";
        case TokenType::semi:
            return "`;`";
        case TokenType::open_paren:
            return "`(`";
        case TokenType::close_paren:
            return "`)`";
        case TokenType::ident:
            return "identifier";
        case TokenType::let:
            return "`let`";
        case TokenType::eq:
            return "`=`";
        case TokenType::plus:
            return "`+`";
        case TokenType::multi:
            return "`*`";
        case TokenType::minus:
            return "`-`";
        case TokenType::div:
            return "`/`";
        case TokenType::open_curly:
            return "`{`";
        case TokenType::close_curly:
            return "`}`";
        case TokenType::if_:
            return "`if`";
        case TokenType::elif:
            return "`elif`";
        case TokenType::else_:
            return "`else`";
        case TokenType::print:
            return "`print`";
        case TokenType::while_:
            return "`while`";
    }
    assert(false);
}

std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::multi:
        case TokenType::div:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    std::optional<std::string> value{};
};