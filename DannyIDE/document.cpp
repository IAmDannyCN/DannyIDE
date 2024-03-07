#include "Document.h"
#include <qobject.h>
#define DEBUG

int q_pow(int a, int n)
{
    int ans=1;
    while(n)
    {
        if(n&1) ans*=a;
        a*=a, n>>=1;
    }
    return ans;
}

#ifdef DEBUG
std::map<int, std::string> ty_list;
void InitTyList()
{
    ty_list[ty_constant]="constant", ty_list[ty_variant]="variant";
    ty_list[ty_assign]="=", ty_list[ty_ret]="[ret]", ty_list[ty_eof]="[EOF]";
    ty_list[ty_lbr]="(", ty_list[ty_rbr]=")", ty_list[ty_output]="[output]";
    ty_list[ty_add]="+", ty_list[ty_minus]="-", ty_list[ty_mul]="*", ty_list[ty_div]="/", ty_list[ty_pow]="**", ty_list[ty_mod]="%";
    ty_list[ty_g]=">", ty_list[ty_geq]=">=", ty_list[ty_l]="<", ty_list[ty_leq]="<=", ty_list[ty_eq]="==", ty_list[ty_neq]="!=";
    ty_list[ty_ques]="?", ty_list[ty_col]=":";
    ty_list[ty_booland]="&&", ty_list[ty_boolor]="||", ty_list[ty_boolnot]="!";
    ty_list[ty_and]="&", ty_list[ty_or]="|", ty_list[ty_xor]="^", ty_list[ty_not]="~";
    ty_list[ty_at]="[@]", ty_list[ty_goto]="[goto]", ty_list[ty_gotoif]="[gotoif]";
}
#endif

std::map<char, int> SingleList;
void InitSingleList()
{
    SingleList['\n']=ty_ret, SingleList['(']=ty_lbr, SingleList[')']=ty_rbr;
    SingleList['+']=ty_add, SingleList['-']=ty_minus, SingleList['/']=ty_div, SingleList['%']=ty_mod;
    SingleList['?']=ty_ques, SingleList[':']=ty_col;
    SingleList['~']=ty_not;
    SingleList['^']=ty_xor;
    SingleList['@']=ty_at;
}

std::map<std::string, int> rg;
std::map<std::string, int> labelpos;
std::string ErrorOutput;
bool FoundError;
int GotoErrorLine;
bool GetFoundError()
{
    return FoundError;
}
void Terminate(std::string s)
{
    ErrorOutput=s;
    FoundError=1;
    return ;
}
int mem_index;
int line_index;

#define Status_OK -1
#define Status_Empty 0

int Status;
std::string UpdateCheckOutput(int ty)
{
    if(FoundError)
    {
        if(ty==0) Status=line_index+1;
        if(ty==1) Status=GotoErrorLine;
        return "Line "+std::to_string(Status)+": " +ErrorOutput;
    }
    else if(mem_index==0)
    {
        Status= Status_Empty;
        return "Input code to continue.";
    }
    else
    {
        Status= Status_OK;
        return "OK.";
    }
}

int GetStatus()
{
    return Status;
}

//=========Token=========
Token::Token(int t)
{
    ty=t, val=0, s="";
}
Token::Token(int t, int tval)
{
    assert(t==0);
    ty=t, val=tval, s="";
}
Token::Token(int t, std::string ts)
{
    assert(t==1);
    ty=t, val=0, s=ts;
}
Token::Token()
{
    ty=-1, val=0, s="";
}

//#ifdef DEBUG
//void Token::Out()
//{
//    std::cout<<ty_list[ty];
//    if(ty==0) std::cout<<'<'<<val<<'>';
//    if(ty==1) std::cout<<'\"'<<s<<'\"';
//    std::cout<<' ';
//}
//#endif
//=========Token=========

std::string Passage;
int PassagePos;
void SetPassage(std::string curPassage)
{
    Passage= curPassage + '\n' + (char)EOF;
    PassagePos= 0;
}
std::string GetPassage()
{
    return Passage;
}

//=========Lexer=========
char buff=' ';
char lasbuff=' ';

char Lexer::getnextchar()
{
    if(PassagePos < (int)Passage.size())
        return Passage[PassagePos++];
    else
        return EOF;
}

void Lexer::GetToken()
{
    while(buff==' ' || buff=='\t') buff=getnextchar();
    if(buff=='\n') line_index++;

    if(buff==-1)
    { t.ty=ty_eof; return ; }

    if(CheckSingle())
    { buff=getnextchar(); return ; }

    if(isdigit(buff))
    {
        t.ty=ty_constant;
        while(isdigit(buff))
            t.val=t.val*10+(buff-'0'), buff=getnextchar();
        return ;
    }
    if(isalpha(buff))
    {
        t.ty=ty_variant;
        while(isalpha(buff) || isdigit(buff))
            t.s+=buff, buff=getnextchar();
        if(t.s=="output") t.ty=ty_output, t.s="";
        if(t.s=="goto") t.ty=ty_goto, t.s="";
        if(t.s=="gotoif") t.ty=ty_gotoif, t.s="";
        return ;
    }

    if(CheckDouble('=',ty_assign, '=',ty_eq)) return ;
    if(CheckDouble('!',ty_boolnot, '=',ty_neq)) return ;
    if(CheckDouble('>',ty_g, '=',ty_geq)) return ;
    if(CheckDouble('<',ty_l, '=',ty_leq)) return ;
    if(CheckDouble('*',ty_mul, '*',ty_pow)) return ;
    if(CheckDouble('&',ty_and, '&',ty_booland)) return ;
    if(CheckDouble('|',ty_or, '|',ty_boolor)) return ;

    Terminate((std::string)"Lexer Error - undefined content '"+buff+"' .");
    return ;
}

inline bool Lexer::isalpha(char &c) const
{ return (c>='a' && c<='z') || (c>='A' && c<='Z'); }

bool Lexer::CheckSingle() // Check whether the operand must be consisted of a single char
{
    if(SingleList[buff])
    { t.ty=SingleList[buff]; return 1; }
    return 0;
}
bool Lexer::CheckDouble(const char fir, const int fir_ty, const char sec, const int sec_ty) // Lex when already known the operand consists of two chars
{
    if(buff!=fir) return 0;

    buff=getnextchar();
    if(buff==sec)
        t.ty=sec_ty, buff=getnextchar();
    else t.ty=fir_ty;
    return 1;
}
//=========Lexer=========

//=========Expression=========
    ConstantExpression::ConstantExpression(Token pre_t)
    { t=pre_t; }
    ConstantExpression::ConstantExpression(int pre_val)
    { t.ty=0, t.val=pre_val; }
    void ConstantExpression::Getval()
    { val=t.val; }

    VariantExpression::VariantExpression(Token pre_t)
    { t=pre_t; }
    VariantExpression::VariantExpression(std::string pre_var)
    { t.ty=1, t.s=pre_var; }
    void VariantExpression::Getval()
    { val=rg[t.s]; }

    void ArithmeticExpression::Getval()
    {
        lexp->Getval(); rexp->Getval();

        if(exp_ty==ty_add)   val=(lexp->val) + (rexp->val);
        if(exp_ty==ty_minus) val=(lexp->val) - (rexp->val);
        if(exp_ty==ty_mul)   val=(lexp->val) * (rexp->val);
        if(exp_ty==ty_div)
        {
            assert((rexp->val)!=0);
            if((rexp->val)==0) val=0;
            else val=(lexp->val) / (rexp->val);
        }
        if(exp_ty==ty_pow)   val=q_pow((lexp->val), (rexp->val));
        if(exp_ty==ty_mod)
        {
            assert((rexp->val)!=0);
            if((rexp->val)==0) val=0;
            else val=(lexp->val) % (rexp->val);
        }

        if(exp_ty==ty_l)   val=(lexp->val) < (rexp->val);
        if(exp_ty==ty_leq) val=(lexp->val) <= (rexp->val);
        if(exp_ty==ty_g)   val=(lexp->val) > (rexp->val);
        if(exp_ty==ty_geq) val=(lexp->val) >= (rexp->val);
        if(exp_ty==ty_eq)  val=(lexp->val) == (rexp->val);
        if(exp_ty==ty_neq) val=(lexp->val) != (rexp->val);

        if(exp_ty==ty_booland) val=(lexp->val) && (rexp->val);
        if(exp_ty==ty_boolor)  val=(lexp->val) || (rexp->val);

        if(exp_ty==ty_and) val=(lexp->val) & (rexp->val);
        if(exp_ty==ty_or)  val=(lexp->val) | (rexp->val);
        if(exp_ty==ty_xor) val=(lexp->val) ^ (rexp->val);
    }

    void TernaryExpression::Getval()
    {
        lexp->Getval();
        if(lexp->val)
        { mexp->Getval(); val=mexp->val; }
        else
        { rexp->Getval(); val=rexp->val; }
    }

    void MonocularExpression::Getval()
    {
        sexp->Getval();

        if(exp_ty==ty_not)     val= ~(sexp->val);
        if(exp_ty==ty_boolnot) val= !(sexp->val);
    }

//=========Expression=========

//=========Handle=========
Handle::Handle()
{
    h.clear();
    memset(bracket_sum,0,sizeof(bracket_sum));
    memset(ternary_sum,0,sizeof(ternary_sum));
}

void Handle::GetHandle()
{
    while(1)
    {
        Lexer L;
        L.GetToken();
        if(FoundError) return ;
        if(L.t.ty==ty_ret)
        {
            if(h.empty()) continue;
            return ;
        }
        if(L.t.ty==ty_eof)
        {
            return ;
        }
        h.push_back(L.t);
    }
}

int Handle::Judger()
{
    if(h[0].ty==ty_output)
    {
        if((int)h.size()<=1) Terminate("Judger Error - Required expression for output.");
        if(FoundError) return -1;

        return exp_ty=exp_output;
    }

    if(h[0].ty==ty_gotoif)
    {
        if((int)h.size()<=2) Terminate("Judger Error - Required a label name and an expression for gotoif.");
        if(FoundError) return -1;

        if(h[1].ty!=ty_variant) Terminate("Judger Error - Illegal label name.");
        if(FoundError) return -1;

        return exp_ty=exp_gotoif;
    }

    if(h[0].ty==ty_goto)
    {
        if((int)h.size()<=1) Terminate("Judger Error - Required a label name after goto.");
        if(FoundError) return -1;

        if((int)h.size()>=3) Terminate("Judger Error - Label name should contain only one word.");
        if(FoundError) return -1;

        if(h[1].ty!=ty_variant) Terminate("Judger Error - Illegal label name.");
        if(FoundError) return -1;

        return exp_ty=exp_goto;
    }

    if(h[0].ty==ty_at)
    {
        if((int)h.size()<=1) Terminate("Judger Error - Required a label name for label.");
        if(FoundError) return -1;

        if((int)h.size()>=3) Terminate("Judger Error - Label name should contain only one word.");
        if(FoundError) return -1;

        if(h[1].ty!=ty_variant) Terminate("Judger Error - Illegal label name.");
        if(FoundError) return -1;

        return exp_ty=exp_at;
    }

    if((int)h.size()>=2 && h[0].ty==ty_variant && h[1].ty==ty_assign)
    {
        if((int)h.size()<=2) Terminate("Judger Error - Required expression for assignment.");
        if(FoundError) return -1;

        return exp_ty=exp_assign;
    }

    Terminate("Judger Error - undefined handle.");
    return 0x1f1e33;
}

void Handle::BuildBracket() // Build : bracket_mp & bracket_sum
{
    std::stack<int> lpos;
    for(int i=0; i<(int)h.size(); i++)
    {
        if(i) bracket_sum[i]=bracket_sum[i-1];

        Token t=h[i];
        if(t.ty==ty_lbr)
        {
            bracket_sum[i]++;
            lpos.push(i);
        }
        if(t.ty==ty_rbr)
        {
            bracket_sum[i]--;
            if(lpos.empty()) Terminate("Expression Error - invalid bracket matching.");
            if(FoundError) return ;
            bracket_mp[lpos.top()]=i, bracket_mp[i]=lpos.top();
            lpos.pop();
        }
    }
    if(!lpos.empty()) Terminate("Expression Error - invalid bracket matching.");
}

void Handle::BuildTernary() // Build : ternary_mp & ternary_sum
{
    std::stack<int> lpos;
    for(int i=0; i<(int)h.size(); i++)
    {
        if(i) ternary_sum[i]=ternary_sum[i-1];

        Token t=h[i];
        if(t.ty==ty_ques)
        {
            ternary_sum[i]++;
            lpos.push(i);
        }
        if(t.ty==ty_col)
        {
            ternary_sum[i]--;
            if(lpos.empty()) Terminate("Expression Error - invalid ternary matching.");
            if(FoundError) return ;
            ternary_mp[lpos.top()]=i, ternary_mp[i]=lpos.top();
            lpos.pop();
        }
    }
    if(!lpos.empty()) Terminate("Expression Error - invalid ternary matching.");
}

void Handle::PreBuild()
{
    BuildBracket();
    if(FoundError) return ;
    BuildTernary();
}

Expression *Handle::ConvertExpression(int l, int r, int RECURSION_LEVEL)
{
    while(h[l].ty==ty_lbr && bracket_mp[l]==r)
        l++, r--;

//#ifdef DEBUG
//    std::cout<<std::setw(RECURSION_LEVEL*3)<<">> "<<l<<" "<<r<<": ";
//    for(int i=l; i<=r; i++) h[i].Out();
//    std::cout<<std::endl;
//#endif

    if(l==r)
    {
        if(h[l].ty==ty_constant)
        {
            ConstantExpression *son= new ConstantExpression(h[l]);
            return son;
        }
        else if(h[l].ty==ty_variant)
        {
            VariantExpression *son= new VariantExpression(h[l]);
            return son;
        }
    }

    if(l>=r)
        Terminate("Expression Error - expected expression between '(' and ')' or between oprands.");
    if(FoundError) return nullptr;

    int ques_pos=-1, addminus_pos=-1, muldivmod_pos=-1,
        comp_pos=-1, andorxor_pos=-1, not_pos=-1, pow_pos=-1;

    int base_bracket_sum=bracket_sum[l]-(h[l].ty==ty_lbr);
    for(int i=l; i<=r; i++)
    {
        if(bracket_sum[i]!=base_bracket_sum) continue;

        if(ques_pos==-1 && h[i].ty==ty_ques) ques_pos=i;

        if(pow_pos==-1 && h[i].ty==ty_pow) pow_pos=i;

        if(addminus_pos==-1 &&
            (h[i].ty==ty_add || h[i].ty==ty_minus) ) addminus_pos=i;

        if(muldivmod_pos==-1 &&
            (h[i].ty==ty_mul || h[i].ty==ty_div || h[i].ty==ty_mod) ) muldivmod_pos=i;

        if(comp_pos==-1 &&
            (h[i].ty==ty_l || h[i].ty==ty_leq ||
             h[i].ty==ty_g || h[i].ty==ty_geq ||
             h[i].ty==ty_eq || h[i].ty==ty_neq) ) comp_pos=i;

        if(andorxor_pos==-1 &&
            (h[i].ty==ty_and || h[i].ty==ty_or ||
             h[i].ty==ty_booland || h[i].ty==ty_boolor || h[i].ty==ty_xor) ) andorxor_pos=i;

        if(not_pos==-1 && (h[i].ty==ty_not || h[i].ty==ty_boolnot) ) not_pos=i;
    }

    if(ques_pos!=-1)
    {
        int col_pos=ternary_mp[ques_pos];
        if(!(l<ques_pos && ques_pos+1<col_pos && col_pos<r))
            Terminate("Expression Error - invalid ternary sequence.");
        if(FoundError) return nullptr;

        TernaryExpression *son= new TernaryExpression;
        son->lexp= ConvertExpression(l, ques_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->mexp= ConvertExpression(ques_pos+1, col_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(col_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;

        return son;
    }
    else if(andorxor_pos!=-1)
    {
        if(!(andorxor_pos>l && andorxor_pos<r))
            Terminate("Expression Error - Expected logic unit located between expressions.");
        if(FoundError) return nullptr;

        ArithmeticExpression *son= new ArithmeticExpression;
        son->lexp= ConvertExpression(l, andorxor_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(andorxor_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[andorxor_pos].ty;

        return son;
    }
    else if(comp_pos!=-1)
    {
        if(!(comp_pos>l && comp_pos<r))
            Terminate("Expression Error - Expected comparing unit located between expressions.");
        if(FoundError) return nullptr;

        ArithmeticExpression *son= new ArithmeticExpression;
        son->lexp= ConvertExpression(l, comp_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(comp_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[comp_pos].ty;

        return son;
    }
    else if(addminus_pos!=-1)
    {
        if(!(addminus_pos>l && addminus_pos<r))
            Terminate("Expression Error - Expected '+' or '-' located between expressions.");
        if(FoundError) return nullptr;

        ArithmeticExpression *son= new ArithmeticExpression;
        son->lexp= ConvertExpression(l, addminus_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(addminus_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[addminus_pos].ty;

        return son;
    }
    else if(muldivmod_pos!=-1)
    {
        if(!(muldivmod_pos>l && muldivmod_pos<r))
            Terminate("Expression Error - Expected '*' or '/' or '%' located between expressions.");
        if(FoundError) return nullptr;

        ArithmeticExpression *son= new ArithmeticExpression;
        son->lexp= ConvertExpression(l, muldivmod_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(muldivmod_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[muldivmod_pos].ty;

        return son;
    }
    else if(pow_pos!=-1)
    {
        if(!(pow_pos>l && pow_pos<r))
            Terminate("Expression Error - Expected '**' located between expressions.");
        if(FoundError) return nullptr;

        ArithmeticExpression *son= new ArithmeticExpression;
        son->lexp= ConvertExpression(l, pow_pos-1, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->rexp= ConvertExpression(pow_pos+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[pow_pos].ty;

        return son;
    }
    else if(not_pos!=-1)
    {
        if(not_pos!=l)
            Terminate("Expression Error - Expected '!' or '~' located at the beginning of a subexpression.");
        if(FoundError) return nullptr;

        MonocularExpression *son= new MonocularExpression;
        son->sexp= ConvertExpression(l+1, r, RECURSION_LEVEL+1); if(FoundError) return nullptr;
        son->exp_ty=h[not_pos].ty;

        return son;
    }

    Terminate("Expression Error - invalid expression.");
    return nullptr;
}
//=========Handle=========

//=========Memory=========
Memory::Memory(Handle cur_h, Expression *cur_root)
{
    for(const auto &tk: cur_h.h) h.push_back(tk);
    root=cur_root;
    exp_ty=cur_h.exp_ty;
}
Memory::Memory(Handle cur_h)
{
    for(const auto &tk: cur_h.h) h.push_back(tk);
    root=nullptr;
    exp_ty=cur_h.exp_ty;
}
Memory::Memory()
{
    h.clear();
    root=nullptr;
}

Memory mem[MAXN];
//=========Memory=========

void Check()
{
    InitSingleList();
    InitTyList();
    mem_index=0; line_index=0;
    rg.clear(); labelpos.clear();
    buff=' ';
    FoundError=0;

    while(1)
    {
        Handle H; H.GetHandle();
        line_index--;
        if(FoundError) return ;

        if(H.h.empty()) // If this expression is EOF
        {
            return ;
        }

        int exp_ty=H.Judger(), start_pos=-1, end_pos=-1;
        if(FoundError) return ;

        if(exp_ty==exp_at)
        {
            std::string labelname= H.h[1].s;
            if(labelpos[labelname]!=0) Terminate("Label Error - Redefinition of label '"+labelname+"'.");
            if(FoundError) return ;

            mem_index++;
            mem[mem_index].exp_ty= exp_at;
            mem[mem_index].h= H.h;
            mem[mem_index].line_number=line_index+1;
            labelpos[labelname]= mem_index;

            line_index++;
            continue;
        }
        if(exp_ty==exp_goto)
        {
            mem_index++;
            mem[mem_index].exp_ty= exp_goto;
            mem[mem_index].h= H.h;
            mem[mem_index].line_number=line_index+1;

            line_index++;
            continue;
        }

        if(exp_ty==exp_assign)
            start_pos=2, end_pos=H.h.size()-1;
        else if(exp_ty==exp_output)
            start_pos=1, end_pos=H.h.size()-1;
        else if(exp_ty==exp_gotoif)
            start_pos=2, end_pos=H.h.size()-1;

        H.PreBuild();
        if(FoundError) return ;

        Expression *root= H.ConvertExpression(start_pos, end_pos, 0);
        if(FoundError) return ;

        mem[++mem_index]=Memory(H, root);
        mem[mem_index].line_number=line_index+1;

        line_index++;
    }
}

void CheckGoto()
{
    for(int i=1; i<=mem_index; i++)
    {
        if(mem[i].exp_ty!=exp_goto && mem[i].exp_ty!=exp_gotoif) continue;

        if(labelpos[mem[i].h[1].s]==0)
        {
            Terminate("Goto Error - Undefined target label '"+mem[i].h[1].s+"'.");
            GotoErrorLine= mem[i].line_number;
        }
        if(FoundError) return ;
    }
}

//Height and line number
std::string MakeLineNumber(std::vector<int> height)
{
    std::string ans="";
    for(int i=0; i<(int)height.size(); i++)
    {
        ans+=std::to_string(i+1);
        for(int j=1; j<=height[i]; j++)
            ans+="\n";
    }
    return ans;
}

//Go and Undo
int curLine;
void Set_curLine(int tar)
{
    curLine=tar;
}

std::stack<std::pair<int, std::pair<std::string, int> > > ChangeHistory;//(preLine, (varname,varval))
void InitChangeHistory()
{
    while(!ChangeHistory.empty())
        ChangeHistory.pop();
}

std::pair<int, std::pair<std::string,int> > Go() //highlight_line_index, (changed_variable_name, val_after_change)
{
    int preLine= curLine;
    if(mem[curLine].exp_ty==exp_goto)
    {
        curLine=labelpos[mem[curLine].h[1].s];
    }
    else if(mem[curLine].exp_ty==exp_gotoif)
    {
        bool gotojudge= mem[curLine].root->val;
        if(gotojudge)
            curLine=labelpos[mem[curLine].h[1].s];
        else
        {
            if(curLine==mem_index)
                return std::make_pair(-1, std::make_pair("[t]", -1));
            curLine++;
        }
    }
    else
    {
        if(curLine==mem_index)
            return std::make_pair(-1, std::make_pair("[t]", -1));
        curLine++;
    }

    if(mem[curLine].exp_ty==exp_at)
    {
        ChangeHistory.push(std::make_pair(preLine, std::make_pair("[empty]", -1)));
        return std::make_pair(mem[curLine].line_number, std::make_pair("[@]", -1));
    }
    if(mem[curLine].exp_ty==exp_goto)
    {
        ChangeHistory.push(std::make_pair(preLine, std::make_pair("[empty]", -1)));
        return std::make_pair(mem[curLine].line_number, std::make_pair("[g]", -1));
    }

    mem[curLine].root->Getval();

    if(mem[curLine].exp_ty==exp_assign)
    {
        ChangeHistory.push(std::make_pair(preLine, std::make_pair(mem[curLine].h[0].s, rg[mem[curLine].h[0].s])));

        rg[mem[curLine].h[0].s]= mem[curLine].root->val;
        return std::make_pair(mem[curLine].line_number, std::make_pair(mem[curLine].h[0].s, mem[curLine].root->val));
    }
    else if(mem[curLine].exp_ty==exp_output)
    {
        ChangeHistory.push(std::make_pair(preLine, std::make_pair("[empty]", -1)));

        return std::make_pair(mem[curLine].line_number, std::make_pair("[o]", mem[curLine].root->val));
    }
    else if(mem[curLine].exp_ty==exp_gotoif)
    {
        ChangeHistory.push(std::make_pair(preLine, std::make_pair("[empty]", -1)));

        return std::make_pair(mem[curLine].line_number, std::make_pair("[g]", mem[curLine].root->val));
    }

    return std::make_pair(-1, std::make_pair("[e]", -1));
}

std::pair<std::string, int> Undo()
{
    if(ChangeHistory.empty())
        return std::make_pair("[s]", -1);

    int tarLine= ChangeHistory.top().first;
    std::string varname= ChangeHistory.top().second.first;
    int varval= ChangeHistory.top().second.second;
    ChangeHistory.pop();

    if(varname!="[empty]")
        rg[varname]=varval;

    curLine=tarLine;

    return std::make_pair("[ok]", mem[curLine].line_number);
}

std::map<std::string, int> Getrg()
{
    return rg;
}

void Updaterg(std::map<std::string, int> currg)
{
    rg.clear();
    rg.merge(currg);
}

