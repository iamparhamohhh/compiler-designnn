#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

class Parser
{
    Lexer &Lex;    // retrieve the next token from the input
    Token Tok;     // stores the next token
    bool HasError; // indicates if an error was detected

    void error()
    {
        llvm::errs() << "Unexpected: " << Tok.getText() << Tok.getKind() << "\n";
        HasError = true;
    }

    // retrieves the next token from the lexer.expect()
    // tests whether the look-ahead is of the expected kind
    void advance() { Lex.next(Tok); }

    bool expect(Token::TokenKind Kind)
    {
        if (Tok.getKind() != Kind)
        {
            error();//========================================================================= comment out =================
            return true;
        }
        return false;
    }

    // retrieves the next token if the look-ahead is of the expected kind
    bool consume(Token::TokenKind Kind)
    {
        if (expect(Kind))
            return true;
        advance();
        return false;
    }

    Program *parseProgram();
    DeclarationInt *parseIntDec();
    DeclarationBool *parseBoolDec();
    Assignment *parseBoolAssign();
    Assignment *parseIntAssign();
    UnaryOp *parseUnary();
    Expr *parseExpr();
    Expr *parseTerm();
    Expr *parseFinal();
    Expr *parseFactor();
    Logic *parseLogic();
    Logic *parseComparison();
    IfStmt *parseIf();
    WhileStmt *parseWhile();
    ForStmt *parseFor();
    PrintStmt *parsePrint();
    SwitchStmt *parseSwitch();
    CaseStmt *parseCase();
    DefaultStmt *parseDefault();

    void parseComment();
    

    llvm::SmallVector<AST *> getBody();

public:
    // initializes all members and retrieves the first token
    Parser(Lexer &Lex) : Lex(Lex), HasError(false)
    {
        advance();
    }

    // get the value of error flag
    bool hasError() { return HasError; }

    Program *parse();
};

#endif