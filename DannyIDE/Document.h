#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "qtextedit.h"
#include <bits/stdc++.h>
#define DEBUG

#define bug std::cout<<"bug "<<__LINE__<<std::endl

#define	ty_constant 0
#define	ty_variant 1
#define	ty_assign 2
#define	ty_ret 3
#define	ty_eof 4
#define	ty_lbr 5
#define	ty_rbr 6
#define	ty_output 7

#define	ty_add 10
#define	ty_minus 11
#define	ty_mul 12
#define	ty_div 13
#define ty_pow 14
#define ty_mod 15

#define	ty_g 20
#define	ty_geq 21
#define	ty_l 22
#define	ty_leq 23
#define	ty_eq 24
#define	ty_neq 25

#define	ty_ques 30
#define	ty_col 31

#define ty_booland 40
#define ty_boolor 41
#define ty_boolnot 42
#define ty_and 43
#define ty_or 44
#define ty_xor 45
#define ty_not 46

#define ty_at 47
#define ty_goto 48
#define ty_gotoif 49

int q_pow(int a, int n);

#ifdef DEBUG
void InitTyList();
#endif

#define exp_output 0
#define exp_assign 1
#define exp_at 2
#define exp_goto 3
#define exp_gotoif 4

const int MAXN=100050;

void InitSingleList();

bool GetFoundError();
void Terminate(std::string s);
std::string UpdateCheckOutput(int ty);
int GetStatus();

class Token
{
public:
    Token(int t);
    Token(int t, int tval);
    Token(int t, std::string ts);
    Token();

    int ty;

    int val; // for constant
    std::string s;// for variant

#ifdef DEBUG
    void Out();
#endif
};

void SetPassage(std::string curPassage);
std::string GetPassage();

class Lexer
{
public:
    Token t;
    char getnextchar();
    void GetToken();

private:
    inline bool isalpha(char &c) const;
    bool CheckSingle();
    bool CheckDouble(const char fir, const int fir_ty, const char sec, const int sec_ty);
};

class Expression
{
public:
    int val;
    virtual void Getval() =0;
};

    class ConstantExpression: public Expression
    {
    public:
        ConstantExpression(Token pre_t);
        ConstantExpression(int pre_val);

        Token t;
        void Getval();
    };
    class VariantExpression: public Expression
    {
    public:
        VariantExpression(Token pre_t);
        VariantExpression(std::string pre_var);

        Token t;
        void Getval();
    };
    class ArithmeticExpression: public Expression
    {
    public:
        Expression *lexp, *rexp;
        int exp_ty;

        void Getval();
    };
    class TernaryExpression: public Expression
    {
    public:
        Expression *lexp, *mexp, *rexp;
        void Getval();
    };
    class MonocularExpression: public Expression
    {
    public:
        Expression *sexp;
        int exp_ty;

        void Getval();
    };

class Handle
{
public:
    Handle();

    std::deque<Token> h;
    int exp_ty;

    std::map<int,int> bracket_mp; // The position of the other part of the bracket
    int bracket_sum[MAXN]; // The level of bracket. +1 when '(', -1 when ')'

    std::map<int,int> ternary_mp; // The position of the other part of '?' or ':'
    int ternary_sum[MAXN]; // The level of ternary. +1 when '?', -1 when ':'

    void GetHandle();
    int Judger();
    void BuildBracket();
    void BuildTernary();
    void PreBuild();
    Expression *ConvertExpression(int l, int r, int RECURSION_LEVEL);
};

class Memory
{
public:
    void extracted(Handle &cur_h);
    Memory(Handle cur_h, Expression *cur_root);
    Memory(Handle cur_h);
    Memory();

    std::deque<Token> h;
    Expression *root;
    int exp_ty;

    int line_number;
};

void Check();
void CheckGoto();

//Height and line number
std::string MakeLineNumber(std::vector<int> height);

//Go and undo
void InitChangeHistory();
void Set_curLine(int tar);
std::pair<int, std::pair<std::string,int> > Go(); //highlight_line_index, (changed_variable_name, val_after_change)
std::pair<std::string, int> Undo();

std::map<std::string, int> Getrg();
void Updaterg(std::map<std::string, int> currg);

#endif // DOCUMENT_H
