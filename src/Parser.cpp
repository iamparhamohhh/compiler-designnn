#include "Parser.h"


// main point is that the whole input has been consumed
Program *Parser::parse()
{
    Program *Res = parseProgram();
    return Res;
}

Program *Parser::parseProgram()
{
    llvm::SmallVector<AST *> data;
    
    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
        case Token::KW_int: {
            DeclarationInt *d;
            d = parseIntDec();
            if (d)
                data.push_back(d);
            else
                goto _error;
                
            break;
        }
        case Token::KW_bool: {
            DeclarationBool *dbool;
            dbool = parseBoolDec();
            if (dbool)
                data.push_back(dbool);
            else
                goto _error;

            break;
        }
        case Token::ident: {
            Token prev_token = Tok;
            const char* prev_buffer = Lex.getBuffer();
            UnaryOp *u;
            u = parseUnary();
            if (Tok.is(Token::semicolon))
            {
                if (u)
                {
                    data.push_back(u);
                    break;
                }
                else{
                    goto _error;
                    
                }
            }
            else
            {
                if (u)
                {
                    goto _error;
                }
                else{
                    Tok = prev_token;
                    Lex.setBufferPtr(prev_buffer);
                }
            }
        
            Assignment *a_int;
            Assignment *a_bool;
            prev_token = Tok;
            prev_buffer = Lex.getBuffer();

            a_bool = parseBoolAssign();

            if (a_bool){
                data.push_back(a_bool);
                break;
            }
            Tok = prev_token;
            Lex.setBufferPtr(prev_buffer);

            a_int = parseIntAssign();
            if (!Tok.is(Token::semicolon))
            {
                goto _error;
            }
            if (a_int)
                data.push_back(a_int);
            else
                goto _error;
                
            break;
        }
        case Token::KW_if: {
            IfStmt *i;
            i = parseIf();
            if (i)
                data.push_back(i);
            else
                goto _error;
            
            break;
        }
        case Token::KW_while: {
            WhileStmt *w;
            w = parseWhile();
            if (w)
                data.push_back(w);
            else {
                goto _error;
            }
            break;
        }
        case Token::KW_for: {
            ForStmt *f;
            f = parseFor();
            if (f)
                data.push_back(f);
            else {
                goto _error;
            }
            break;
        }
        case Token::KW_print: {
            PrintStmt *p;
            p = parsePrint();
            if (p)
                data.push_back(p);
            else {
                goto _error;
            }
            break;
        }
        case Token::start_comment: {
            parseComment();
            if (!Tok.is(Token::end_comment))
                goto _error;
            break;
        }
        default: {
            error();

            goto _error;
            break;
        }
        }
        advance();
        
    }
    return new Program(data);
_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

DeclarationInt *Parser::parseIntDec()
{
    Expr *E = nullptr;
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Expr *> Values;
    
    if (expect(Token::KW_int)){
        goto _error;
    }
    advance();
    
    if (expect(Token::ident)){
        goto _error;
    }

    Vars.push_back(Tok.getText());
    advance();

    if (Tok.is(Token::assign))
    {
        advance();
        E = parseExpr();
        if(E){
            Values.push_back(E);
        }
        else{
            goto _error;
        }
    }
    else
    {
        Values.push_back(new Final(Final::Number, llvm::StringRef("0")));
    }
    
    
    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::ident)){
            goto _error;
        }
            
        Vars.push_back(Tok.getText());
        advance();

        if(Tok.is(Token::assign)){
            advance();
            E = parseExpr();
            if(E){
                Values.push_back(E);
            }
            else{
                goto _error;
            }
        }
        else{
            Values.push_back(new Final(Final::Number, llvm::StringRef("0")));
        }
    }

    if (expect(Token::semicolon)){
        goto _error;
    }


    return new DeclarationInt(Vars, Values);
_error: 
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}


DeclarationBool *Parser::parseBoolDec()
{
    Logic *L = nullptr;
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Logic *> Values;
    
    if (expect(Token::KW_bool)){
        goto _error;
    }
    advance();
    
    if (expect(Token::ident)){
        goto _error;
    }

    Vars.push_back(Tok.getText());
    advance();

    if (Tok.is(Token::assign))
    {
        advance();
        L = parseLogic();
        if(L){
            Values.push_back(L);
        }
        else{
            goto _error;
        }
    }
    else
    {
        Values.push_back(new Comparison(nullptr, nullptr, Comparison::False));
    }
    
    
    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::ident)){
            goto _error;
        }
            
        Vars.push_back(Tok.getText());
        advance();

        if(Tok.is(Token::assign)){
            advance();
            L = parseLogic();
            if(L){
                Values.push_back(L);
            }
            else{
                goto _error;
            }
        }
        else{
            Values.push_back(new Comparison(nullptr, nullptr, Comparison::False));
        }
    }

    if (expect(Token::semicolon)){
        goto _error;
    }
    return new DeclarationBool(Vars, Values);
_error: 
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}



Assignment *Parser::parseBoolAssign()
{
    Final *F = nullptr;
    Assignment::AssignKind AK;
    Logic *L = nullptr;

    F = (Final *)(parseFinal());
    if (F == nullptr)
    {
        goto _error;
    }
    
    if (Tok.is(Token::assign))
    {
        AK = Assignment::Assign;
        advance();
        L = parseLogic();   // check if expr is logical

        if(L)
        {
            if (!Tok.is(Token::semicolon))
            {
                goto _error;
            }
            return new Assignment(F, nullptr, AK, L);
        }
        else
            goto _error;
    }
    else
    {
        goto _error;
    }
    
_error:
        while (Tok.getKind() != Token::eoi)
            advance();
        return nullptr;
    
}

Assignment *Parser::parseIntAssign()
{
    Expr *E = nullptr;
    Final *F = nullptr;
    Assignment::AssignKind AK;
    F = (Final *)(parseFinal());
    if (F == nullptr)
    {
        goto _error;
    }
    
    if (Tok.is(Token::assign))
    {
        AK = Assignment::Assign;
    }
    else if (Tok.is(Token::plus_assign))
    {
        AK = Assignment::Plus_assign;
    }
    else if (Tok.is(Token::minus_assign))
    {
        AK = Assignment::Minus_assign;
    }
    else if (Tok.is(Token::star_assign))
    {
        AK = Assignment::Star_assign;
    }
    else if (Tok.is(Token::slash_assign))
    {
        AK = Assignment::Slash_assign;
    }
    else
    {
        goto _error;
    }
    advance();
    E = parseExpr();    // check for mathematical expr
    if(E){
        return new Assignment(F, E, AK, nullptr);
    }
    else{
        goto _error;
    }

_error:
        while (Tok.getKind() != Token::eoi)
            advance();
        return nullptr;
}


UnaryOp *Parser::parseUnary()
{
    UnaryOp* Res = nullptr;
    llvm::StringRef var;

    if (expect(Token::ident)){
        goto _error;
    }

    var = Tok.getText();
    advance();
    if (Tok.getKind() == Token::plus_plus){
        Res = new UnaryOp(UnaryOp::Plus_plus, var);
    }
    else if(Tok.getKind() == Token::minus_minus){
        Res = new UnaryOp(UnaryOp::Minus_minus, var);
    }
    else{
        goto _error;
    }

    advance();

    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}
Expr *Parser::parseExpr()
{
    Expr *Left = parseTerm();

    if (Left == nullptr)
    {
        goto _error;
    }
    
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        BinaryOp::Operator Op;
        if (Tok.is(Token::plus))
            Op = BinaryOp::Plus;
        else if (Tok.is(Token::minus))
            Op = BinaryOp::Minus;
        else {
            error();

            goto _error;
        }
        advance();
        Expr *Right = parseTerm();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    if (Left == nullptr)
    {
        goto _error;
    }
    while (Tok.isOneOf(Token::star, Token::mod, Token::slash))
    {
        BinaryOp::Operator Op;
        if (Tok.is(Token::star))
            Op = BinaryOp::Mul;
        else if (Tok.is(Token::slash))
            Op = BinaryOp::Div;
        else if (Tok.is(Token::mod))
            Op = BinaryOp::Mod;
        else {
            error();

            goto _error;
        }
        advance();
        Expr *Right = parseFactor();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseFactor()
{
    Expr *Left = parseFinal();
    if (Left == nullptr)
    {
        goto _error;
    }
    while (Tok.is(Token::exp))
    {
        BinaryOp::Operator Op;
        if (Tok.is(Token::exp))
            Op = BinaryOp::Exp;
        else {
            error();
            goto _error;
        }
        advance();
        Expr *Right = parseFactor();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseFinal()
{
    Expr *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::number:{
        Res = new Final(Final::Number, Tok.getText());
        advance();
        break;
    }
    case Token::ident: {
        Res = new Final(Final::Ident, Tok.getText());
        Token prev_tok = Tok;
        const char* prev_buffer = Lex.getBuffer();
        Expr* u = parseUnary();
        if(u)
            return u;
        else{
            Tok = prev_tok;
            Lex.setBufferPtr(prev_buffer);
            advance();
        }
        break;
    }
    case Token::plus:{
        advance();
        if(Tok.getKind() == Token::number){
            Res = new SignedNumber(SignedNumber::Plus, Tok.getText());
            advance();
            break;
        }
        goto _error;
    }

    case Token::minus:{
        advance();
        if (Tok.getKind() == Token::number){
            Res = new SignedNumber(SignedNumber::Minus, Tok.getText());
            advance();
            break;
        }
        goto _error;
    }
    case Token::minus_paren:{
        advance();
        Expr *math_expr = parseExpr();
        if(math_expr == nullptr)
            goto _error;
        Res = new NegExpr(math_expr);
        if (!consume(Token::r_paren))
            break;
        
        goto _error;

    }
    case Token::l_paren:{
        advance();
        Res = parseExpr();
        if(Res == nullptr){
            goto _error;
        }
        if (!consume(Token::r_paren))
            break;
        
    }
    case Token::KW_xor:{//========================================================================================================
        advance();
        if (Tok.getKind() == Token::number){
            Res = new SignedNumber(SignedNumber::Xor, Tok.getText());
            advance();
            break;
        }
        goto _error;
    }
    default:{
        error();
        goto _error;
    }
    }
    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Logic *Parser::parseComparison()
{
    Logic *Res = nullptr;
    Final *Ident = nullptr;
    Expr *Left = nullptr;
    Expr *Right = nullptr;
    Token prev_Tok;
    const char* prev_buffer;
    if (Tok.is(Token::l_paren)) {
        advance();
        Res = parseLogic();
        if (Res == nullptr)
        {
            goto _error;
        }
        if (consume(Token::r_paren))
            goto _error;
    }
    else {
        if(Tok.is(Token::KW_true)){
            Res = new Comparison(nullptr, nullptr, Comparison::True);
            advance();
            return Res;
        }
        else if(Tok.is(Token::KW_false)){
            Res = new Comparison(nullptr, nullptr, Comparison::False);
            advance();
            return Res;
        }
        else if(Tok.is(Token::ident)){
            Ident = new Final(Final::Ident, Tok.getText());
        }
        prev_Tok = Tok;
        prev_buffer = Lex.getBuffer();
        Left = parseExpr();
        if(Left == nullptr)
            goto _error;
        

        Comparison::Operator Op;
            if (Tok.is(Token::eq))
                Op = Comparison::Equal;
            else if (Tok.is(Token::neq))
                Op = Comparison::Not_equal;
            else if (Tok.is(Token::gt))
                Op = Comparison::Greater;
            else if (Tok.is(Token::lt))
                Op = Comparison::Less;
            else if (Tok.is(Token::gte))
                Op = Comparison::Greater_equal;
            else if (Tok.is(Token::lte))
                Op = Comparison::Less_equal;    
            else {
                if (Ident){
                    Tok = prev_Tok;
                    Lex.setBufferPtr(prev_buffer);
                    Res = new Comparison(Ident, nullptr, Comparison::Ident);
                    advance();
                    return Res;
                }
                goto _error;
            }
            advance();
            Right = parseExpr();
            if (Right == nullptr)
            {
                goto _error;
            }
            
            Res = new Comparison(Left, Right, Op);
    }
    
    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Logic *Parser::parseLogic()
{
    Logic *Left = parseComparison();
    Logic *Right;
    if (Left == nullptr)
    {
        goto _error;
    }
    while (Tok.isOneOf(Token::KW_and, Token::KW_or))
    {
        LogicalExpr::Operator Op;
        if (Tok.is(Token::KW_and))
            Op = LogicalExpr::And;
        else if (Tok.is(Token::KW_or))
            Op = LogicalExpr::Or;
        else {
            error();

            goto _error;
        }
        advance();
        Right = parseComparison();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new LogicalExpr(Left, Right, Op);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

IfStmt *Parser::parseIf()
{
    llvm::SmallVector<AST *> ifStmts;
    llvm::SmallVector<AST *> elseStmts;
    llvm::SmallVector<elifStmt *> elifStmts;
    llvm::SmallVector<AST *> Stmts;
    Logic *Cond = nullptr;
    Token prev_token_if;
    const char* prev_buffer_if;
    Token prev_token_elif;
    const char* prev_buffer_elif;
    bool hasElif = false;
    bool hasElse = false;


    if (expect(Token::KW_if)){
        goto _error;
    }

    advance();

    if (expect(Token::l_paren)){
        goto _error;
    }

    advance();

    Cond = parseLogic();
    if (Cond == nullptr)
    {
        goto _error;
    }

    if (expect(Token::r_paren)){
        goto _error;
    }
        
    advance();

    if (expect(Token::l_brace)){
        goto _error;
    }

    advance();
    
    ifStmts = getBody();
        
    if(ifStmts.empty())
        goto _error;
    
    prev_token_if = Tok;
    prev_buffer_if = Lex.getBuffer();
    
    advance();

    while (true)
    {
        if (Tok.is(Token::KW_else))
        {
            advance();
            if (Tok.is(Token::KW_if))
            {
                hasElif = true;
                advance();
                
                if (expect(Token::l_paren)){
                    goto _error;
                }

                advance();

                Logic *Cond = parseLogic();

                if (Cond == nullptr)
                {
                    goto _error;
                }

                if (expect(Token::r_paren)){
                    goto _error;
                }

                advance();

                if (expect(Token::l_brace)){
                    goto _error;
                }

                advance();

                Stmts = getBody();
                prev_token_elif = Tok;
                prev_buffer_elif = Lex.getBuffer();
                
                if(!Stmts.empty())
                    advance();
                else
                    goto _error;
                
                elifStmt *elif = new elifStmt(Cond, Stmts);
                elifStmts.push_back(elif);
            }
            else
            {
                hasElse = true;

                if (expect(Token::l_brace)){
                    goto _error;
                }

                advance();

                elseStmts = getBody();
                
                if(elseStmts.empty())
                    goto _error;

                break;
            }
        }
        else
            break;
    }

    if(hasElif && !hasElse){
        Tok = prev_token_elif;
        Lex.setBufferPtr(prev_buffer_elif);
    }
    else if(!hasElif && !hasElse){
        Tok = prev_token_if;
        Lex.setBufferPtr(prev_buffer_if);
    }
        
    return new IfStmt(Cond, ifStmts, elseStmts, elifStmts);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}



PrintStmt *Parser::parsePrint()
{
    llvm::StringRef Var;
    if (expect(Token::KW_print)){
        goto _error;
    }
    advance();
    if (expect(Token::l_paren)){
        goto _error;
    }
    advance();
    if (expect(Token::ident)){
        goto _error;
    }
    Var = Tok.getText();
    advance();
    if (expect(Token::r_paren)){
        goto _error;
    }
    advance();
    if (expect(Token::semicolon)){
        goto _error;
    }
    return new PrintStmt(Var);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;

}

WhileStmt *Parser::parseWhile()
{
    llvm::SmallVector<AST *> Body;
    Logic *Cond = nullptr;

    if (expect(Token::KW_while)){
        goto _error;
    }
        
    advance();

    if(expect(Token::l_paren)){
        goto _error;
    }

    advance();

    Cond = parseLogic();
    if (Cond == nullptr)
    {
        goto _error;
    }
    if(expect(Token::r_paren)){
        goto _error;
    }

    advance();

    if (expect(Token::l_brace)){
        goto _error;
    }

    advance();

    Body = getBody();
    if(Body.empty())
        goto _error;
        

    return new WhileStmt(Cond, Body);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

ForStmt *Parser::parseFor()
{
    Assignment *First = nullptr;
    Logic *Second = nullptr;
    Assignment *ThirdAssign = nullptr;
    UnaryOp *ThirdUnary = nullptr;
    llvm::SmallVector<AST *> Body;
    Token prev_token;
    const char* prev_buffer;

    if (expect(Token::KW_for)){
        goto _error;
    }
        
    advance();

    if(expect(Token::l_paren)){
        goto _error;
    }

    advance();

    First = parseIntAssign();

    if (First == nullptr)
        goto _error;
        
    if (First->getAssignKind() != Assignment::Assign)    // The first part can only have a '=' sign
        goto _error;

    if(expect(Token::semicolon)){
        goto _error;
    }

    advance();

    Second = parseLogic();

    if (Second == nullptr)
        goto _error;
        
    if(expect(Token::semicolon)){
        goto _error;
    }

    advance();

    prev_token = Tok;
    prev_buffer = Lex.getBuffer();

    ThirdAssign = parseIntAssign();

    if (ThirdAssign == nullptr){
        Tok = prev_token;
        Lex.setBufferPtr(prev_buffer);

        ThirdUnary = parseUnary();
        if (ThirdUnary == nullptr){
            goto _error;
        }

    }
    else{
        if(ThirdAssign->getAssignKind() == Assignment::Assign)   // The third part cannot have only '=' sign
            goto _error;
    }


    if(expect(Token::r_paren)){
        goto _error;
    }

    advance();

    if(expect(Token::l_brace)){
        goto _error;
    }

    advance();

    Body = getBody();

    if (Body.empty())
        goto _error;

    return new ForStmt(First, Second, ThirdAssign, ThirdUnary, Body);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;  

}

void Parser::parseComment()
{
    if (expect(Token::start_comment)) {
        goto _error;
    }
    advance();

    while (!Tok.isOneOf(Token::end_comment, Token::eoi)) advance();

    return;
_error: 
    while (Tok.getKind() != Token::eoi)
        advance();
}
//====================================================================================================================
SwitchStmt *Parser::parseSwitch() {
    if (!Tok.is(Token::KW_switch)) {
        error();
        return nullptr;
    }
    advance(); // Consume 'switch'

    if (!Tok.is(Token::l_paren)) {
        error();
        return nullptr;
    }
    advance(); // Consume '('

    // Parse the switch condition
    Expr *condition = parseExpr();
    if (!condition) {
        error();
        return nullptr;
    }

    if (!Tok.is(Token::r_paren)) {
        error();
        return nullptr;
    }
    advance(); // Consume ')'

    if (!Tok.is(Token::l_brace)) {
        error();
        return nullptr;
    }
    advance(); // Consume '{'

    // Parse cases and default
    llvm::SmallVector<CaseStmt *> cases;
    DefaultStmt *defaultCase = nullptr;

    while (!Tok.is(Token::r_brace)) {
        if (Tok.is(Token::KW_case)) {
            CaseStmt *caseStmt = parseCase();
            if (!caseStmt) {
                error();
                return nullptr;
            }
            cases.push_back(caseStmt);
        } else if (Tok.is(Token::KW_default)) {
            if (defaultCase != nullptr) {
                error(); // Only one default case allowed
                return nullptr;
            }
            defaultCase = parseDefault();
            if (!defaultCase) {
                error();
                return nullptr;
            }
        } else {
            error(); // Unexpected token inside switch
            return nullptr;
        }
    }
    advance(); // Consume '}'

    // Return the switch statement node
    return new SwitchStmt(condition, cases, defaultCase);
}


CaseStmt *Parser::parseCase() {
    // Ensure we start with 'case'
    if (!Tok.is(Token::KW_case)) {
        error();
        return nullptr;
    }
    advance(); // Consume 'case'

    // Parse the case value (e.g., an integer, character, or expression)
    Expr *caseValue = parseExpr();
    if (!caseValue) {
        error();
        return nullptr;
    }

    // Ensure there is a colon following the case value
    if (!Tok.is(Token::colon)) {
        error();
        return nullptr;
    }
    advance(); // Consume ':'

    // Parse the body of the case
    llvm::SmallVector<AST *> body;
    while (!Tok.is(Token::KW_case) && !Tok.is(Token::KW_default) && !Tok.is(Token::r_brace)) {
        AST *stmt = parseProgram();//=====================================================================have to make a change
        if (!stmt) {
            error();
            return nullptr;
        }
        body.push_back(stmt);
    }

    // Return the case statement node
    return new CaseStmt(caseValue, body);
}


DefaultStmt *Parser::parseDefault() {
    // Ensure we start with 'default'
    if (!Tok.is(Token::KW_default)) {
        error();
        return nullptr;
    }
    advance(); // Consume 'default'

    // Ensure there is a colon following 'default'
    if (!Tok.is(Token::colon)) {
        error();
        return nullptr;
    }
    advance(); // Consume ':'

    // Parse the body of the default case
    llvm::SmallVector<AST *> body;
    while (!Tok.is(Token::KW_case) && !Tok.is(Token::r_brace)) {
        AST *stmt = parseProgram();
        if (!stmt) {
            error();
            return nullptr;
        }
        body.push_back(stmt);
    }

    // Return the default case node
    return new DefaultStmt(body);
}



llvm::SmallVector<AST *> Parser::getBody()
{
    llvm::SmallVector<AST *> body;
    while (!Tok.is(Token::r_brace))
    {
        switch (Tok.getKind())
        {
        
        case Token::ident:{
            Token prev_token = Tok;
            const char* prev_buffer = Lex.getBuffer();
            UnaryOp *u;
            u = parseUnary();
            if (Tok.is(Token::semicolon))
            {
                if (u)
                {
                    body.push_back(u);
                    break;
                }
                else{

                    goto _error;
                }
                    
            }
            else
            {
                if (u)
                {

                    goto _error;
                }
                else{
                    Tok = prev_token;
                    Lex.setBufferPtr(prev_buffer);
                }
                    
            }

            
            Assignment *a_int;
            Assignment *a_bool;
            prev_token = Tok;
            prev_buffer = Lex.getBuffer();

            a_bool = parseBoolAssign();

            if (a_bool){
                body.push_back(a_bool);
                break;
            }
            Tok = prev_token;
            Lex.setBufferPtr(prev_buffer);

            a_int = parseIntAssign();
            if (a_int)
                body.push_back(a_int);
            else
                goto _error;
            if (!Tok.is(Token::semicolon))
            {
                goto _error;
            }

            break;
        }
        case Token::KW_if: {
            IfStmt *i;
            i = parseIf();
            if (i)
                body.push_back(i);
            else
                goto _error;
            
            break;
        }
        case Token::KW_while:{
            WhileStmt *w;
            w = parseWhile();
            if (w)
                body.push_back(w);
            else {
                goto _error;
            }
            break;
        }
        case Token::KW_for:{
            ForStmt *f;
            f = parseFor();
            if (f)
                body.push_back(f);
            else {
                goto _error;
            }
            break;
        }
        case Token::KW_print: {
            PrintStmt *p;
            p = parsePrint();
            if (p)
                body.push_back(p);
            else {
                goto _error;
            }
            break;
        }
        case Token::start_comment: {
            parseComment();
            if (!Tok.is(Token::end_comment))
                goto _error;
            break;
        }
        default:{
            error();

            goto _error;
            break;
        }
        }
        advance();

    }
    if(Tok.is(Token::r_brace)){
        return body;
    }

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return body;

}
