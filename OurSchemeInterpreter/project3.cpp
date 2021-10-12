# include <iostream>
# include <stdio.h>
# include <string>
# include <vector>
# include <stdlib.h>
# include <cstdlib>
# include <iomanip>
# include <exception>
# include <cmath>
# include <stack>
# include <sstream>

using namespace std;

int gLine = 0, gColumn = 0;

enum TokenType {
  EOF_, NONE, LP, RP, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, PROCEDURE
};

enum NodeType{
  ROOT, INTERNAL, LEAF
};

struct Token {
  TokenType type ;
  string str;
} ;

struct Tree {
  NodeType type;
  Token token;
  Tree * left;
  Tree * right;
};

struct Symbol {
  string name;
  Tree * exp;
};

struct Function {
  string name;
  string procedure;
  vector<Symbol> parameter;
  Tree * functionbody;
};

struct StackLevel {
  Function func;
  StackLevel * next;
  StackLevel * prev;
};

stack<Token> gUnDealToken;
stack<Function> gLambdaStack;
stack<StackLevel *> gCallStack;
vector<Function> gUserDefineFunction;

Tree * Eval( Tree * root, Tree * resultexpr, bool isTopLevel ) ;
Tree * IsList( Tree * root ) ;
Tree * IsPair( Tree * root ) ;
void Atom( bool & correct, Tree * node ) ;
void S_exp( bool & correct, Tree * node ) ;
void Preorder( Tree *current ) ;

bool IsWhiteSpace( char ch ) {
  if ( ch == ' ' || ch == '\t' || ch == '\n' ) {
    return true;
  } // if
  
  return false;
} // IsWhiteSpace()

bool IsSeparator( char ch ) {
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '(' || ch == ')' 
       || ch == '\'' || ch == '\"' || ch == ';' ) {
    return true;
  } // if
  
  return false;
} // IsSeparator()

void Initialize( Token & token ) {
  token.type = NONE;
  token.str = "";
} // Initialize()

void Initialize( Token & token, TokenType type, string str ) {
  token.type = type;
  token.str = str;
} // Initialize()

void Initialize( Tree * tree, NodeType type, Token token ) {
  tree->type = type;
  tree->token = token;
  tree->left = NULL;
  tree->right = NULL;
} // Initialize()

void Initialize( Tree * tree, NodeType type ) {
  tree->type = type;
  Initialize( tree->token );
  tree->left = NULL;
  tree->right = NULL;
} // Initialize()

char GetTheNextChar() {
  char ch = '\0';
  ch = cin.get();
  if ( ch == EOF ) {
    cout << "> ERROR (no more input) : END-OF-FILE encountered\n";
    cout << "Thanks for using OurScheme!";
    exit( 0 );
    throw 0;
  } // if
  else if ( ch == '\n' ) {
    ++gLine;
    gColumn = 0;
    return ch;
  } // else if
  else {
    ++gColumn;
    return ch;
  } // else 

} // GetTheNextChar()

char GetTheNextNonWhiteSpaceChar() {
  char ch = '\0', comment = '\0';
  ch = GetTheNextChar();
  while ( IsWhiteSpace( ch ) ) {
    ch = GetTheNextChar();
  } // while

  while ( ch == ';' ) {
    comment = cin.peek();   
    comment = '\0';
    ch = GetTheNextChar();
    while ( ch != '\n' ) { 
      ch = GetTheNextChar();
    } // while
      
    while ( IsWhiteSpace( ch ) ) {
      ch = GetTheNextChar();
    } // while

  } // while
    
  return ch;
} // GetTheNextNonWhiteSpaceChar()

Token GetToken() {
  char ch = '\0';
  Token token;
  Initialize( token );
  if ( !gUnDealToken.empty() ) {
    // 有尚未處理的token, 先拿尚未處理的token來用
    token = gUnDealToken.top();
    gUnDealToken.pop();
  } // if
  else {
    // 沒有尚未處理的token, 從input取資料 
    ch = GetTheNextNonWhiteSpaceChar();
    if ( ch == EOF ) {
      token.type = EOF_;
    } // if
    else if ( ch == '(' ) {
      token.type = LP;
      token.str = ch;
    } // else if
    else if ( ch == ')' ) {
      token.type = RP;
      token.str = ch;
    } // else if
    else if ( ch == '\'' ) {
      token.type = QUOTE;
      token.str = ch;
    } // else if
    else if ( ch == '\"' ) {
      token.str = ch;
      ch = cin.peek();
      while ( ch != '\"' ) {
        if ( ch == '\n' ) {
          if ( gLine == 0 ) {
            gLine = 1;  
          } // if
          
          cout << "> ERROR (no closing quote) : END-OF-LINE encountered at Line " 
          << gLine << " Column " << gColumn + 1 << "\n\n";
          ch = GetTheNextChar();
          throw 0;
        } // if
        else if ( ch == '\\' ) {
          ch = GetTheNextChar();
          char ch1 = cin.peek();
          if ( ch1 == '\"' ) {
            ch1 = GetTheNextChar();
            ch = '\"';
          } // if
          else if ( ch1 == '\\' ) {
            ch1 = GetTheNextChar();
            ch = '\\';
          } // if
          else if ( ch1 == 'n' ) {
            ch1 = GetTheNextChar();
            ch = '\n';
          } // if
          else if ( ch1 == 't' ) {
            ch1 = GetTheNextChar();
            ch = '\t';
          } // if
          
        } // else if
        else {
          ch = GetTheNextChar();
        } // else
        
        token.str += ch;
        ch = cin.peek();
      } // while
      
      if ( ch == '\"' ) {
        ch = GetTheNextChar();
      } // if
      
      token.str += ch;
      token.type = STRING;
    } // else if
    else if ( ( ch >= '0' && ch <= '9' ) ) {
      int dotCount = 0;
      bool isSymbol = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        if ( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
          if ( ch == '.' ) {
            dotCount++; 
          } // if
          
        } // if
        else {
          isSymbol = true;
        } // else
        
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
        
      if ( dotCount == 0 ) {
        token.type = INT;
      } // if
      else if ( dotCount == 1 ) {
        token.type = FLOAT;
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
      if ( isSymbol ) {
        token.type = SYMBOL;
      } // if
  
    } // else if
    else if ( ch == '#' ) {
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
      
      if ( token.str == "#f" ) {
        token.type = NIL;
      } // if
      else if ( token.str == "#t" ) {
        token.type = T;
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
    } // else if
    else if ( ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) ) {
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
      
      if ( token.str == "nil" ) {
        token.type = NIL;
      } // if
      else if ( token.str == "t" ) {
        token.type = T;
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
    } // else if
    else if ( ch == '+' ) {
      int dotCount = 0;
      bool isSymbol = false;
      bool hasNum = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        if ( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
          if ( ch == '.' ) {
            dotCount++; 
          } // if
          else {
            hasNum = true;
          } // else
    
        } // if
        else {
          isSymbol = true;
        } // else
      
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
        
      if ( dotCount == 0 ) {
        token.type = INT;
      } // if
      else if ( dotCount == 1 ) {
        if ( hasNum ) {
          token.type = FLOAT;
        } // if
        else {
          token.type = SYMBOL;
        } // else
        
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
      if ( token.str.length() == 1 ) { // only +
        isSymbol = true;
      } // if
      
      if ( isSymbol ) {
        token.type = SYMBOL;
      } // if
      
    } // else if
    else if ( ch == '-' ) {
      int dotCount = 0;
      bool isSymbol = false;
      bool hasNum = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        if ( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
          if ( ch == '.' ) {
            dotCount++; 
          } // if
          else {
            hasNum = true;
          } // else
          
        } // if
        else {
          isSymbol = true;
        } // else
        
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
        
      if ( dotCount == 0 ) {
        token.type = INT;
      } // if
      else if ( dotCount == 1 ) {
        if ( hasNum ) {
          token.type = FLOAT;
        } // if
        else {
          token.type = SYMBOL;
        } // else
        
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
      if ( token.str.length() == 1 ) { // only -
        isSymbol = true;
      } // if
      
      if ( isSymbol ) {
        token.type = SYMBOL;
      } // if
      
    } // else if
    else if ( ch == '.' ) {
      int dotCount = 1;
      bool isSymbol = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        if ( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
          if ( ch == '.' ) {
            dotCount++; 
          } // if
            
        } // if
        else {
          isSymbol = true;
        } // else
      
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
        
      if ( dotCount == 0 ) {
        token.type = INT;
      } // if
      else if ( dotCount == 1 ) {
        token.type = FLOAT;
      } // else if
      else {
        token.type = SYMBOL;
      } // else
      
      if ( token.str.length() == 1 ) { // only .
        isSymbol = false;
        token.type = DOT;
      } // if
      
      if ( isSymbol ) {
        token.type = SYMBOL;
      } // if
      
    } // else if
    else {
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        ch = GetTheNextChar();
        token.str += ch;
        ch = cin.peek();
      } // while
      
      token.type = SYMBOL;
    } // else
    
  } // else
  
  if ( gLine == 0 ) {
    gLine = 1;
  } // if
  
  return token;
} // GetToken()

void PeekToken() {
  gUnDealToken.push( GetToken() );
} // PeekToken()

string EnumToString( TokenType value ) {
  string table[] = { "EOF_", "NONE", "LP", "RP", "INT", "STRING"
                     , "DOT", "FLOAT", "NIL", "T", "QUOTE", "SYMBOL" };
  return table[value];
} // EnumToString()

// 總共有四個可能會出現的error：

//  ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
//  ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
//  ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
//  ERROR (no more input) : END-OF-FILE encountered


void S_exp( bool & correct, Tree * node ) {
  // <S-exp> ::= <ATOM> 
  //        | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
  //        | QUOTE <S-exp>
  // cout << "S_exp\n";
  Token token;
  correct = true;
  bool s_expCorrect = true;
  bool atomCorrect = true;
  Tree * current = node;
  if ( current == NULL ) {
    current = new Tree;
    Initialize( current, ROOT );
  } // if
  
  Atom( atomCorrect, node );
  if ( atomCorrect ) { // <ATOM>
    return;
  } // if
  
  PeekToken();
  
  if ( gUnDealToken.top().type == LP ) {
    token = GetToken();
    current->left = new Tree;
    Initialize( current->left, ROOT );
    S_exp( s_expCorrect, current->left );
    if ( !s_expCorrect ) {
      correct = false;
      token = GetToken();
      cout << "> ERROR (unexpected token) : atom or '(' expected when token at Line "
           << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
      throw 0;
    } // if

    while ( correct ) {
      Tree * lastCurrent = current;
      Tree*temp = new Tree;
      Initialize( temp, INTERNAL );
      current->right = temp;
      current = current->right;
      current->left = new Tree;
      Initialize( current->left, ROOT );
      S_exp( correct, current->left );
      
      if ( !correct ) {
        current = lastCurrent;
        lastCurrent->right = NULL;
      } // if

    } // while
    
    correct = true;
    PeekToken();
    
    if ( gUnDealToken.top().type == DOT ) {
      token = GetToken();
      current->right = new Tree;
      Initialize( current->right, INTERNAL );
      S_exp( s_expCorrect, current->right );
      if ( !s_expCorrect ) {
        correct = false;
        token = GetToken();
        cout << "> ERROR (unexpected token) : atom or '(' expected when token at Line "
             << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
        throw 0;
      } // if
      
    } // if
    else { // 補 NIL 
      current->right = new Tree;
      Token ttemp;
      Initialize( ttemp, NIL, "nil" );
      Initialize( current->right, LEAF, ttemp );
    } // else
    
    PeekToken();
    if ( gUnDealToken.top().type == RP ) {
      token = GetToken();
    } // if
    else {
      token = GetToken();
      cout << "> ERROR (unexpected token) : ')' expected when token at Line " << gLine 
      << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n" ;
      throw 0;
    } // else
  
  } // if
  else if ( gUnDealToken.top().type == QUOTE ) {

    token = GetToken();
    Tree * test = current;
    current->left = new Tree;
    Initialize( current->left, LEAF, token );
    
    current->right = new Tree;
    Initialize( current->right, INTERNAL );
    
    current->right->left = new Tree;
    Initialize( current->right->left, ROOT );
    
    S_exp( s_expCorrect, current->right->left );

    Token ttemp;
    Initialize( ttemp, NIL, "nil" );
    current->right->right = new Tree;
    Initialize( current->right->right, LEAF, ttemp );

    if ( !s_expCorrect ) {
      correct = false;
      token = GetToken();
      cout << "> ERROR (unexpected token) : "
           << "atom or '(' expected when token at Line "
           << gLine << " Column " << gColumn - token.str.size() + 1 
           << " is >>" << token.str << "<<\n\n"; // \n
      throw 0;
    } // if

    node = current;

  } // else if
  else {
    correct = false;
  } // else

} // S_exp()

bool Boolean( Tree * root ) {
  
  if ( root->left == NULL && root->right == NULL && root->token.type == NIL  ) {
    return false;
  } // if
  else {
    return true;
  } // else
  
} // Boolean()

void Atom( bool & correct, Tree * node ) {
  // <ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
  //        | NIL | T | LEFT-PAREN RIGHT-PAREN
  // cout << "Atom\n";
  Token token;
  correct = true;
  // Tree * temp = NULL;
  PeekToken();
  if ( gUnDealToken.top().type == SYMBOL || gUnDealToken.top().type == INT 
       || gUnDealToken.top().type == FLOAT || gUnDealToken.top().type == STRING 
       || gUnDealToken.top().type == NIL || gUnDealToken.top().type == T ) {
    token = GetToken();
    // temp = new Tree;
    Initialize( node, LEAF, token );
    // node = temp;
  } // if
  else if ( gUnDealToken.top().type == LP ) {
    token = GetToken();
    PeekToken();
    if ( gUnDealToken.top().type != RP ) {
      gUnDealToken.push( token );
      correct = false;
      return;
    } // if
    
    token = GetToken();
    token.type = NIL;
    token.str = "nil";
    // temp = new Tree;
    Initialize( node, LEAF, token );
    // node = temp;
  } // else if
  else {
    node = NULL;
    correct = false;
    return;
  } // else
  
} // Atom()

void PrintFormat( Tree * node ) {
  // <ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
  //        | NIL | T | LEFT-PAREN RIGHT-PAREN
  if ( node->token.type == INT ) {
    cout << atoi( node->token.str.c_str() );
  } // if
  else if ( node->token.type == STRING ) {
    cout << node->token.str;
  } // else if
  else if ( node->token.type == SYMBOL ) {
    cout << node->token.str;
  } // else if
  else if ( node->token.type == NIL ) {
    cout << "nil";
  } // else if
  else if ( node->token.type == T ) {
    cout << "#t";
  } // else if
  else if ( node->token.type == QUOTE ) {
    cout << "quote";
  } // else if
  else if ( node->token.type == FLOAT ) {
    cout << fixed << setprecision( 3 ) << atof( node->token.str.c_str() );
  } // else if
  else if ( node->token.type == PROCEDURE ) {
    cout << node->token.str;
  } // else if
  
  cout << "\n";
} // PrintFormat()

void PrintSexp( Tree * node, int m ) {
  bool isSexp = false;
  bool isQuote = false;
  Tree * current = node;
  if ( node == NULL ) {
    return;
  } // if
  
  if ( node->left == NULL && node->right == NULL ) {
    // 這是一個ATOM
    PrintFormat( node );
  } // if
  else {
    cout << "( ";
    m += 2 ;
    
    if ( current != NULL ) {
      if ( current->left != NULL ) {
        PrintSexp( current->left, m );
      } // if
    } // if
    
    while ( current->right != NULL ) {
      current = current -> right;
      if ( current->left != NULL ) {
        for ( int i = 0 ; i < m ; i++ ) {
          cout << " ";
        } // for
        
        PrintSexp( current->left, m );
      } // if
      
    } // while
    
    if ( current->token.type != NIL ) {
      for ( int i = 0 ; i < m ; i++ ) {
        cout << " ";
      } // for
      
      cout << ".\n";
      for ( int i = 0 ; i < m ; i++ ) {
        cout << " ";
      } // for
      
      PrintFormat( current );
    } // if
    
    m -= 2 ;
    for ( int i = 0 ; i < m ; i++ ) {
      cout << " ";
    } // for
    
    cout << ")\n";
  } // else
  
} // PrintSexp()

vector <Symbol> gSymbolTable;
vector <Symbol> gSystemFunctionTable;

void PutSymbolIntoSymbolTable( Symbol newSymbol, string funcName, string tokenStr ) {
  Tree * temp = new Tree;
  temp->left = NULL;
  temp->right = NULL;
  temp->type = ROOT;
  temp->token.str = tokenStr;
  temp->token.type = PROCEDURE;
  newSymbol.exp = temp;
  newSymbol.name = funcName;
  gSymbolTable.push_back( newSymbol );
  // internalFunctionTable.push_back(newSymbol);
  gSystemFunctionTable.push_back( newSymbol );
} // PutSymbolIntoSymbolTable()

void InitialEnvironment() {
  // #<procedure funcname>
  Tree * temp;
  Symbol newSymbol;
  gSymbolTable.clear();
  gSystemFunctionTable.clear();
  gUserDefineFunction.clear();
  PutSymbolIntoSymbolTable( newSymbol, "cons", "#<procedure cons>" );
  PutSymbolIntoSymbolTable( newSymbol, "list", "#<procedure list>" );
  PutSymbolIntoSymbolTable( newSymbol, "quote", "#<procedure quote>" );
  PutSymbolIntoSymbolTable( newSymbol, "\'", "#<procedure quote>" );
  PutSymbolIntoSymbolTable( newSymbol, "define", "#<procedure define>" );
  PutSymbolIntoSymbolTable( newSymbol, "car", "#<procedure car>" );
  PutSymbolIntoSymbolTable( newSymbol, "cdr", "#<procedure cdr>" );
  PutSymbolIntoSymbolTable( newSymbol, "atom?", "#<procedure atom?>" );
  PutSymbolIntoSymbolTable( newSymbol, "pair?", "#<procedure pair?>" );
  PutSymbolIntoSymbolTable( newSymbol, "list?", "#<procedure list?>" );
  PutSymbolIntoSymbolTable( newSymbol, "null?", "#<procedure null?>" );
  PutSymbolIntoSymbolTable( newSymbol, "integer?", "#<procedure integer?>" );
  PutSymbolIntoSymbolTable( newSymbol, "real?", "#<procedure real?>" );
  PutSymbolIntoSymbolTable( newSymbol, "number?", "#<procedure number?>" );
  PutSymbolIntoSymbolTable( newSymbol, "string?", "#<procedure string?>" );
  PutSymbolIntoSymbolTable( newSymbol, "boolean?", "#<procedure boolean?>" );
  PutSymbolIntoSymbolTable( newSymbol, "symbol?", "#<procedure symbol?>" );
  PutSymbolIntoSymbolTable( newSymbol, "+", "#<procedure +>" );
  PutSymbolIntoSymbolTable( newSymbol, "-", "#<procedure ->" );
  PutSymbolIntoSymbolTable( newSymbol, "*", "#<procedure *>" );
  PutSymbolIntoSymbolTable( newSymbol, "/", "#<procedure />" );
  PutSymbolIntoSymbolTable( newSymbol, "not", "#<procedure not>" );
  PutSymbolIntoSymbolTable( newSymbol, "and", "#<procedure and>" );
  PutSymbolIntoSymbolTable( newSymbol, "or", "#<procedure or>" );
  PutSymbolIntoSymbolTable( newSymbol, ">", "#<procedure >>" );
  PutSymbolIntoSymbolTable( newSymbol, ">=", "#<procedure >=>" );
  PutSymbolIntoSymbolTable( newSymbol, "<", "#<procedure <>" );
  PutSymbolIntoSymbolTable( newSymbol, "<=", "#<procedure <=>" );
  PutSymbolIntoSymbolTable( newSymbol, "=", "#<procedure =>" );
  PutSymbolIntoSymbolTable( newSymbol, "string-append", "#<procedure string-append>" );
  PutSymbolIntoSymbolTable( newSymbol, "string>?", "#<procedure string>?>" );
  PutSymbolIntoSymbolTable( newSymbol, "string<?", "#<procedure string<?>" );
  PutSymbolIntoSymbolTable( newSymbol, "string=?", "#<procedure string=?>" );
  PutSymbolIntoSymbolTable( newSymbol, "eqv?", "#<procedure eqv?>" );
  PutSymbolIntoSymbolTable( newSymbol, "equal?", "#<procedure equal?>" );
  PutSymbolIntoSymbolTable( newSymbol, "begin", "#<procedure begin>" );
  PutSymbolIntoSymbolTable( newSymbol, "if", "#<procedure if>" );
  PutSymbolIntoSymbolTable( newSymbol, "cond", "#<procedure cond>" );
  PutSymbolIntoSymbolTable( newSymbol, "clean-environment", "#<procedure clean-environment>" );
  PutSymbolIntoSymbolTable( newSymbol, "exit", "#<procedure exit>" );
  PutSymbolIntoSymbolTable( newSymbol, "let", "#<procedure let>" );
  PutSymbolIntoSymbolTable( newSymbol, "lambda", "#<procedure lambda>" );
  while ( !gCallStack.empty() ) {
    gCallStack.pop();
  } // while
  
  while ( !gLambdaStack.empty() ) {
    gLambdaStack.pop();
  } // while
  
} // InitialEnvironment()

Tree * Unbound( bool & found, Token token ) {
  // cout << "UNBOUND>>>\n";
  StackLevel * stackLevel;
  if ( !gCallStack.empty() ) {
    for ( stackLevel = gCallStack.top() ; stackLevel != NULL ; stackLevel = stackLevel->prev ) {
      for ( int i = 0; i < stackLevel->func.parameter.size() ; i++ ) {
        if ( token.str == stackLevel->func.parameter.at( i ).name ) {
          if ( stackLevel->func.parameter.at( i ).exp != NULL ) {
            found = true;
            return stackLevel->func.parameter.at( i ).exp;
          } // if
          
        } // if
        
      } // for

    } // for
    
  } // if
  
  for ( int i = 0; i < gSymbolTable.size() ; i++ ) {    
    if ( token.str == gSymbolTable.at( i ).name ) {
      found = true;
      return gSymbolTable.at( i ).exp;
    } // if
    
  } // for
  
  found = false;
  return NULL;
} // Unbound()

bool Unbound( Token token ) {
  for ( int i = 0; i < gSymbolTable.size() ; i++ ) {
    // cout << "\t" << token.str << "\t" << symbolTable.at(i).name << "\t" 
    // << symbolTable.at(i).exp->token.type << "\t" << symbolTable.at(i).exp << endl;
    if ( token.str == gSymbolTable.at( i ).exp->token.str ) {
      return true;
    } // if
    
  } // for
  
  return false;
} // Unbound()

bool IsPureList( Tree * root ) {
  // 龍骨最右下角不是nil 
  Tree * cur = root;
  while ( cur->right != NULL ) {
    cur = cur->right;
  } // while
  
  if ( cur->token.type != NIL ) {
    return false;
  } // if
  
  return true;
} // IsPureList()

vector <Tree *> GetArgs( Tree * root ) {
  Tree * cur = root;
  vector <Tree *> args;
  if ( cur != NULL ) {
    while ( cur->type != LEAF ) {
      args.push_back( cur->left );
      cur = cur->right;
    } // while 
  } // if
  
  return args;
} // GetArgs()

void ThrowFormatException( Tree * root ) {
  // 可增加toUpperCaseFunction
  string funcName = root->left->token.str;
  if ( funcName == "define" ) {
    cout << "> ERROR (DEFINE format) : ";
    PrintSexp( root, 0 );
    
  } // if
  else if ( funcName == "set!" ) {
    cout << "> ERROR (SET! format) : ";
    PrintSexp( root, 0 );
  } // else if
  else if ( funcName == "lambda" ) {
    cout << "> ERROR (lambda format)\n";
  } // else if
  else if ( funcName == "let" ) {
    cout << "> ERROR (let format)\n";
  } // else if
  else if ( funcName == "cond" ) {
    cout << "> ERROR (COND format) : ";
    PrintSexp( root, 0 );
  } // else if
  
  cout << "\n";
  throw 0;
} // ThrowFormatException()

bool Redefine( string funcName, vector<Symbol> table ) {
  for ( int i = 0; i < table.size() ; i++ ) {
    if ( funcName == table.at( i ).name ) {
      return true;
    } // if
  } // for
  
  return false;
} // Redefine()

void FormatErrorChecking( Tree * root ) {
  string funcName = root->left->token.str;
  vector <Tree *> args = GetArgs( root->right );
  vector <Tree *> symbols;
  vector <Tree *> sexps;
  if ( funcName == "define" ) {
    if ( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    if ( args.at( 0 )->type == ROOT ) {
      symbols = GetArgs( args.at( 0 ) );
    } // if
    else {
      symbols.push_back( args.at( 0 ) );
    } // else
    
    for ( int i = 1; i < args.size() ; i++ ) {
      sexps.push_back( args.at( i ) );
    } // for
    
    for ( int i = 0; i < symbols.size() ; i++ ) {
      if ( symbols.at( i )->token.type != SYMBOL ) {
        ThrowFormatException( root );
      } // if
    } // for
    
    for ( int i = 0; i < symbols.size() ; i++ ) {
      for ( int j = 0; j < gSystemFunctionTable.size() ; j++ ) {
        if ( symbols.at( i )->token.str == gSystemFunctionTable.at( j ).name ) {
          ThrowFormatException( root );
        } // if
      } // for
    } // for
    
  } // if
  else if ( funcName == "set!" ) {
    
  } // else if
  else if ( funcName == "lambda" ) {
    if ( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    if ( args.at( 0 )->token.type != NIL ) {
      if ( args.at( 0 )->type == ROOT ) {
        symbols = GetArgs( args.at( 0 ) );
        for ( int i = 0; i < symbols.size() ; i++ ) {
          if ( symbols.at( i )->token.type != SYMBOL ) {
            ThrowFormatException( root );
          } // if
        } // for
        
      } // if
      else {
        ThrowFormatException( root );
      } // else
      
    } // if
    else {
      // 表沒有區域變數 
    } // else
    
    
  } // else if
  else if ( funcName == "let" ) {
    vector <Tree *> pairs;
    vector <Tree *> pairitems;
    if ( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    if ( args.at( 0 )->token.type != NIL ) {
      if ( args.at( 0 )->type == ROOT ) {
        pairs = GetArgs( args.at( 0 ) );
        for ( int i = 0; i < pairs.size() ; i++ ) {
          pairitems = GetArgs( pairs.at( 0 ) );
          if ( pairitems.size() != 2 ) {
            ThrowFormatException( root );
          } // if
          
          if ( pairitems.at( 0 )->token.type != SYMBOL ) {
            ThrowFormatException( root );
          } // if
          
        } // for
      } // if
      else {
        ThrowFormatException( root );
      } // else
      
    } // if
    else {
      // 表沒有區域變數 
    } // else

  } // else if
  else if ( funcName == "cond" ) {
    if ( args.size() < 1 ) {
      ThrowFormatException( root );
    } // if
    
    for ( int i = 0; i < args.size() ; i++ ) {
      
      if ( !IsPureList( args.at( i ) ) ) {
        ThrowFormatException( root );
      } // if
      
      sexps = GetArgs( args.at( i ) );
      if ( sexps.size() < 2 ) {
        ThrowFormatException( root );
      } // if
      
    } // for

  } // else if
  
} // FormatErrorChecking()

void DEBUG_ARGLIST( vector <Tree *> args ) {
  cout << "DEBUG_ARGLIST\n";
  for ( int i = 0; i < args.size() ; i++ ) {
    PrintSexp( args.at( i ), 0 );
  } // for
} // DEBUG_ARGLIST()

Tree * Cons( Tree * root ) {
  Tree * result = new Tree;
  vector <Tree *> args = GetArgs( root->right );
  result->left = Eval( args.at( 0 ), result->left, false );
  result->right = Eval( args.at( 1 ), result->right, false );

  return result;
} // Cons()

Tree * Quote( vector <Tree *> args ) {
  Tree * root = new Tree;
  Initialize( root->token );
  root = args.at( 0 );
  return root;
} // Quote()

Tree * List( vector <Tree *> args ) {
  Tree * root = new Tree;
  Initialize( root, ROOT );
  Tree * temp = new Tree;
  Tree * cur = root;
  Token nilToken;
  Initialize( nilToken, NIL, "nil" );
  // cout << "EVAL ARGs\n";

  if ( args.size() == 0 ) {
    Initialize( root, LEAF, nilToken );
    return root;
  } // if
  
  for ( int i = 0; i < args.size() ; i++ ) {
    // cout << i << endl;
    temp = Eval( args.at( i ), temp, false );
    cur->left = temp;
    if ( i != args.size() - 1 ) {
      cur->right = new Tree;
      Initialize( cur->right, INTERNAL );
      cur = cur->right;
    } // if
    
  } // for
  
  cur->right = new Tree;
  Initialize( cur->right, LEAF, nilToken );
  return root;
} // List()

Tree * Define( Tree * root ) {
  // cout << "AAAAAAAA";
  string symbolName;
  Symbol symbol;
  bool redefine = false;
  vector <Tree *> args = GetArgs( root->right );
  Tree * sym = args.at( 0 );
  vector<Tree *> symbols;
  vector<Tree *> sexps;
  if ( args.at( 0 )->type == ROOT ) {
    // cout << "FUNCTION definition\n";
    Function func;
    symbols = GetArgs( args.at( 0 ) );
    Symbol aFormalParameter;
    
    func.name = symbols.at( 0 )->token.str;
    func.procedure = "#<procedure " + symbols.at( 0 )->token.str + ">";

    for ( int i = 1; i < symbols.size() ; i++ ) {
      aFormalParameter.name = symbols.at( i )->token.str;
      aFormalParameter.exp = NULL;
      func.parameter.push_back( aFormalParameter );
    } // for
    
    func.functionbody = root->right->right;
    for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
      if ( gUserDefineFunction.at( i ).name == func.name ) {
        gUserDefineFunction.at( i ) = func;
        redefine = true;
      } // if
      
    } // for
  
    if ( !redefine ) {
      gUserDefineFunction.push_back( func );
    } // if
    
    Tree * result = new Tree;
    result->token.str = func.procedure;
    result->token.type = PROCEDURE;
    result->left = NULL;
    result->right = NULL;
    result->type = ROOT;
    symbol.name = symbols.at( 0 )->token.str;
    symbol.exp = result;
    redefine = false;
    for ( int i = 0; i < gSymbolTable.size() ; i++ ) {
      if ( symbol.name == gSymbolTable.at( i ).name ) {
        // redefine
        redefine = true;
        gSymbolTable.at( i ).exp = symbol.exp;
      } // if
    } // for
    
    if ( !redefine ) {
      gSymbolTable.push_back( symbol );
    } // if
    
    cout << "> " << symbol.name << " defined\n";
  } // if
  else if ( args.at( 0 )->type == LEAF ) {
    // cout << "SYMBOL definition\n";
    symbols.push_back( args.at( 0 ) );
    for ( int i = 1; i < args.size() ; i++ ) {
      sexps.push_back( args.at( i ) );
    } // for
    
    if ( sexps.size() != 1 ) {
      ThrowFormatException( root );
    } // if
    
    symbol.name = symbols.at( 0 )->token.str;
    symbol.exp = Eval( sexps.at( 0 ), symbol.exp, false );
    
    if ( !gLambdaStack.empty() ) {
      // cout << "SYMBOL definition and Stack not EMPTY\n";
      Function func;
      func = gLambdaStack.top();
      func.name = symbol.name;
      for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
        if ( gUserDefineFunction.at( i ).name == func.name ) {
          gUserDefineFunction.at( i ) = func;
          redefine = true;
        } // if
        
      } // for
    
      if ( !redefine ) {
        gUserDefineFunction.push_back( func );
      } // if
      
      gLambdaStack.pop();
    } // if
    else {
      // cout << "DEFINE :" << symbol.exp->token.str << endl;
      // cout << sexps.at( 0 )->token.str << endl;
      if ( symbol.exp->token.str == "#<procedure lambda>" ) {
        // cout << "SSSSSSSSSSSSSS";
        Function func;
        for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
          if ( gUserDefineFunction.at( i ).name == sexps.at( 0 )->token.str ) {
            func = gUserDefineFunction.at( i );
          } // if
          
        } // for
        
        func.name = symbol.name;
        redefine = false;
        for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
          if ( gUserDefineFunction.at( i ).name == func.name ) {
            gUserDefineFunction.at( i ) = func;
            redefine = true;
          } // if
          
        } // for
      
        if ( !redefine ) {
          gUserDefineFunction.push_back( func );
        } // if
        
        Tree * result = new Tree;
        result->token.str = func.procedure;
        result->token.type = PROCEDURE;
        result->left = NULL;
        result->right = NULL;
        result->type = ROOT;
        symbol.name = symbols.at( 0 )->token.str;
        symbol.exp = result;
        
        redefine = false;
        for ( int i = 0; i < gSymbolTable.size() ; i++ ) {
          if ( symbol.name == gSymbolTable.at( i ).name ) {
            // redefine
            redefine = true;
            gSymbolTable.at( i ).exp = symbol.exp;
          } // if
        } // for
        
        if ( !redefine ) {
          gSymbolTable.push_back( symbol );
        } // if
        
        return NULL;
      } // if
      
      int itemToRemove = -1;
      for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
        if ( gUserDefineFunction.at( i ).name == symbol.name ) {
          // cout << "FOUND REMOVE\n";
          itemToRemove = i;
        } // if
        
      } // for
      
      if ( itemToRemove >= 0 ) {
        // cout << "REMOVE :" << gUserDefineFunction.at( itemToRemove ).name << "\n";
        gUserDefineFunction.erase( gUserDefineFunction.begin() + itemToRemove );
      } // if
      
    } // else
    
    redefine = false;
    for ( int i = 0; i < gSymbolTable.size() ; i++ ) {
      if ( symbol.name == gSymbolTable.at( i ).name ) {
        // redefine
        redefine = true;
        gSymbolTable.at( i ).exp = symbol.exp;
      } // if
    } // for
    
    if ( !redefine ) {
      gSymbolTable.push_back( symbol );
    } // if
    
    cout << "> " << symbol.name << " defined\n";
  } // else if
  
  return NULL;
} // Define()

Tree * Car( vector <Tree *> args ) {
  Tree * result = new Tree;
  Tree * temp = new Tree;
  Initialize( result, ROOT );
  temp = Eval( args.at( 0 ), temp, false );

  if ( !Boolean( IsPair( temp ) ) ) {
    cout << "> ERROR (car with incorrect argument type) : ";
    PrintSexp( temp, 0 );
    cout << "\n";
    throw 0;
  } // if
  else {
    result = temp->left;
  } // else
  
  return result;
} // Car()

Tree * Cdr( vector <Tree *> args ) {
  Tree * root = new Tree;
  Tree * temp = new Tree;
  Initialize( root, ROOT );
  temp = Eval( args.at( 0 ), temp, false );
  if ( !Boolean( IsPair( temp ) ) ) {
    cout << "> ERROR (cdr with incorrect argument type) : ";
    PrintSexp( temp, 0 );
    cout << "\n";
    throw 0;
  } // if
  else {
    root = temp->right;
    root->type = ROOT;
  } // else
  
  return root;
} // Cdr()

Tree * True() {
  Tree * root = new Tree;
  Token token;
  Initialize( token, T, "#f" );
  Initialize( root, ROOT, token );
  return root;
} // True()

Tree * False() {
  Tree * root = new Tree;
  Token token;
  Initialize( token, NIL, "nil" );
  Initialize( root, ROOT, token );
  return root;
} // False()


Tree * IsAtom( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  if ( result->left == NULL && result->right == NULL ) {
    return True();
  } // if
  
  return False();
} // IsAtom()

Tree * IsPair( Tree * root ) {
  // 只接收Evaluate後參數 
  if ( root->right != NULL ) {
    return True();
  } // if
  
  return False();
} // IsPair()

Tree * IsList( Tree * root ) {
  Tree * result = new Tree;
  
  vector <Tree *> args = GetArgs( root->right );
  result = Eval( args.at( 0 ), result, false );
  if ( IsPureList( result ) ) {
    return True();
  } // if

  return False();
} // IsList()

Tree * IsNull( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  if ( result->left == NULL && result->right == NULL && result->token.type == NIL ) {
    return True();
  } // if

  return False();
} // IsNull()

Tree * IsInteger( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL && result->token.type == INT ) {
    return True();
  } // if

  return False();
} // IsInteger()

Tree * IsReal( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL 
       && ( result->token.type == INT || result->token.type == FLOAT ) ) {
    return True();
  } // if

  return False();
} // IsReal()

Tree * IsNumber( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL 
       && ( result->token.type == INT || result->token.type == FLOAT ) ) {
    return True();
  } // if

  return False();
} // IsNumber()

Tree * IsString( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL && result->token.type == STRING ) {
    return True();
  } // if

  return False();
} // IsString()

Tree * IsSymbol( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL && result->token.type == SYMBOL ) {
    return True();
  } // if

  return False();
} // IsSymbol()

Tree * IsBoolean( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  
  if ( result->left == NULL && result->right == NULL 
       && ( result->token.type == NIL || result->token.type == T ) ) {
    return True();
  } // if

  return False();
} // IsBoolean()

Tree * Add( vector <Tree *> args ) {
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize( token, INT, "0" );
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  // atof(node->token.str.c_str())
  for ( int i = 0; i < args.size() ; i++ ) {
    number = Eval( args.at( i ), number, false );
    if ( number->left != NULL && number->right != NULL ) {
      cout << "> ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n"; 
      throw 0;
    } // if
    else if ( number->left == NULL && number->right == NULL 
              && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "> ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n";
      throw 0;
    } // else if
    
    if ( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    resultValue = resultValue + atof( number->token.str.c_str() );
  } // for
  
  // 將 resultValue 轉成str
  if ( result->token.type == INT ) {
    // 將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
  } // if
  else if ( result->token.type == FLOAT ) {
    // 將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Add()

Tree * Sub( vector <Tree *> args ) {
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize( token, INT, "0" );
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  number = Eval( args.at( 0 ), number, false );
  if ( number->left != NULL && number->right != NULL ) {
    cout << "> ERROR (- with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n"; 
    throw 0;
  } // if
  else if ( number->left == NULL && number->right == NULL 
            && number->token.type != INT && number->token.type != FLOAT ) {
    cout << "> ERROR (- with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n";
    throw 0;
  } // else if
  
  if ( number->token.type == FLOAT ) {
    result->token.type = FLOAT;
  } // if
  
  resultValue = atof( number->token.str.c_str() );
  // cout << "HHH " << resultValue << " HHH";
  // atof(node->token.str.c_str())
  for ( int i = 1; i < args.size() ; i++ ) {
    number = Eval( args.at( i ), number, false );
    if ( number->left != NULL && number->right != NULL ) {
      cout << "> ERROR (- with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n"; 
      throw 0;
    } // if
    else if ( number->left == NULL && number->right == NULL 
              && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "> ERROR (- with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n";
      throw 0;
    } // else if
    
    if ( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    // cout << resultValue << "\t";
    // cout << atof( number->token.str.c_str() ) << "\t";
    resultValue = resultValue - atof( number->token.str.c_str() );
    // cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if ( result->token.type == INT ) {
    // 將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    // string num_str = sstream.str();
  } // if
  else if ( result->token.type == FLOAT ) {
    // 將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Sub()

Tree * Mult( vector <Tree *> args ) {
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize( token, INT, "0" );
  Initialize( result, ROOT, token );
  float resultValue = 1;
  // atof(node->token.str.c_str())
  for ( int i = 0; i < args.size() ; i++ ) {
    number = Eval( args.at( i ), number, false );
    if ( number->left != NULL && number->right != NULL ) {
      cout << "> ERROR (* with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n"; 
      throw 0;
    } // if
    else if ( number->left == NULL && number->right == NULL 
              && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "> ERROR (* with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n";
      throw 0;
    } // else if
    
    if ( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    resultValue = resultValue * atof( number->token.str.c_str() );
  } // for
  
  // 將 resultValue 轉成str
  if ( result->token.type == INT ) {
    // 將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
  } // if
  else if ( result->token.type == FLOAT ) {
    // 將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Mult()

Tree * Div( vector <Tree *> args ) {
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize( token, INT, "0" );
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  number = Eval( args.at( 0 ), number, false );
  if ( number->left != NULL && number->right != NULL ) {
    cout << "> ERROR (/ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n"; 
    throw 0;
  } // if
  else if ( number->left == NULL && number->right == NULL 
            && number->token.type != INT && number->token.type != FLOAT ) {
    cout << "> ERROR (/ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n";
    throw 0;
  } // else if
  
  if ( number->token.type == FLOAT ) {
    result->token.type = FLOAT;
  } // if
  
  resultValue = atof( number->token.str.c_str() );
  
  // atof(node->token.str.c_str())
  for ( int i = 1; i < args.size() ; i++ ) {
    number = Eval( args.at( i ), number, false );
    if ( number->left != NULL && number->right != NULL ) {
      cout << "> ERROR (/ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n"; 
      throw 0;
    } // if
    else if ( number->left == NULL && number->right == NULL 
              && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "> ERROR (/ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n";
      throw 0;
    } // else if
    
    if ( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    // cout << resultValue << "\t";
    // cout << atof( number->token.str.c_str() ) << "\t";
    if ( atof( number->token.str.c_str() ) == 0 ) {
      cout << "> ERROR (division by zero) : /\n\n";
      throw 0;
    } // if
    
    resultValue = resultValue / atof( number->token.str.c_str() );
    // cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if ( result->token.type == INT ) {
    // 將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    // string num_str = sstream.str();
  } // if
  else if ( result->token.type == FLOAT ) {
    // 將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Div()

Tree * Not( vector <Tree *> args ) {
  Tree * result = new Tree;
  result = Eval( args.at( 0 ), result, false );
  if ( result->left == NULL && result->right == NULL && result->token.type == NIL ) {
    return True();
  } // if
  
  return False();
} // Not()

Tree * Greater( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      bigger = Eval( args.at( i ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL 
           || ( bigger->token.type != INT && bigger->token.type != FLOAT ) ) {
        cout << "> ERROR (> with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      smaller = Eval( args.at( i + 1 ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL
           || ( smaller->token.type != INT && smaller->token.type != FLOAT ) ) {
        cout << "> ERROR (> with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( atof( bigger->token.str.c_str() ) <= atof( smaller->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Greater()

Tree * Ge( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      bigger = Eval( args.at( i ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL 
           || ( bigger->token.type != INT && bigger->token.type != FLOAT ) ) {
        cout << "> ERROR (>= with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      smaller = Eval( args.at( i + 1 ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL
           || ( smaller->token.type != INT && smaller->token.type != FLOAT ) ) {
        cout << "> ERROR (>= with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( atof( bigger->token.str.c_str() ) < atof( smaller->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Ge()

Tree * Less( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      smaller = Eval( args.at( i ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL 
           || ( smaller->token.type != INT && smaller->token.type != FLOAT ) ) {
        cout << "> ERROR (< with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      bigger = Eval( args.at( i + 1 ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL
           || ( bigger->token.type != INT && bigger->token.type != FLOAT ) ) {
        cout << "> ERROR (< with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( atof( smaller->token.str.c_str() ) >= atof( bigger->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Less()

Tree * Le( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      smaller = Eval( args.at( i ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL 
           || ( smaller->token.type != INT && smaller->token.type != FLOAT ) ) {
        cout << "> ERROR (<= with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      bigger = Eval( args.at( i + 1 ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL
           || ( bigger->token.type != INT && bigger->token.type != FLOAT ) ) {
        cout << "> ERROR (<= with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( atof( smaller->token.str.c_str() ) > atof( bigger->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Le()

Tree * Equ( vector <Tree *> args ) {
  Tree * result = True();
  Tree * equ1 = new Tree;
  Tree * equ2 = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      equ1 = Eval( args.at( i ), equ1, false );
      
      if ( equ1->left != NULL || equ1->right != NULL 
           || ( equ1->token.type != INT && equ1->token.type != FLOAT ) ) {
        cout << "> ERROR (= with incorrect argument type) : ";
        PrintSexp( equ1, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      equ2 = Eval( args.at( i + 1 ), equ2, false );
      
      if ( equ2->left != NULL || equ2->right != NULL
           || ( equ2->token.type != INT && equ2->token.type != FLOAT ) ) {
        cout << "> ERROR (= with incorrect argument type) : ";
        PrintSexp( equ2, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( atof( equ1->token.str.c_str() ) != atof( equ2->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Equ()

Tree * StringAppend( vector <Tree *> args ) {
  Tree * result = new Tree();
  Initialize( result, ROOT );
  Tree * temp = new Tree;
  result = Eval( args.at( 0 ), result, false );

  if ( result->left != NULL || result->right != NULL || result->token.type != STRING ) {
    cout << "> ERROR (string-append with incorrect argument type) : ";
    PrintSexp( result, 0 );
    cout << "\n";
    throw 0;
  } // if
  
  result->token.str.erase( 0, 1 );
  result->token.str.erase( result->token.str.size()-1, 1 );
  
  for ( int i = 1; i < args.size() ; i++ ) {

    temp = Eval( args.at( i ), temp, false );
      
    if ( temp->left != NULL || temp->right != NULL || temp->token.type != STRING ) {
      cout << "> ERROR (string-append with incorrect argument type) : ";
      PrintSexp( temp, 0 );
      cout << "\n";
      throw 0;
    } // if
      
    temp->token.str.erase( 0, 1 );
    temp->token.str.erase( temp->token.str.size()-1, 1 );
    result->token.str = result->token.str + temp->token.str;
  } // for
  
  result->token.str = "\"" + result->token.str + "\"";
  return result;
} // StringAppend()

Tree * StringGreater( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      bigger = Eval( args.at( i ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL 
           || bigger->token.type != STRING ) {
        cout << "> ERROR (string>? with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      smaller = Eval( args.at( i + 1 ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL
           || smaller->token.type != STRING ) {
        cout << "> ERROR (string>? with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( bigger->token.str <= smaller->token.str ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // StringGreater()

Tree * StringLess( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  
  for ( int i = 0; i < args.size() ; i++ ) {
    if ( i + 1 != args.size() ) {
      smaller = Eval( args.at( i ), smaller, false );
      
      if ( smaller->left != NULL || smaller->right != NULL 
           || smaller->token.type != STRING ) {
        cout << "> ERROR (string<? with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      bigger = Eval( args.at( i + 1 ), bigger, false );
      
      if ( bigger->left != NULL || bigger->right != NULL
           || bigger->token.type != STRING ) {
        cout << "> ERROR (string<? with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n";
        throw 0;
      } // if
      
      if ( smaller->token.str >= bigger->token.str ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // StringLess()

// StringEqu
Tree * StringEqu( vector <Tree *> args ) {
  Tree * result = True();
  Tree * str1 = new Tree;
  Tree * str2 = new Tree;
  
  str1 = Eval( args.at( 0 ), str1, false );
  if ( str1->left != NULL || str1->right != NULL 
       || str1->token.type != STRING ) {
    cout << "> ERROR (string=? with incorrect argument type) : ";
    PrintSexp( str1, 0 );
    cout << "\n";
    throw 0;
  } // if
  
  for ( int i = 1; i < args.size() ; i++ ) {
    str2 = Eval( args.at( i ), str2, false );
    if ( str2->left != NULL || str2->right != NULL
         || str2->token.type != STRING ) {
      cout << "> ERROR (string=? with incorrect argument type) : ";
      PrintSexp( str2, 0 );
      cout << "\n";
      throw 0;
    } // if
    
    if ( str1->token.str != str2->token.str ) {
      result = False();
    } // if
      
    
  } // for
  
  return result;
} // StringEqu()

Tree * Eqv( vector <Tree *> args ) {
  Tree * result = False();
  Tree * arg1 = new Tree;
  Tree * arg2 = new Tree;
  arg1 = Eval( args.at( 0 ), arg1, false );
  arg2 = Eval( args.at( 1 ), arg2, false );
  vector <Tree *> arg1s;
  vector <Tree *> arg2s;
  arg1s.push_back( args.at( 0 ) );
  arg2s.push_back( args.at( 1 ) );
  if ( Boolean( IsAtom( arg1s ) ) == true || Boolean( IsAtom( arg2s ) ) == true ) {
    if ( arg1->token.type == STRING && arg2->token.type == STRING ) {
      if ( arg1 == arg2 ) {
        return True();
      } // if
    } // if
    else if ( arg1->token.type != STRING && arg2->token.type != STRING ) {
      if ( arg1->token.str == arg2->token.str ) {
        return True();
      } // if
      else if ( arg1->token.type == NIL && arg2->token.type == NIL ) {
        return True();
      } // else if
    } // else if
    else {
      return False();
    } // else
  } // if
  else {
    if ( arg1 == arg2 ) {
      return True();
    } // if
  } // else
  
  return False();
} // Eqv()

void Preorder( vector <Tree *> & nodes, Tree * current ) {
  if ( current != NULL ) {  
    nodes.push_back( current ); 
    Preorder( nodes, current->left );
    Preorder( nodes, current->right ); 
  } // if
} // Preorder()

Tree * Equal( vector <Tree *> args ) {
  Tree * result = True();
  Tree * arg1 = new Tree;
  Tree * arg2 = new Tree;
  arg1 = Eval( args.at( 0 ), arg1, false );
  arg2 = Eval( args.at( 1 ), arg2, false );
  vector<Tree *> nodes1;
  vector<Tree *> nodes2;
  Preorder( nodes1, arg1 );
  Preorder( nodes2, arg2 );
  if ( nodes1.size() != nodes2.size() ) {
    return False();
  } // if
  
  for ( int i = 0; i < nodes1.size() ; i++ ) {
    // cout << "AAA " << nodes1.at(i)->token.str << "\t" << nodes2.at(i)->token.str <<"\t" " AAA";
    if ( nodes1.at( i )->token.str != nodes2.at( i )->token.str 
         && ( nodes1.at( i )->token.type != NIL || nodes2.at( i )->token.type != NIL ) ) {
      result = False() ;
    } // if
    
  } // for
  
  return result;
} // Equal()

Tree * And( vector <Tree *> args ) {
  Tree * result = new Tree;
  for ( int i = 0; i < args.size() ; i++ ) {
    result = Eval( args.at( i ), result, false );
    // cout << i;
    if ( result->left == NULL && result->right == NULL && result->token.type == NIL ) {
      return result;
    } // if
  } // for
  
  return result;
} // And()

Tree * Or( vector <Tree *> args ) {
  Tree * result = new Tree;
  for ( int i = 0; i < args.size() ; i++ ) {
    result = Eval( args.at( i ), result, false );
    // cout << i;
    if ( result->left == NULL && result->right == NULL && result->token.type != NIL ) {
      return result;
    } // if
  } // for
  
  return result;
} // Or()

Tree * If( Tree * root ) {
  vector<Tree *> args = GetArgs( root->right );
  Tree * result = new Tree;
  Tree * condition = new Tree;
  condition = Eval( args.at( 0 ), condition, false );
  if ( Boolean( condition ) == true ) {
    result = Eval( args.at( 1 ), result, false );
    return result;
  } // if
  else {
    if ( args.size() == 2 ) {
      cout << "> ERROR (no return value) : ";
      PrintSexp( root, 0 );
      cout << "\n";
      throw 0;
    } // if
    else {
      result = Eval( args.at( 2 ), result, false );
      return result;
    } // else
    
  } // else
} // If()

Tree * Cond( Tree * root ) {
  vector<Tree *> args = GetArgs( root->right );
  Tree * result = new Tree;
  vector<Tree *> situations;
  Tree * condition = new Tree;
  for ( int i = 0; i < args.size() ; i++ ) {
    situations = GetArgs( args.at( i ) );
    
    if ( i == args.size() - 1 ) {
      if ( situations.at( 0 )->token.str == "else" ) {
        for ( int j = 1; j < situations.size() ; j++ ) {
          result = Eval( situations.at( j ), result, false );
        } // for
        
        result->type = ROOT;
        return result;
      } // if
       
    } // if
    
    condition = Eval( situations.at( 0 ), condition, false );
    if ( Boolean( condition ) ) {
      for ( int j = 1; j < situations.size() ; j++ ) {
        result = Eval( situations.at( j ), result, false );
      } // for

      return result;
    } // if
    else {
      // 是false跳過此選擇 
    } // else
    
  } // for
  
  cout << "> ERROR (no return value) : ";
  PrintSexp( root, 0 );
  cout << "\n"; 
  throw 0;

} // Cond()

Tree * Begin( vector<Tree *> args ) {
  Tree * result = new Tree;
  for ( int i = 0; i < args.size() ; i++ ) {
    result = Eval( args.at( i ), result, false );
  } // for
  
  return result;
} // Begin()

Tree * Exit( vector<Tree *> args ) {
  cout << "> \nThanks for using OurScheme!";
  exit( 0 );
} // Exit()

Tree * Let( Tree * root ) {
  Symbol aVariable;
  Function func;
  Tree * result = new Tree;
  vector<Tree *> args = GetArgs( root->right );
  vector<Tree *> localVariables = GetArgs( args.at( 0 ) );
  vector<Symbol> parameters;
  for ( int i = 0; i < localVariables.size() ; i++ ) {
    aVariable.name = localVariables.at( i )->left->token.str;
    aVariable.exp = Eval( localVariables.at( i )->right->left, aVariable.exp, false );
    parameters.push_back( aVariable );
  } // for
  
  func.name = "let";
  func.procedure = "#<procedure let>";
  func.parameter = parameters;
  
  StackLevel * stackLevel = new StackLevel;
  stackLevel->func = func;
  stackLevel->prev = NULL;
  stackLevel->next = NULL;
  

  if ( !gCallStack.empty() ) {
    stackLevel->prev = gCallStack.top();
    gCallStack.top()->next = stackLevel;
  } // if

  gCallStack.push( stackLevel );

  for ( int i = 1; i < args.size() ; i++ ) {
    result = Eval( args.at( i ), result, false );
  } // for
  
  gCallStack.top()->func.parameter.clear();
  gCallStack.pop();
  
  if ( !gCallStack.empty() ) {
    gCallStack.top()->next = NULL;
  } // if
  
  return result;
} // Let()

Tree * DefineLambda( Tree * root ) {
  // cout << "DEFINE LAMBDA\n";
  // define 1 個lambda放進lambda stack 
  // bool reDefine = false;
  vector<Tree *> args = GetArgs( root->right );
  vector<Tree *> formalParameter = GetArgs( args.at( 0 ) );
  Function func;
  func.name = "lambda";
  func.procedure = "#<procedure lambda>";

  Symbol aFormalParameter;
  for ( int i = 0; i < formalParameter.size() ; i++ ) {
    aFormalParameter.name = formalParameter.at( i )->token.str;
    aFormalParameter.exp = NULL;
    func.parameter.push_back( aFormalParameter );
  } // for
  
  func.functionbody = root->right->right;
  gLambdaStack.push( func );

  Tree * result = new Tree;
  result->token.str = "#<procedure lambda>";
  result->token.type = PROCEDURE;
  result->type = ROOT;
  result->left = NULL;
  result->right = NULL;
  return result;
} // DefineLambda()

Tree * CleanEnvironment( vector<Tree *> args ) {
  InitialEnvironment();
  cout << "> environment cleaned\n";
  return NULL;
} // CleanEnvironment()

Tree * Lambda( Tree * root ) {
  vector<Tree *> args = GetArgs( root->right );
  Function func;
    
  if ( gLambdaStack.empty() ) {
    // cout << "LAMBDA DEFINITION\n";
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : lambda\n\n";
      throw 0;
    } // if
  
    vector<Tree *> formalParameter = GetArgs( args.at( 0 ) );
    func.name = "lambda";
    func.procedure = "#<procedure lambda>";
  
    Symbol aFormalParameter;
    for ( int i = 0; i < formalParameter.size() ; i++ ) {
      aFormalParameter.name = formalParameter.at( i )->token.str;
      aFormalParameter.exp = NULL;
      func.parameter.push_back( aFormalParameter );
    } // for
    
    func.functionbody = root->right->right;
    gLambdaStack.push( func );
  
    Tree * result = new Tree;
    result->token.str = "#<procedure lambda>";
    result->token.type = PROCEDURE;
    result->type = ROOT;
    result->left = NULL;
    result->right = NULL;
    return result;
  } // if
  else {
    // cout << "LAMBDA EXCUTION\n";
    // cout << "FUNCTION NAME: " << root->left->token.str << "\n";
    StackLevel * stackLevel = new StackLevel;
    
    if ( !gLambdaStack.empty() ) {
      // 宣告完直接執行的lambda
      stackLevel->func = gLambdaStack.top();
      stackLevel->prev = NULL;
      stackLevel->next = NULL;
      gLambdaStack.pop();
    } // if
    
    if ( !gCallStack.empty() ) {
      stackLevel->prev = gCallStack.top();
      stackLevel->next = NULL;
      gCallStack.top()->next = stackLevel;
    } // if
  
    gCallStack.push( stackLevel );
    
    // vector<Tree *> args = GetArgs( root->right );
    if ( args.size() != gCallStack.top()->func.parameter.size() ) {
      cout << "ERROR (incorrect number of arguments) : lambda expression\n\n";
      throw 0;
    } // if
    
    vector<Symbol> parameterAfterEval;
    for ( int i = 0; i < args.size() ; i++ ) {
      Symbol aParameterAfterEval;
      aParameterAfterEval.exp = Eval( args.at( i ), aParameterAfterEval.exp, false ) ;
      parameterAfterEval.push_back( aParameterAfterEval );
    } // for
    
    for ( int i = 0; i < parameterAfterEval.size() ; i++ ) {
      gCallStack.top()->func.parameter.at( i ).exp = parameterAfterEval.at( i ).exp;
    } // for
    
    Tree * result = new Tree;
    vector<Tree *> toBeEval = GetArgs( gCallStack.top()->func.functionbody );
    for ( int i = 0; i < toBeEval.size() ; i++ ) {
      result = Eval( toBeEval.at( i ), result, false );
    } // for
    
    if ( !gCallStack.empty() ) {
      gCallStack.top()->func.name = "";
      gCallStack.top()->func.parameter.clear();
      //gCallStack.top()->func.functionbody = NULL;
      gCallStack.top()->next = NULL;
      gCallStack.top()->prev = NULL;
      gCallStack.pop();
      if ( !gCallStack.empty() ) {
        gCallStack.top()->next = NULL;
      } // if
      
    } // if
    
    if ( !gLambdaStack.empty() ) {
      gLambdaStack.top().parameter.clear();
      gLambdaStack.top().functionbody = NULL;
      gLambdaStack.pop();
      
    } // if
    
    return result;
  } // else
  
} // Lambda()

Tree * UserFunction( Tree * root ) {
  // cout << "USER FUNCTION\n";
  string procdureName = Eval( root->left, root->left, false )->token.str;
  string funcName = root->left->token.str;
  
  bool lambdafunc = false;
  
  StackLevel * stackLevel = new StackLevel;
  stackLevel->next = NULL;
  stackLevel->prev = NULL;
  // cout << "BBB " << gUserDefineFunction.size() << " BBB\n";
  for ( int i = 0; i < gUserDefineFunction.size() ; i++ ) {
    // cout << "INUSRFUNC: " << gUserDefineFunction.at( i ).name << "\t" << funcName << "\n";
    // cout << "PROCE: " << gUserDefineFunction.at( i ).procedure 
    // << "\t" << Eval( root->left, root->left, false )->token.str << "\n";
    if ( gUserDefineFunction.at( i ).procedure == procdureName ) {
      if ( procdureName == "#<procedure lambda>" ) {
        if ( gUserDefineFunction.at( i ).name == funcName ) {
          stackLevel->func = gUserDefineFunction.at( i );
          for ( int j = 0; j < stackLevel->func.parameter.size() ; j++ ) {
            stackLevel->func.parameter.at( j ).exp = NULL;
          } // for
          
          if ( !gCallStack.empty() ) {
            stackLevel->prev = gCallStack.top();
            stackLevel->next = NULL;
            gCallStack.top()->next = stackLevel;
          } // if
    
          gCallStack.push( stackLevel );
        } // if
        
      } // if
      else {
        stackLevel->func = gUserDefineFunction.at( i );        
        for ( int j = 0; j < stackLevel->func.parameter.size() ; j++ ) {
          stackLevel->func.parameter.at( j ).exp = NULL;
        } // for
        
        if ( !gCallStack.empty() ) {
          stackLevel->prev = gCallStack.top();
          stackLevel->next = NULL;
          gCallStack.top()->next = stackLevel;
        } // if
  
        gCallStack.push( stackLevel );
      } // else
      
    } // if
   
  } // for
  
  vector<Tree *> args = GetArgs( root->right );
  
  if ( args.size() != gCallStack.top()->func.parameter.size() ) {
    cout << "ERROR (incorrect number of arguments) : " + root->left->token.str + " expression\n\n";
    throw 0;
  } // if
  
  
  Tree * temp = new Tree;
  // cout << "MMM : " << args.size() << " MMM\n";
  vector<Symbol> parameterAfterEval;
  for ( int i = 0; i < args.size() ; i++ ) {
    Symbol aParameterAfterEval;
    aParameterAfterEval.exp = Eval( args.at( i ), temp, false ) ;
    parameterAfterEval.push_back( aParameterAfterEval );
  } // for
  
  for ( int i = 0; i < parameterAfterEval.size() ; i++ ) {
    gCallStack.top()->func.parameter.at( i ).exp = parameterAfterEval.at( i ).exp;
  } // for
  
  Tree * result = new Tree;  
  
  vector<Tree *> toBeEval = GetArgs( gCallStack.top()->func.functionbody );
  for ( int i = 0; i < toBeEval.size() ; i++ ) {
    result = Eval( toBeEval.at( i ), result, false );
  } // for
  
  if ( !gCallStack.empty() ) {
    for ( int i = 0; i < gCallStack.top()->func.parameter.size() ; i++ ) {
      gCallStack.top()->func.parameter.at( i ).exp = NULL;
    } // for
    
    gCallStack.top()->next = NULL;
    gCallStack.top()->prev = NULL;
    gCallStack.pop();
    if ( !gCallStack.empty() ) {
      gCallStack.top()->next = NULL;
    } // if
    
  } // if

  // cout << "EEEEEENNNNNDDDDD";
  return result;
  
} // UserFunction()

Tree * Evaluate( Tree * root ) {
  // cout << "AAAAAAAA";
  vector <Tree *> args = GetArgs( root->right );
  string funcName;
  string funcProcedure;
  // DEBUG_ARGLIST( args );
  Tree * procedure = new Tree;
  if ( root->left->token.type == PROCEDURE ) {
    funcName = root->left->token.str;
  } // if
  
  funcName = root->left->token.str;
  procedure = Eval( root->left, procedure, false );
  if ( procedure->token.str != "#<procedure lambda>" ) {
    funcName = procedure->token.str;
  } // if

  // funcName = procedure->token.str;
  
  // cout << "... " << funcName << " ...\n";
  
  if ( funcName == "#<procedure cons>" || funcName == "cons" ) {
    if ( args.size() != 2 ) {
      cout << "> ERROR (incorrect number of arguments) : cons\n\n";
      throw 0;
    } // if
    
    return Cons( root );
  } // if
  else if ( funcName == "#<procedure list>" || funcName == "list" ) {
    return List( args );
  } // else if
  else if ( funcName == "#<procedure quote>" || funcName == "quote" || funcName == "\'" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : quote\n\n";
      throw 0;
    } // if
    
    return Quote( args );
  } // else if
  else if ( funcName == "#<procedure define>" || funcName == "define" ) {
    if ( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    return Define( root );
  } // else if
  else if ( funcName == "#<procedure car>" || funcName == "car" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : car\n\n";
      throw 0;
    } // if
    
    return Car( args );
  } // else if
  else if ( funcName == "#<procedure cdr>" || funcName == "cdr" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : cdr\n\n";
      throw 0;
    } // if
    
    return Cdr( args );
  } // else if
  else if ( funcName == "#<procedure atom?>" || funcName == "atom?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : atom?\n\n";
      throw 0;
    } // if
    
    return IsAtom( args );
  } // else if
  else if ( funcName == "#<procedure pair?>" || funcName == "pair?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : pair?\n\n";
      throw 0;
    } // if
    
    Tree * temp;
    temp = Eval( args.at( 0 ), temp, false );
    return IsPair( temp );
  } // else if
  else if ( funcName == "#<procedure list?>" || funcName == "list?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : list?\n\n";
      throw 0;
    } // if
    
    return IsList( root );
  } // else if
  else if ( funcName == "#<procedure null?>" || funcName == "null?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : null?\n\n";
      throw 0;
    } // if
    
    return IsNull( args );
  } // else if
  else if ( funcName == "#<procedure integer?>" || funcName == "integer?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : integer?\n\n";
      throw 0;
    } // if
    
    return IsInteger( args );
  } // else if
  else if ( funcName == "#<procedure real?>" || funcName == "real?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : real?\n\n";
      throw 0;
    } // if
    
    return IsReal( args );
  } // else if
  else if ( funcName == "#<procedure number?>" || funcName == "number?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : number?\n\n";
      throw 0;
    } // if
    
    return IsNumber( args );
  } // else if
  else if ( funcName == "#<procedure string?>" || funcName == "string?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : string?\n\n";
      throw 0;
    } // if
    
    return IsString( args );
  } // else if
  else if ( funcName == "#<procedure boolean?>" || funcName == "boolean?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : boolean?\n\n";
      throw 0;
    } // if
    
    return IsBoolean( args );
  } // else if
  else if ( funcName == "#<procedure symbol?>" || funcName == "symbol?" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : symbol?\n\n";
      throw 0;
    } // if
    
    return IsSymbol( args );
  } // else if
  else if ( funcName == "#<procedure +>" || funcName == "+" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : +\n\n";
      throw 0;
    } // if
    
    return Add( args );
  } // else if
  else if ( funcName == "#<procedure ->" || funcName == "-" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : -\n\n";
      throw 0;
    } // if
    
    return Sub( args );
  } // else if
  else if ( funcName == "#<procedure *>" || funcName == "*" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : *\n\n";
      throw 0;
    } // if
    
    return Mult( args );
  } // else if
  else if ( funcName == "#<procedure />" || funcName == "/" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : /\n\n";
      throw 0;
    } // if
    
    return Div( args );
  } // else if
  else if ( funcName == "#<procedure not>" || funcName == "not" ) {
    if ( args.size() != 1 ) {
      cout << "> ERROR (incorrect number of arguments) : not\n\n";
      throw 0;
    } // if
    
    return Not( args );
  } // else if
  else if ( funcName == "#<procedure >>" || funcName == ">" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : >\n\n";
      throw 0;
    } // if
    
    return Greater( args );
  } // else if
  else if ( funcName == "#<procedure >=>" || funcName == ">=" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : >=\n\n";
      throw 0;
    } // if
    
    return Ge( args );
  } // else if
  else if ( funcName == "#<procedure <>" || funcName == "<" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : <\n\n";
      throw 0;
    } // if
    
    return Less( args );
  } // else if
  else if ( funcName == "#<procedure <=>" || funcName == "<=" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : <=\n\n";
      throw 0;
    } // if
    
    return Le( args );
  } // else if
  else if ( funcName == "#<procedure =>" || funcName == "=" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : =\n\n";
      throw 0;
    } // if
    
    return Equ( args );
  } // else if
  else if ( funcName == "#<procedure string-append>" || funcName == "string-append" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : string-append\n\n";
      throw 0;
    } // if
    
    return StringAppend( args );
  } // else if
  else if ( funcName == "#<procedure string>?>" || funcName == "string>?" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : string>?\n\n";
      throw 0;
    } // if
    
    return StringGreater( args );
  } // else if
  else if ( funcName == "#<procedure string<?>" || funcName == "string<?" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : string<?\n\n";
      throw 0;
    } // if
    
    return StringLess( args );
  } // else if
  else if ( funcName == "#<procedure string=?>" || funcName == "string=?" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : string=?\n\n";
      throw 0;
    } // if
    
    return StringEqu( args );
  } // else if
  else if ( funcName == "#<procedure eqv?>" || funcName == "eqv?" ) {
    if ( args.size() != 2 ) {
      cout << "> ERROR (incorrect number of arguments) : eqv?\n\n";
      throw 0;
    } // if
    
    return Eqv( args );
  } // else if
  else if ( funcName == "#<procedure equal?>" || funcName == "equal?" ) {
    if ( args.size() != 2 ) {
      cout << "> ERROR (incorrect number of arguments) : equal?\n\n";
      throw 0;
    } // if
    
    return Equal( args );
  } // else if
  else if ( funcName == "#<procedure and>" || funcName == "and" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : and\n\n";
      throw 0;
    } // if
    
    return And( args );
  } // else if
  else if ( funcName == "#<procedure or>" || funcName == "or" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : or\n\n";
      throw 0;
    } // if
    
    return Or( args );
  } // else if
  else if ( funcName == "#<procedure if>" || funcName == "if" ) {
    if ( args.size() != 2 && args.size() != 3 ) {
      cout << "> ERROR (incorrect number of arguments) : if\n\n";
      throw 0;
    } // if
    
    return If( root );
  } // else if
  else if ( funcName == "#<procedure cond>" || funcName == "cond" ) {
    return Cond( root );
  } // else if
  else if ( funcName == "#<procedure begin>" || funcName == "begin" ) {
    if ( args.size() < 1 ) {
      cout << "> ERROR (incorrect number of arguments) : begin\n\n";
      throw 0;
    } // if
    
    return Begin( args );
  } // else if
  else if ( funcName == "#<procedure clean-environment>" || funcName == "clean-environment" ) {
    if ( args.size() != 0 ) {
      cout << "> ERROR (incorrect number of arguments) : clean-environment\n\n";
      throw 0;
    } // if
    
    return CleanEnvironment( args );
  } // else if
  else if ( funcName == "#<procedure exit>" || funcName == "exit" ) {
    if ( args.size() != 0 ) {
      cout << "> ERROR (incorrect number of arguments) : exit\n\n";
      throw 0;
    } // if
    
    return Exit( args );
  } // else if
  else if ( funcName == "#<procedure let>" || funcName == "let" ) {
    if ( args.size() < 2 ) {
      cout << "> ERROR (incorrect number of arguments) : let\n\n";
      throw 0;
    } // if
    
    return Let( root );
  } // else if
  else if ( funcName == "#<procedure lambda>" || funcName == "lambda" ) {
    // 此處為宣告     
    return Lambda( root );
  } // else if
  else {
    // userdefinefunction
    return UserFunction( root );
  } // else
  
  return NULL;
  
} // Evaluate()

Tree * Eval( Tree * root, Tree * resultexpr, bool isTopLevel ) {
  // cout << "EVALLLLLL\n";
  bool found = false;
  if ( root->left == NULL && root->right == NULL 
       && root->token.type != SYMBOL && root->token.type != QUOTE ) {
    // cout << "AAAAA\n";
    resultexpr = root;
    return resultexpr;
  } // if
  else if ( root->left == NULL && root->right == NULL 
            && ( root->token.type == SYMBOL || root->token.type == QUOTE ) ) {
    // cout << "BBBBB\n";
    resultexpr = Unbound( found, root->token );
    
    if ( !found ) {
      cout << "> ERROR (unbound symbol) : " << root->token.str << "\n\n";
      throw 0;
    } // if
    else {
      return resultexpr;
    } // else
    
  } // else if
  else {
    // cout << "CCCCC\n";
    if ( !IsPureList( root ) ) {
      // cout << "C-A\n";
      cout << "> ERROR (non-list) : ";
      PrintSexp( root, 0 );
      cout << "\n";
      throw 0;
    } // if
    else if ( root->left->type == LEAF && root->left->token.type != SYMBOL 
              && root->left->token.type != QUOTE ) {
      // cout << "C-B\n";
      cout << "> ERROR (attempt to apply non-function) : " << root->left->token.str << "\n\n" ;
      throw 0;
    } // else if
    else if ( root->left->type == LEAF 
              && ( root->left->token.type == SYMBOL || root->left->token.type == QUOTE ) ) {
      // cout << "C-C\n";
      Token sym = root->left->token;
      Tree * symbol = root->left;
      // cout << "2222222222222222";
      resultexpr = Unbound( found, sym );
      if ( found && resultexpr->token.type == PROCEDURE ) {
        // cout << "C-C-A\n";
        if ( !isTopLevel 
             && ( sym.str == "clean-environment" || sym.str == "define" || sym.str == "exit" ) ) {
          // cout << "C-C-A-A\n";
          if ( sym.str == "clean-environment" ) {
            cout << "> ERROR (level of CLEAN-ENVIRONMENT)\n\n";
            throw 0;
          } // if
          else if ( sym.str == "define" ) {
            cout << "> ERROR (level of DEFINE)\n\n";
            throw 0;
          } // else if
          else if ( sym.str == "exit" ) {
            cout << "> ERROR (level of EXIT)\n\n";
            throw 0;
          } // else if
           
        } // if
        else if ( sym.str == "define" || sym.str == "set!" || sym.str == "let" 
                  || sym.str == "cond" || sym.str == "lambda" ) {
          // cout << "C-C-A-B\n";
          // set!, let, lambda project3才有 
          FormatErrorChecking( root );
          resultexpr = Evaluate( root );
          return resultexpr;
          
        } // else if
        else if ( sym.str == "if" || sym.str == "and" || sym.str == "or" ) {
          // cout << "C-C-A-C\n";
          resultexpr = Evaluate( root );
          return resultexpr;
        } // else if
        else {
          // cout << "C-C-A-D\n";
          // SYM is a known function name 'abc' which is nither 
          // 'define' nor 'let' nor 'cond' nor 'lambda' 
          resultexpr = Evaluate( root );
          return resultexpr;
        } // else
        
      } // if
      else {
        // cout << "C-C-B\n";
        // SYM is 'abc' which is not the name of a known function
        if ( !found ) {
          cout << "> ERROR (unbound symbol) : " << sym.str << "\n\n";
          throw 0;
        } // if
        else if ( found && resultexpr->token.type != PROCEDURE ) {
          resultexpr = Eval( symbol, resultexpr, false );
          cout << "> ERROR (attempt to apply non-function) : " ;        // << sym.str << endl;
          PrintSexp( resultexpr, 0 );
          cout << "\n";
          throw 0;
        } // else if
        
      } // else
      
    } // else if
    else {
      // cout << "C-D\n";
      root->left = Eval( root->left, root->left, false );
      if ( root->left->left == NULL && root->left->right == NULL && root->left->token.type == PROCEDURE ) {
        found = Unbound( root->left->token ); // 去看是不是internal function 
        if ( !found ) {
          cout << "> ERROR (unbound symbol) : " << root->left->token.str << "\n\n";
          throw 0;
        } // if
      } // if
      else {
        cout << "> ERROR (attempt to apply non-function) : " ;        // << sym.str << endl;
        PrintSexp( root->left, 0 );
        cout << "\n";
        throw 0;
      } // else
      
      resultexpr = Evaluate( root );
      return resultexpr;
    } // else

  } // else
  
  return NULL;
  
} // Eval()

int main() {
  Token token;
  char ch;
  Initialize( token );
  bool correct = true;
  InitialEnvironment();
  int testNum;
  cin >> testNum;
  cout << "Welcome to OurScheme!\n\n";
  while ( true ) {
    while ( !gLambdaStack.empty() ) {
      gLambdaStack.pop();
    } // while
    
    while ( !gCallStack.empty() ) {
      for ( int i = 0; i < gCallStack.top()->func.parameter.size() ; i++ ) {
        gCallStack.top()->func.parameter.at( i ).exp = NULL;
      } // for
      
      gCallStack.pop();
    } // while
    
    try {
      Tree * expr = new Tree;
      Tree * resultExpr = NULL;
      Initialize( expr, ROOT );

      S_exp( correct, expr );

      if ( !correct ) {
        token = GetToken();
        cout << "> ERROR (unexpected token) : atom or '(' expected when token at Line "
             << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
        throw 0;
      } // if
      else {
        gLine = 0;
        gColumn = 0;
      } // else

      resultExpr = Eval( expr, resultExpr, true );
      if ( resultExpr != NULL ) {
        cout << "> ";
      } // if
      
      PrintSexp( resultExpr, 0 );
      cout << "\n";

    } catch( int err ) {
      while ( gColumn != 0 ) {
        ch = GetTheNextChar();
      } // while
      
      while ( !gUnDealToken.empty() ) {
        gUnDealToken.pop();
      } // while
      
      while ( !gCallStack.empty() ) {
        gCallStack.pop();
      } // while
      
      gLine = 0;
      
    } // catch()

  } // while

} // main()
