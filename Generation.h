#pragma once


class Generation {
public:
    inline explicit Generation(Node::NodeExit root, std::string path)
        :m_exit_node(std::move(root)), m_path(std::move(path))
    {

    }

    inline void generate_assembly() const {
        std::ofstream outFile(m_path, std::ios::out);

        outFile << "global _start:" << std::endl;
        outFile << "_start:" << std::endl;
        outFile << "\tmov rax, 60" << std::endl;
        outFile << "\t" << "mov rdi, " << m_exit_node.m_expr.m_int_lit.Value->c_str() << std::endl;
        outFile << "\tsyscall" << std::endl;
        outFile.close();
    }

private:
    const Node::NodeExit m_exit_node;
    std::string m_path;
};