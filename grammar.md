$$
\begin{align}

[\text{Prog}] &\to [\text{Stmt}]* \\

[\text{Stmt}] &\to 

\begin{cases}

[\text{Exit}]\\
[\text{Int Var}]\\
[\text{Bool Var}] \\
[\text{Scope}]\\
[\text{If}]\\
[\text{VarReassigment}]\\
[\text{VarIncrement}] \\ 
[\text{VarDecrement}] \\ 
[\text{InstOp}] \\
[\text{Repeat}] \\

\end{cases} \\


[\text{Exit}] &\to \text{вийти}([\text{Expr}]); \\

[\text{Int Var}] &\to \text{ціле ident} = [\text{Expr}]; \\

[\text{Scope}] &\to \{  [Stmt]*  \} \\

[\text{If}] &\to \text{якщо} ([\text{BoolExpr}] ) [\text{Scope}] [\text{If\_Pred}] \\

[\text{If\_Pred}] &\to 
\begin{cases}

\text{інак якщо}([\text{BoolExpr}] ) [\text{Scope}] [\text{If\_Pred}] \\
\text{інак} [\text{Scope}]  \\
null\\

\end{cases}\\

[\text{VarReassigment}] &\to \text{ident} = [\text{Expr}]; \\


[\text{Expr}] &\to
\begin{cases}

[\text{Term}] \\
[\text{BinExpr}] \\

\end{cases}\\


[\text{Term}] &\to
\begin{cases}

\text{int\_lit} \\
[\text{Int Var}] \\

\end{cases}\\


[\text{BinExpr}] &\to
\begin{cases}

[\text{Expr}] + [\text{Expr}];  \text{pres = 0} \\
[\text{Expr}] - [\text{Expr}];  \text{pres = 0} \\
[\text{Expr}] * [\text{Expr}];  \text{pres = 1} \\
[\text{Expr}] / [\text{Expr}];  \text{pres = 1} \\

\end{cases}\\

[\text{VarIncrement}] &\to [\text{Var}]++; \\



[\text{VarDecrement}] &\to [\text{Var}] --; \\


[\text{InstOp}] &\to 
\begin{cases}

[\text{Var}] += [\text{Expr}] \\
[\text{Var}] -= [\text{Expr}] \\
[\text{Var}] *= [\text{Expr}] \\
[\text{Var}] /= [\text{Expr}] \\

\end{cases}\\

[\text{Repeat}] &\to \text{повторити}([\text{Expr}]) [\text{Scope}] \\


[\text{BoolExpr}] &\to 
\begin{cases}

 \text{BoolTerm} \\


\end{cases}\\



[\text{BoolTerm}] &\to 
\begin{cases}

 \text{bool\_lit} \\
[\text{Bool Var}] \\

\end{cases}\\


\end{align}
$$
