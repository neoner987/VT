#pragma once



namespace Node {

    struct NodeExpr {
        Token m_int_lit;
    };

    struct NodeExit {
        NodeExpr m_expr;
    };

}


class Parser {

public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)){

    }



    std::optional<Node::NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().Type == TokenType::int_lit) {
            return Node::NodeExpr {
                .m_int_lit = consume()
            };
        }
    }

    std::optional<Node::NodeExit> parse() {
        std::optional<Node::NodeExit> exit_node;
        while (peek().has_value()) {
            if (peek().value().Type == TokenType::exit) {
                consume();
                if (auto node_expr = parse_expr()) {
                    exit_node = Node::NodeExit { .m_expr = node_expr.value() };
                }
                else {
                    std::cout << "Bruh! Invalid expr after exit.";
                    exit(EXIT_FAILURE);
                }
                if (!peek().has_value() || peek().value().Type != TokenType::semi) {
                    std::cout << "Bruh! No semi after exit T_T ";
                    exit(EXIT_FAILURE);
                }
                else {
                    consume();
                }
            }
        }
        m_index = 0;
        return exit_node;
    }


private:
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens[m_index + offset];
        }
    };

    inline Token consume() {
        return m_tokens[m_index++];
    }
    size_t m_index = 0;
    const std::vector<Token> m_tokens;
};

