[Prog] ==> {

*     [Stmt]*

}

[Stmt] ==> {

*     exit([Expr]); 
*     let ident = [Expr];
*     if ([Expr]) [Scope]
*     print([Expr]);

}

[Scope] ==> {

*     [Stmt]*

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