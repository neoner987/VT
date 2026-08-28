$$
\begin{align}

[\text{Prog}] &\to [\text{Stmt}]* \\

[\text{Stmt}] &\to 

\begin{cases}

[\text{Exit}]\\
[\text{IntVar}]\\
[\text{BoolVar}] \\
[\text{Scope}]\\
[\text{If}]\\
[\text{VarReassigment}]\\
[\text{VarIncrement}] \\ 
[\text{VarDecrement}] \\ 
[\text{InstOp}] \\
[\text{Repeat}] \\

\end{cases} \\


[\text{Exit}] &\to \text{вийти}([\text{IntExpr}]); \\

[\text{Int Var}] &\to \text{ціле ident} = [\text{Expr}]; \\

[\text{Scope}] &\to \{  [Stmt]*  \} \\

[\text{If}] &\to \text{якщо} ([\text{BoolExpr}] ) [\text{Scope}] [\text{If\_Pred}] \\

[\text{If\_Pred}] &\to 
\begin{cases}

\text{інак якщо}([\text{BoolExpr}] ) [\text{Scope}] [\text{If\_Pred}] \\
\text{інак} [\text{Scope}]  \\

null\\

\end{cases}\\

[\text{VarReassigment}] &\to \text{ident} = 
\begin{cases}

\text{Bool\_lit} \\
\text{int\_lit} \\
\text{ident}

\end{cases}\\


[\text{IntExpr}] &\to
\begin{cases}

[\text{Term}] \\
[\text{BinExpr}] \\

\end{cases}\\


[\text{Term}] &\to
\begin{cases}

\text{int\_lit} \\
[\text{IntVar}] \\
[\text{IntParentExpr}]\\

\end{cases}\\


[\text{IntParentExpr}] &\to [\text{IntExpr}] \\

[\text{BinExpr}] &\to
\begin{cases}

[\text{IntExpr}] + [\text{IntExpr}];  \text{pres = 0} \\
[\text{IntExpr}] - [\text{IntExpr}];  \text{pres = 0} \\
[\text{IntExpr}] * [\text{IntExpr}];  \text{pres = 1} \\
[\text{IntExpr}] / [\text{IntExpr}];  \text{pres = 1} \\

\end{cases}\\

[\text{VarIncrement}] &\to [\text{IntVar}]++; \\



[\text{VarDecrement}] &\to [\text{IntVar}] --; \\


[\text{InstOp}] &\to 
\begin{cases}

[\text{IntVar}] += [\text{IntExpr}] \\
[\text{IntVar}] -= [\text{IntExpr}] \\
[\text{IntVar}] *= [\text{IntExpr}] \\
[\text{IntVar}] /= [\text{IntExpr}] \\

\end{cases}\\

[\text{Repeat}] &\to \text{повторити}([\text{Expr}]) [\text{Scope}] \\


[\text{BoolOpExpr}] &\to
\begin{cases}

[\text{BoolExpr}] \&\& [\text{BoolExpr}]; \\
[\text{BoolExpr}] || [\text{BoolExpr}]; \\
![\text{BoolExpr}] \\

[\text{IntVar}] == [\text{IntExpr}] \\
[\text{IntVar}] != [\text{IntExpr}] \\
[\text{IntVar}] > [\text{IntExpr}] \\
[\text{IntVar}] < [\text{IntExpr}] \\
[\text{IntVar}] >= [\text{IntExpr}] \\
[\text{IntVar}] <= [\text{IntExpr}] \\


\end{cases}\\


[\text{BoolExpr}] &\to 
\begin{cases}

 [\text{BoolTerm}] \\
 [\text{BoolOpExpr}] \\


\end{cases}\\



[\text{BoolTerm}] &\to 
\begin{cases}

 \text{bool\_lit} \\
[\text{BoolVar}] \\
[\text{BoolParentExpr}] \\

\end{cases}\\

[\text{BoolParentExpr}] &\to [\text{BoolExpr}]\\


\end{align}
$$
