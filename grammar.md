[Prog] ==> {

*     [Stmt]*

}

[Stmt] ==> {

*     exit([Expr]); 
*     let ident = [Expr];
*     ident = [Expr];
*     print([Expr]);
*     [Scope]
*     if ([Expr]) [Scope] [IfPred]
*     while([Expr])[Scope]

}

[Scope] ==> {

*     [Stmt]*

}

[IfPred] ==> {

*     elif([expr])[Scope][IfPred]
*     else[Scope]
*     ε

}

[Expr] ==> {

*     [Term]
*     [BinExpr]

}

[BinExpr] ==> {

*     [Expr] * [Expr] & precedence = 1
*     [Expr] / [Expr] & precedence = 1
*     [Expr] + [Expr] & precedence = 0
*     [Expr] - [Expr] & precedence = 0

}

[Term] ==>{

*     int_lit
*     ident
*     ([Expr])

}