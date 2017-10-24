#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cradle.h"

void Term();
void Expression();
void Add();
void Substract();
void Factor();
void Ident();
void Assignment();

void GetChar() 
{
    Look = getchar();
}


void Error(char *s)
{
    printf("\nError: %s.", s);
}

void Abort(char *s)
{
    Error(s);
    exit(1);
}


void Expected(char *s)
{
    sprintf(tmp, "%s Expected", s);
    Abort(tmp);
}


void Match(char x)
{
    if(Look == x) {
        GetChar();
        SkipWhite();
    } else {
        sprintf(tmp, "' %c ' ",  x);
        Expected(tmp);
    }
}


int IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
} 

int IsDigit(char c)
{
    return (c >= '0') && (c <= '9');
}

int IsAlNum(char c)
{
    return IsAlpha(c) || IsDigit(c);
}

int IsAddop(char c)
{
    return (c == '+') || (c == '-');
}

int IsWhite(char c)
{
    return (c == ' ') || (c == '\t');
}

char* GetName()
{
    char *token = token_buf;

    if( !IsAlNum(Look)) {
        Expected("Name");
    }
    while (IsAlNum(Look)) {
        *token = Look;
        token++;

        GetChar();
    }

    SkipWhite();

    *token = '\0';
    return token_buf;
}


char* GetNum()
{
    char *value = token_buf;

    if( !IsAlNum(Look)) {
        Expected("Integer");
    }
    while (IsDigit(Look)) {
        *value = Look;
        value++;

        GetChar();
    }

    SkipWhite();

    *value = '\0';
    return token_buf;
}

void SkipWhite()
{
    while (IsWhite(Look)) {
        GetChar();
    }
}

void Emit(char *s)
{
    printf("\t%s", s);
}

void EmitLn(char *s)
{
    Emit(s);
    printf("\n");
}

void Init()
{
    GetChar();
    SkipWhite();
}
void Multiply()
{
    Match('*');
    Factor();
    EmitLn("imull (%esp), %eax");
    /* push of the stack */
    EmitLn("addl $4, %esp");
} 

void Divide()
{
    Match('/');
    Factor();

    EmitLn("movl (%esp), %edx");
    EmitLn("addl $4, %esp");

    EmitLn("pushl %eax");

    EmitLn("movl %edx, %eax");

    /* sign extesnion */
    EmitLn("sarl $31, %edx");
    EmitLn("idivl (%esp)");
    EmitLn("addl $4, %esp");

}

void Ident()
{
    char *name = GetName();
    if (Look == '(') {
        Match('(');
        Match(')');
        sprintf(tmp, "call %s", name);
        EmitLn(tmp);
    } else {
        sprintf(tmp, "movl %s, %%eax", name);
        EmitLn(tmp);
    }
}

void Factor()
{
    if(Look == '(') {
        Match('(');
        Expression();
        Match(')');
     } else if(IsAddop(Look)) {
        Match('-');
        sprintf(tmp,"movl $%s, %%eax", GetNum());
        EmitLn(tmp);
        EmitLn("negl %eax");
    } else if (IsAlpha(Look)) {
        Ident();
    } else {
        sprintf(tmp,"movl $%s, %%eax", GetNum());
        EmitLn(tmp);
    }
}

void Term()
{
    Factor();
    while (strchr("*/", Look)) {

        EmitLn("pushl %eax");

        switch(Look)
        {
            case '*':
                Multiply();
                break;
            case '/':
                Divide();
                break;
            default:
                Expected("Mulop");
        }
    }
}

void Expression()
{
    if(IsAddop(Look))
        EmitLn("xor %eax, %eax");
    else
        Term();

    while (strchr("+-", Look)) {

        EmitLn("pushl %eax");

        switch(Look)
        {
            case '+':
                Add();
                break;
            case '-':
                Substract();
                break;
            default:
                Expected("Addop");
        }
    }
}


void Add()
{
    Match('+');
    Term();
    EmitLn("addl (%esp), %eax");
    EmitLn("addl $4, %esp");
    
}


void Substract()
{
    Match('-');
    Term();
    EmitLn("subl (%esp), %eax");
    EmitLn("negl %eax");
    EmitLn("addl $4, %esp");
}

void Assignment()
{
    char *name = GetName();
    Match('=');
    Expression();
    sprintf(tmp, "lea %s, %%ebx", name);
    EmitLn(tmp);
    EmitLn("movl %eax, (%ebx)");
}

int main()
{

    Init();
    EmitLn(".text");
    EmitLn(".global _start");
    EmitLn("_start:");
    /* Expression(); */
    Assignment();
    if (Look != '\n') {
        Expected("NewLine");
    }


    /* return the result */
    EmitLn("movl %eax, %ebx");
    EmitLn("movl $1, %eax");
    EmitLn("int $0x80");
    return 0;
}
