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
  _EOF, NONE, LP, RP, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, PROCEDURE
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

stack<Token> gUnDealToken;
void Eval( Tree * root, Tree * & resultexpr, bool isTopLevel );

bool IsWhiteSpace( char ch ) {
  if ( ch == ' ' || ch == '\t' || ch == '\n' ) {
    return true;
  } // if
  
  return false;
} // IsWhiteSpace()

bool IsSeparator( char ch ) {
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';' ) {
    return true;
  } // if
  
  return false;
} // IsWhiteSpace()

void Initialize( Token & token ) {
  token.type = NONE;
  token.str = "";
} // Initialize()

void Initialize( Token & token, TokenType type, string str ) {
  token.type = type;
  token.str = str;
} // Initialize()

void Initialize( Tree *& tree, NodeType type, Token token ) {
  tree = new Tree;
  tree->type = type;
  tree->token = token;
  tree->left = NULL;
  tree->right = NULL;
} // Initialize()

void Initialize( Tree *& tree, NodeType type ) {
  tree = new Tree;
  tree->type = type;
  Initialize(tree->token);
  tree->left = NULL;
  tree->right = NULL;
} // Initialize()

 char GetTheNextChar() {
  char ch = '\0';
  ch = cin.get();
  if ( ch == EOF ) {
    cout << "ERROR (no more input) : END-OF-FILE encountered\n";
    cout << "Thanks for using OurScheme!";
    exit(0);
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
      token.type = _EOF;
    } // if
    else if( ch == '(' ) {
      token.type = LP;
      token.str = ch;
  	} // else if
  	else if( ch == ')' ) {
        token.type = RP;
        token.str = ch;
  	} // else if
  	else if( ch == '\'' ) {
        token.type = QUOTE;
        token.str = ch;
  	} // else if
  	else if( ch == '\"' ) {
  	  token.str = ch;
  	  ch = cin.peek();
  	  while( ch != '\"' ) {
  	  	if( ch == '\n' ) {
  	      if( gLine == 0 ) {
  	        gLine = 1;  
          } // if
          
          cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " 
          << gLine << " Column " << gColumn + 1 << "\n\n";
          ch = GetTheNextChar();
  	      throw 0;
    		} // if
    		else if( ch == '\\' ) {
    		  ch = GetTheNextChar();
    			char ch1 = cin.peek();
    			if( ch1 == '\"' ) {
    				ch1 = GetTheNextChar();
    				ch = '\"';
    			} // if
    			else if( ch1 == '\\' ) {
    				ch1 = GetTheNextChar();
    				ch = '\\';
    			} // if
    			else if( ch1 == 'n' ) {
    				ch1 = GetTheNextChar();
    				ch = '\n';
    			} // if
    			else if( ch1 == 't' ) {
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
  	  
  	  if( ch == '\"' ) {
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
      	if( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
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
        
      if( dotCount == 0 ) {
        token.type = INT;
      } // if
      else if( dotCount == 1 ) {
      	token.type = FLOAT;
      } // else if
      else {
      	token.type = SYMBOL;
      } // else
      
      if( isSymbol ) {
      	token.type = SYMBOL;
      } // if
  
    } // else if
    else if( ch == '#' ) {
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        ch = GetTheNextChar();
  	    token.str += ch;
  	    ch = cin.peek();
  	  } // while
  	  
  	  if( token.str == "#f" ) {
  	  	token.type = NIL;
  	  } // if
  	  else if( token.str == "#t" ) {
  	  	token.type = T;
  	  } // #t
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
  	  
  	  if( token.str == "nil" ) {
  	  	token.type = NIL;
  	  } // if
  	  else if( token.str == "t" ) {
  	  	token.type = T;
  	  } // #t
  	  else {
  	  	token.type = SYMBOL;
  	  } // else
  	  
    } // else if
    else if( ch == '+' ) {
      int dotCount = 0;
      bool isSymbol = false;
      bool hasNum = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
        if( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
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
        
      if( dotCount == 0 ) {
        token.type = INT;
  	  } // if
  	  else if( dotCount == 1 ) {
  	  	if( hasNum ) {
  	      token.type = FLOAT;
        } // if
        else {
          token.type = SYMBOL;
        } // else
        
  	  } // else if
  	  else {
  	  	token.type = SYMBOL;
  	  } // else
  	  
  	  if( token.str.length() == 1 ) { // only +
  	    isSymbol = true;
  	  } // if
  	  
  	  if( isSymbol ) {
  	  	token.type = SYMBOL;
  	  } // if
  	  
  	} // else if
  	else if( ch == '-' ) {
      int dotCount = 0;
      bool isSymbol = false;
      bool hasNum = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
      	if( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
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
        
      if( dotCount == 0 ) {
        	token.type = INT;
  	  } // if
  	  else if( dotCount == 1 ) {
  	    if( hasNum ) {
  	      token.type = FLOAT;
        } // if
        else {
          token.type = SYMBOL;
        } // else
        
  	  } // else if
  	  else {
  	  	token.type = SYMBOL;
  	  } // else
  	  
  	  if( token.str.length() == 1 ) { // only -
  	    isSymbol = true;
  	  } // if
  	  
  	  if( isSymbol ) {
  	  	token.type = SYMBOL;
  	  } // if
  	  
  	} // else if
  	else if( ch == '.' ) {
      int dotCount = 1;
      bool isSymbol = false;
      token.str = ch;
      ch = cin.peek();
      while ( !IsSeparator( ch ) ) {
      	if( ( ch >= '0' && ch <= '9' ) || ch == '.' ) {
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
        
      if( dotCount == 0 ) {
        	token.type = INT;
  	  } // if
  	  else if( dotCount == 1 ) {
  	  	token.type = FLOAT;
  	  } // else if
  	  else {
  	  	token.type = SYMBOL;
  	  } // else
  	  
  	  if( token.str.length() == 1 ) { // only .
  	    isSymbol = false;
  	    token.type = DOT;
  	  } // if
  	  
  	  if( isSymbol ) {
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

string EnumToString(TokenType value) {
  string table[] = { "_EOF", "NONE", "LP", "RP", "INT", "STRING", "DOT", "FLOAT", "NIL", "T", "QUOTE", "SYMBOL" };
  return table[value];
} // EnumToString()

void Atom( bool & correct, Tree *& node );
void S_exp( bool & correct, Tree *& node );
void Preorder(Tree *current);

// 總共有四個可能會出現的error：

//  ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
//  ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
//  ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
//  ERROR (no more input) : END-OF-FILE encountered


void S_exp( bool & correct, Tree *& node ) {
  // <S-exp> ::= <ATOM> 
  //        | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
  //        | QUOTE <S-exp>
  //cout << "S_exp\n";
  Token token;
  correct = true;
  bool s_expCorrect = true;
  bool atomCorrect = true;
  Tree * current = node;
  if( current == NULL ) {
    Initialize( current, ROOT );
  } // if
  
  Atom( atomCorrect, node );
  if( atomCorrect ) { // <ATOM>
    return;
  } // if
  
	PeekToken();
	
  if( gUnDealToken.top().type == LP ) {
    token = GetToken();
    current->left = new Tree;
    Initialize( current->left, ROOT );
    S_exp( s_expCorrect, current->left );
    if( !s_expCorrect ) {
      correct = false;
      token = GetToken();
      cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
           << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
      throw 0;
    } // if

    while( correct ) {
      Tree * lastCurrent = current;
      Tree*temp = new Tree;
      Initialize( temp, INTERNAL );
      current->right = temp;
      current = current->right;
      current->left = new Tree;
      Initialize( current->left, ROOT );
      S_exp( correct, current->left );
      
      if( !correct ) {
        current = lastCurrent;
        lastCurrent->right = NULL;
      } // if

    } // while
    
    correct = true;
    PeekToken();
    
    if( gUnDealToken.top().type == DOT ) {
      token = GetToken();
      current->right = new Tree;
      Initialize( current->right, INTERNAL );
      S_exp( s_expCorrect, current->right );
      if( !s_expCorrect ) {
        correct = false;
        token = GetToken();
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
             << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
        throw 0;
      } // if
      
    } // if
    else { // 補 NIL 
      current->right = new Tree;
      Token ttemp;
      Initialize( ttemp, NIL, "nil" );
      Initialize( current->right, LEAF, ttemp );
      //current->right->token.str = "nil";
      //current->right->token.type = NIL;
    } // else
    
    PeekToken();
    if( gUnDealToken.top().type == RP ) {
      token = GetToken();
    } // if
    else {
      token = GetToken();
      cout << "ERROR (unexpected token) : ')' expected when token at Line " << gLine 
      << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n" ;
      throw 0;
    } // else
	
  } // if
  else if( gUnDealToken.top().type == QUOTE ) {
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
    Initialize( current->right->right, LEAF, ttemp );
    
    if( !s_expCorrect ) {
      correct = false;
      token = GetToken();
      cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
            << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n"; // \n
      throw 0;
    } // if
    
    node = current;

  } // else if
  else {
    correct = false;
  } // else

} // S_exp()

void Atom( bool & correct, Tree *& node ) {
  //<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
  //        | NIL | T | LEFT-PAREN RIGHT-PAREN
  //cout << "Atom\n";
  Token token;
  correct = true;
  Tree * temp = NULL;
  PeekToken();
  if( gUnDealToken.top().type == SYMBOL || gUnDealToken.top().type == INT || gUnDealToken.top().type == FLOAT 
      || gUnDealToken.top().type == STRING || gUnDealToken.top().type == NIL || gUnDealToken.top().type == T ) {
    token = GetToken();
    temp = new Tree;
    Initialize( temp, LEAF, token );
    node = temp;
  } // if
  else if( gUnDealToken.top().type == LP ) {
    token = GetToken();
    PeekToken();
    if( gUnDealToken.top().type != RP ) {
      gUnDealToken.push(token);
      correct = false;
      return;
    } // if
		
    token = GetToken();
    token.type = NIL;
    token.str = "nil";
    temp = new Tree;
    Initialize( temp, LEAF, token );
    node = temp;
  } // else if
  else {
    node = NULL;
    correct = false;
    return;
  } // else
  
} // Atom()

void PrintFormat( Tree * node ) {
  //<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
  //        | NIL | T | LEFT-PAREN RIGHT-PAREN
  if ( node->token.type == INT ) {
    cout << atoi(node->token.str.c_str());
  } // if
  else if( node->token.type == STRING ) {
    cout << node->token.str;
  } // else if
  else if( node->token.type == SYMBOL ) {
    cout << node->token.str;
  } // else if
  else if( node->token.type == NIL ) {
    cout << "nil";
  } // else if
  else if( node->token.type == T ) {
    cout << "#t";
  } // else if
  else if( node->token.type == QUOTE ) {
    cout << "quote";
  } // else if
  else if( node->token.type == FLOAT ) {
    cout << fixed << setprecision( 3 ) << atof(node->token.str.c_str());
  } // else if
  else if( node->token.type == PROCEDURE ){
    cout << node->token.str;
  } // else
  
  cout << "\n";
} // PrintFormat()

bool Exit( Tree * expr ) {
 if ( expr->type == ROOT ) {
   if ( expr->left->token.str == "exit" && expr->right->token.type == NIL ) {
     if ( expr->left->left == NULL && expr->left->right == NULL
          && expr->right->left == NULL && expr->right->right == NULL ) {
        return true;
     } // if
     
   } // if
   
 } // if
 
 return false;
} // exit()


void PrintSexp( Tree * node, int m ) {
  bool isSexp = false;
  bool isQuote = false;
  Tree * current = node;
  if( node == NULL ) {
    return;
  } // if
  else if ( node->left == NULL && node->right == NULL ) {
    // 這是一個ATOM
    PrintFormat(node);
  } // if
  else {
    if( current->type == ROOT ) {
      isSexp = true;
      if( current->left->token.type == QUOTE ) {
        isQuote = true;
      } // if
      
      // cout << "AAAAA\n";
      cout << "( ";
      m+=2;
    } // if
      
    // 每經過一個龍骨就往左邊走 
    if ( current->type == ROOT ) {
      PrintSexp( current->left, m );
    } // if
    else {
      for( int i = 0 ; i < m; i++ ) {
        cout << " ";
      } // for
      
      PrintSexp( current->left, m );
    } // else
      
    if ( current->right->type != LEAF ) {
      // 往龍骨右邊走
      
      PrintSexp( current->right, m );
    } // if
    else {
      current = current->right;
      if ( current->token.type != NIL ) {
        if( !isQuote ) {
          for( int i = 0 ; i < m; i++ ) {
            cout << " ";
          } // for
          
          cout << ".\n";
        } // if
        
        for( int i = 0 ; i < m; i++ ) {
          cout << " ";
        } // for
        
        PrintFormat(current);
      } // if
      else {
        if( isQuote ) {
          
          for( int i = 0 ; i < m; i++ ) {
            cout << " ";
          } // for
          
          PrintFormat(current);
        } // if
      } // else
      
    } // else
    
    if ( isSexp ) {
      m-=2;
      for( int i = 0 ; i < m; i++ ) {
        cout << " ";
      } // for
      cout << ")\n";
    } // if
    
  } // else
  
} // PrintSexp()

struct Symbol {
  string name;
  Tree * exp;
};

vector <Symbol> symbolTable;
vector <Symbol> internalFunctionTable;
vector <Symbol> systemFunctionTable;

void PutSymbolIntoSymbolTable( Symbol newSymbol, string funcName, string tokenStr ) {
  Tree * temp = new Tree;
  temp->left = NULL;
  temp->right = NULL;
  temp->type = ROOT;
  temp->token.str = tokenStr;
  temp->token.type = PROCEDURE;
  newSymbol.exp = temp;
  newSymbol.name = funcName;
  symbolTable.push_back(newSymbol);
  //internalFunctionTable.push_back(newSymbol);
  systemFunctionTable.push_back(newSymbol);
} // putInSymbol

void InitialEnvironment() {
  // #<procedure funcname>
  Tree * temp;
  Symbol newSymbol;
  symbolTable.clear();
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
} // InitialEnvironment()

bool Unbound( Token token, Tree *& resultexpr ) {
  resultexpr = NULL;
  for( int i = 0; i < symbolTable.size(); i++ ) {
    //cout << "\t" << token.str << "\t" << symbolTable.at(i).name << "\t" << symbolTable.at(i).exp->token.type << "\t" << symbolTable.at(i).exp << endl;
    if( token.str == symbolTable.at(i).name ) {
      resultexpr = symbolTable.at(i).exp;
      return false;
    } // if
    
  } // for
  
  return true;
} // Unbound()

bool IsPureList( Tree * root ) {
  // 龍骨最右下角不是nil 
  Tree * cur = root;
  while( cur->right != NULL ) {
    cur = cur->right;
  } // while
  
  if( cur->token.type != NIL ) {
    return false;
  } // if
  
  return true;
} // IsPureList()

vector <Tree *> GetArgs( Tree * root ) {
  Tree * cur = root;
  vector <Tree *> args;
  if( cur != NULL ) {
    while( cur->type != LEAF ) {
      args.push_back( cur->left );
      cur = cur->right;
    } // while 
  } // if
  
  return args;
} // GetArgs()

void ThrowFormatException( Tree * root ) {
  // 可增加toUpperCaseFunction
  string funcName = root->left->token.str;
  if( funcName == "define" ) {
    cout << "ERROR (DEFINE format) : ";
    PrintSexp( root, 0 );
    
  } // if
  else if( funcName == "set!" ) {
    cout << "ERROR (SET! format) : ";
    PrintSexp( root, 0 );
  } // else if
  else if( funcName == "lambda" ) {
    cout << "ERROR (LAMBDA format) : ";
    PrintSexp( root, 0 );
  } // else if
  else if( funcName == "let" ) {
    cout << "ERROR (LET format) : ";
    PrintSexp( root, 0 );
  } // else if
  else if( funcName == "cond" ) {
    cout << "ERROR (COND format) : ";
    PrintSexp( root, 0 );
  } // else if
  
  cout << "\n\n";
  throw 0;
} // ThrowFormatException()

bool Redefine( string funcName, vector<Symbol> table ) {
  for( int i = 0; i < table.size(); i++ ) {
    if( funcName == table.at(i).name ) {
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
  if( funcName == "define" ) {
    if( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    if( args.at(0)->type == ROOT ) {
      symbols = GetArgs( args.at(0) );
    } // if
    else {
      symbols.push_back( args.at(0) );
    } // else
    
    for( int i = 1; i < args.size(); i++ ) {
      sexps.push_back( args.at(i) );
    } // for
    
    for( int i = 0; i < symbols.size(); i++ ) {
      if( symbols.at(i)->token.type != SYMBOL ) {
        ThrowFormatException( root );
      } // if
    } // for
    
    for( int i = 0; i < symbols.size(); i++ ) {
      for( int j = 0; j < systemFunctionTable.size(); j++ ) {
        if( symbols.at(i)->token.str == systemFunctionTable.at(j).name ) {
          ThrowFormatException( root );
        } // if
      } // for
    } // for
    
  } // if
  else if( funcName == "set!" ) {
    
  } // else if
  else if( funcName == "lambda" ) {
    
  } // else if
  else if( funcName == "let" ) {
    
  } // else if
  else if( funcName == "cond" ) {
    
  } // else if
  
} // FormatErrorChecking()

void DEBUG_ARGLIST( vector <Tree *> args ) {
  cout << "DEBUG_ARGLIST\n";
  for( int i = 0; i < args.size(); i++ ) {
    PrintSexp( args.at(i), 0 );
  } // for
} // DEBUG_ARGLIST()

Tree * Cons( vector <Tree *> args ) {
  Tree * root = new Tree;
  root->type = ROOT;
  Initialize(root->token);
  cout << "EVAL ARG1\n";
  Eval( args.at(0), root->left, false );

  if( root->left->left == NULL && root->left->right == NULL ) {
    root->left->type = LEAF;
  } // if
  else {
    root->left->type = ROOT;
  } // else
  
  cout << "EVAL ARG2\n";
  Eval( args.at(1), root->right, false );
  if( root->right->left == NULL && root->right->right == NULL ) {
    root->right->type = LEAF;
  } // if
  else {
    root->right->type = INTERNAL;
  } // else
  
  return root;
} // Cons()

Tree * Quote( vector <Tree *> args ) {
  Tree * root = new Tree;
  Initialize(root->token);
  cout << "EVAL ARG1\n";
  root = args.at(0);
  root->type = ROOT;
  return root;
} // Quote()

Tree * List( vector <Tree *> args ) {
  // 應改成Unbound先 
  Tree * root = new Tree;
  Initialize( root, ROOT );
  Tree * temp = new Tree;
  Tree * cur = root;
  Token nilToken;
  Initialize( nilToken, NIL, "nil" );
  cout << "EVAL ARGs\n";
  if( args.size() == 0 ) {
    Token noneToken;
    Initialize( noneToken, NONE, "" );
    Initialize( temp, LEAF, noneToken );
    root->left = temp;
    Initialize( root->right, LEAF, nilToken );
  } // if
  
  
  for( int i = 0; i < args.size(); i++ ) {
    //cout << i << endl;
    Eval( args.at(i), temp, false );
    cur->left = temp;
    if( i != args.size() - 1 ) {
      cur->right = new Tree;
      Initialize( cur->right, INTERNAL );
      cur = cur->right;
    } // if
    
  } // for
  
  Initialize( cur->right, LEAF, nilToken );
  
  return root;
} // List()

Tree * Define( Tree * root ) {
  string symbolName;
  Symbol symbol;
  bool redefine = false;
  vector <Tree *> args = GetArgs( root->right );
  Tree * sym = args.at(0);
  vector<Tree *> symbols;
  vector<Tree *> sexps;
  if( args.at(0)->type == ROOT ) {
    cout << "function definition\n";
    symbols = GetArgs( args.at(0) );
    for( int i = 1; i < args.size(); i++ ) {
      sexps.push_back( args.at(i) );
    } // for
    
    
  } // if
  else if( args.at(0)->type == LEAF ) {
    cout << "symbol definition\n";
    symbols.push_back( args.at(0) );
    for( int i = 1; i < args.size(); i++ ) {
      sexps.push_back( args.at(i) );
    } // for
    
    if( sexps.size() != 1 ) {
      ThrowFormatException(root);
    } // if
    
    
    symbol.name = symbols.at(0)->token.str;
    Eval( sexps.at(0), symbol.exp, false );
    symbol.exp->type = ROOT;
  
    
    for( int i = 0; i < symbolTable.size(); i++ ) {
      if( symbol.name == symbolTable.at(i).name ) {
        // redefine
        redefine = true;
        symbolTable.at(i).exp = symbol.exp;
      } // if
    } // for
    
    if( !redefine ) {
      symbolTable.push_back( symbol );
    } // if
    
  } // else if
  
  cout << symbol.name << " defined\n";
  return NULL;
} // Define()

Tree * Car( vector <Tree *> args ) {
  Tree * root = new Tree;
  Tree * temp = new Tree;
  Initialize( root, ROOT );
  Eval( args.at(0), temp, false );
  // 修改前code在projrct2_newquote_procedure 
  if( temp->right == NULL ) { // 不是pair 
    cout << "ERROR (car with incorrect argument type) : " ;
    PrintSexp( temp, 0 );
    cout <<  "\n";
    throw 0;
  } // if
  else {
    root = temp->left;
    root->type = ROOT;
  } // else
  
  return root;
} // Car()

Tree * Cdr( vector <Tree *> args ) {
  Tree * root = new Tree;
  Tree * temp = new Tree;
  Initialize( root, ROOT );
  Eval( args.at(0), temp, false );
  //PrintSexp( temp, 0 );
  // 修改前code在projrct2_newquote_procedure 
  if( temp->right == NULL ) { // 不是pair 
    cout << "ERROR (car with incorrect argument type) : " ;
    PrintSexp( temp, 0 );
    cout <<  "\n";
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
  Eval( args.at(0), result, false );
  if( result->left == NULL && result->right == NULL && result->token.type != SYMBOL ) {
    return True();
  } // if
  
  return False();
} // IsAtom()

Tree * IsPair( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  if( result->right != NULL ) {
    return True();
  } // if
  
  return False();
} // IsPair()

Tree * IsList( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  if( IsPureList( result ) ) {
    return True();
  } // if
  
  return False();
} // IsList()

Tree * IsNull( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && result->token.type == NIL ) {
    return True();
  } // if

  return False();
} // IsNull()

Tree * IsInteger( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && result->token.type == INT ) {
    return True();
  } // if

  return False();
} // IsInteger()

Tree * IsReal( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && ( result->token.type == INT || result->token.type == FLOAT ) ) {
    return True();
  } // if

  return False();
} // IsReal()

Tree * IsNumber( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && ( result->token.type == INT || result->token.type == FLOAT ) ) {
    return True();
  } // if

  return False();
} // IsNumber()

Tree * IsString( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && result->token.type == STRING ) {
    return True();
  } // if

  return False();
} // IsString()

Tree * IsSymbol( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && result->token.type == SYMBOL ) {
    return True();
  } // if

  return False();
} // IsSymbol()

Tree * IsBoolean( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  
  if( result->left == NULL && result->right == NULL && ( result->token.type == NIL || result->token.type == T ) ) {
    return True();
  } // if

  return False();
} // IsBoolean()

Tree * Add( vector <Tree *> args ) {
  // 應改成Unbound先 
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize(token, INT, "0");
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  // atof(node->token.str.c_str())
  // 先確認unbound 
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
  } // for
  
  
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
    if( number->left != NULL && number->right != NULL ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n"; 
      throw 0;
    } // if
    else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n";
      throw 0;
    } // else if
    
    if( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    //cout << resultValue << "\t";
    //cout << atof( number->token.str.c_str() ) << "\t";
    resultValue = resultValue + atof( number->token.str.c_str() );
    //cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if( result->token.type == INT ) {
    //將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    //string num_str = sstream.str();
  } // if
  else if( result->token.type == FLOAT ) {
    //將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Add()

Tree * Sub( vector <Tree *> args ) {
  // 應改成Unbound先 
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize(token, INT, "0");
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  
  // 先確認unbound 
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
  } // for
  
  
  Eval( args.at(0), number, false );
  if( number->left != NULL && number->right != NULL ) {
    cout << "ERROR (+ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n\n"; 
    throw 0;
  } // if
  else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
    cout << "ERROR (+ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n\n";
    throw 0;
  } // else if
  
  if( number->token.type == FLOAT ) {
    result->token.type = FLOAT;
  } // if
  
  resultValue = atof( number->token.str.c_str() );
  
  // atof(node->token.str.c_str())
  for( int i = 1; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
    if( number->left != NULL && number->right != NULL ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n"; 
      throw 0;
    } // if
    else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n";
      throw 0;
    } // else if
    
    if( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    //cout << resultValue << "\t";
    //cout << atof( number->token.str.c_str() ) << "\t";
    resultValue = resultValue - atof( number->token.str.c_str() );
    //cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if( result->token.type == INT ) {
    //將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    //string num_str = sstream.str();
  } // if
  else if( result->token.type == FLOAT ) {
    //將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Sub()

Tree * Mult( vector <Tree *> args ) {
  // 應改成Unbound先 
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize(token, INT, "0");
  Initialize( result, ROOT, token );
  float resultValue = 1;
  // atof(node->token.str.c_str())
  // 先確認unbound 
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
  } // for
  
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
    if( number->left != NULL && number->right != NULL ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n"; 
      throw 0;
    } // if
    else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n";
      throw 0;
    } // else if
    
    if( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    //cout << resultValue << "\t";
    //cout << atof( number->token.str.c_str() ) << "\t";
    resultValue = resultValue * atof( number->token.str.c_str() );
    //cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if( result->token.type == INT ) {
    //將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    //string num_str = sstream.str();
  } // if
  else if( result->token.type == FLOAT ) {
    //將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Mult()

Tree * Div( vector <Tree *> args ) {
  // 應改成Unbound先 
  Tree * number = new Tree;
  Tree * result = new Tree;
  Token token;
  Initialize(token, INT, "0");
  Initialize( result, ROOT, token );
  float resultValue = 0.0;
  // 先確認unbound 
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
  } // for
  
  Eval( args.at(0), number, false );
  if( number->left != NULL && number->right != NULL ) {
    cout << "ERROR (+ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n\n"; 
    throw 0;
  } // if
  else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
    cout << "ERROR (+ with incorrect argument type) : ";
    PrintSexp( number, 0 );
    cout << "\n\n";
    throw 0;
  } // else if
  
  if( number->token.type == FLOAT ) {
    result->token.type = FLOAT;
  } // if
  
  resultValue = atof( number->token.str.c_str() );
  
  // atof(node->token.str.c_str())
  for( int i = 1; i < args.size(); i++ ) {
    Eval( args.at(i), number, false );
    if( number->left != NULL && number->right != NULL ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n"; 
      throw 0;
    } // if
    else if( number->left == NULL && number->right == NULL && number->token.type != INT && number->token.type != FLOAT ) {
      cout << "ERROR (+ with incorrect argument type) : ";
      PrintSexp( number, 0 );
      cout << "\n\n";
      throw 0;
    } // else if
    
    if( number->token.type == FLOAT ) {
      result->token.type = FLOAT;
    } // if
    
    //cout << resultValue << "\t";
    //cout << atof( number->token.str.c_str() ) << "\t";
    if( atof( number->token.str.c_str() ) == 0 ) {
      cout << "ERROR (division by zero) : /\n\n";
      throw 0;
    } // if
    resultValue = resultValue / atof( number->token.str.c_str() );
    //cout << "HHH " << resultValue << " HHH\n"; 
  } // for
  
  // 將 resultValue 轉成str
  if( result->token.type == INT ) {
    //將 resultValue 轉成int再轉str
    stringstream sstream;
    sstream << ( int ) resultValue;
    result->token.str = sstream.str();
    
    //string num_str = sstream.str();
  } // if
  else if( result->token.type == FLOAT ) {
    //將 resultValue 直接轉str
    stringstream sstream;
    sstream << resultValue;
    result->token.str = sstream.str();
  } // if

  return result;
} // Div()

Tree * Not( vector <Tree *> args ) {
  Tree * result = new Tree;
  Eval( args.at(0), result, false );
  if( result->left == NULL && result->right == NULL && result->token.type == NIL ) {
    return True();
  } // if
  
  return False();
} // Not()



Tree * Greater( vector <Tree *> args ) {
  Tree * result = True();
  Tree * bigger = new Tree;
  Tree * smaller = new Tree;
  //bool res = false;
  vector <Tree *> argsAfterEval;
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), result, false );
    argsAfterEval.push_back(result);
  } // for
  
  for( int i = 0; i < argsAfterEval.size(); i++ ) {
    if( i + 1 != argsAfterEval.size() ) {
      bigger = argsAfterEval.at(i);
      smaller = argsAfterEval.at(i+1);
      
      if( bigger->left != NULL || bigger->right != NULL 
          || ( bigger->token.type != INT && bigger->token.type != FLOAT ) ) {
        cout << "ERROR (> with incorrect argument type) : ";
        PrintSexp( bigger, 0 );
        cout << "\n\n";
        throw 0;
      } // if
      
      if( smaller->left != NULL || smaller->right != NULL 
          || ( smaller->token.type != INT && smaller->token.type != FLOAT ) ) {
        cout << "ERROR (> with incorrect argument type) : ";
        PrintSexp( smaller, 0 );
        cout << "\n\n";
        throw 0;
      } // if

      if( atof( bigger->token.str.c_str() ) < atof( smaller->token.str.c_str() ) ) {
        result = False();
      } // if
      
    } // if
    
  } // for
  
  return result;
} // Greater()

Tree * Ge( vector <Tree *> args ) {
  Tree * result = new Tree;
  vector <Tree *> argsAfterEval;
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), result, false );
    argsAfterEval.push_back(result);
  } // for
  
  return False();
} // Ge()

Tree * Less( vector <Tree *> args ) {
  Tree * result = new Tree;
  vector <Tree *> argsAfterEval;
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), result, false );
    argsAfterEval.push_back(result);
  } // for
  
  return False();
} // Less()

Tree * Le( vector <Tree *> args ) {
  Tree * result = new Tree;
vector <Tree *> argsAfterEval;
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), result, false );
    argsAfterEval.push_back(result);
  } // for
  
  return False();
} // Le()

Tree * Equ( vector <Tree *> args ) {
  Tree * result = new Tree;
vector <Tree *> argsAfterEval;
  for( int i = 0; i < args.size(); i++ ) {
    Eval( args.at(i), result, false );
    argsAfterEval.push_back(result);
  } // for
  
  return False();
} // Equ()


Tree * Evaluate( Tree * root ) {
  vector <Tree *> args = GetArgs( root->right );
  string funcName;
  DEBUG_ARGLIST( args );
  Tree * procedure = new Tree;
  Eval( root->left, procedure, false );
  funcName = procedure->token.str;
  cout << "Evaluate " <<  funcName << endl ;
  
  if( funcName == "#<procedure cons>" ) {
    if( args.size() != 2 ) {
      cout << "ERROR (incorrect number of arguments) : cons\n\n";
      throw 0;
    } // if
    
    return Cons( args );
  } // if
  else if( funcName == "#<procedure list>" ) {
    return List( args );
  } // else if
  else if( funcName == "#<procedure quote>" ) {
    //cout << "aaaaaaa";
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : quote\n\n";
      throw 0;
    } // if
    
    return Quote( args );
  } // else if
  else if( funcName == "#<procedure define>" ) {
    if( args.size() < 2 ) {
      ThrowFormatException( root );
    } // if
    
    return Define( root );
  } // else if
  else if( funcName == "#<procedure car>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : car\n\n";
      throw 0;
    } // if
    
    return Car( args );
  } // else if
  else if( funcName == "#<procedure cdr>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : cdr\n\n";
      throw 0;
    } // if
    
    return Cdr( args );
  } // else if
  else if( funcName == "#<procedure atom?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : atom?\n\n";
      throw 0;
    } // if
    
    return IsAtom( args );
  } // else if
  else if( funcName == "#<procedure pair?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : pair?\n\n";
      throw 0;
    } // if
    
    return IsPair( args );
  } // else if
  else if( funcName == "#<procedure list?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : list?\n\n";
      throw 0;
    } // if
    
    return IsList( args );
  } // else if
  else if( funcName == "#<procedure null?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : null?\n\n";
      throw 0;
    } // if
    
    return IsNull( args );
  } // else if
  else if( funcName == "#<procedure integer?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : integer?\n\n";
      throw 0;
    } // if
    
    return IsInteger( args );
  } // else if
  else if( funcName == "#<procedure real?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : real?\n\n";
      throw 0;
    } // if
    
    return IsReal( args );
  } // else if
  else if( funcName == "#<procedure number?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : number?\n\n";
      throw 0;
    } // if
    
    return IsNumber( args );
  } // else if
  else if( funcName == "#<procedure string?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : string?\n\n";
      throw 0;
    } // if
    
    return IsString( args );
  } // else if
  else if( funcName == "#<procedure boolean?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : boolean?\n\n";
      throw 0;
    } // if
    
    return IsBoolean( args );
  } // else if
  else if( funcName == "#<procedure symbol?>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : symbol?\n\n";
      throw 0;
    } // if
    
    return IsSymbol( args );
  } // else if
  else if( funcName == "#<procedure +>" ) {
    if( args.size() < 2 ) {
      cout << "ERROR (incorrect number of arguments) : +\n\n";
      throw 0;
    } // if
    
    return Add( args );
  } // else if
  else if( funcName == "#<procedure ->" ) {
    if( args.size() < 2 ) {
      cout << "ERROR (incorrect number of arguments) : -\n\n";
      throw 0;
    } // if
    
    return Sub( args );
  } // else if
  else if( funcName == "#<procedure *>" ) {
    if( args.size() < 2 ) {
      cout << "ERROR (incorrect number of arguments) : *\n\n";
      throw 0;
    } // if
    
    return Mult( args );
  } // else if
  else if( funcName == "#<procedure />" ) {
    if( args.size() < 2 ) {
      cout << "ERROR (incorrect number of arguments) : /\n\n";
      throw 0;
    } // if
    
    return Div( args );
  } // else if
  else if( funcName == "#<procedure not>" ) {
    if( args.size() != 1 ) {
      cout << "ERROR (incorrect number of arguments) : not\n\n";
      throw 0;
    } // if
    
    return Not( args );
  } // else if
  else if( funcName == "#<procedure >>" ) {
    if( args.size() < 2 ) {
      cout << "ERROR (incorrect number of arguments) : not\n\n";
      throw 0;
    } // if
    
    return Greater( args );
  } // else if
  
} // Evaluate()



void Eval( Tree * root, Tree * & resultexpr, bool isTopLevel ) {
  cout << "Eval\n" ;
  bool unbound = false;
  if( root->left == NULL && root->right == NULL && root->token.type != SYMBOL && root->token.type != QUOTE ) {
    cout << "DEBUG AAAAA\n";
    resultexpr = root;
    return;
  } // if
  else if( root->left == NULL && root->right == NULL && ( root->token.type == SYMBOL || root->token.type == QUOTE ) ) {
    cout << "DEBUG BBBBB\n";
    //cout << root->token.str;
    unbound = Unbound( root->token, resultexpr );
    
    if( unbound ) {
      cout << "ERROR (unbound symbol) : " << root->token.str << "\n\n";
      throw 0;
    } // if
    else {
      return;
    } // else
    
  } // else if
  else {
    cout << "DEBUG CCCCC\n";
    if( !IsPureList(root) ) {
      cout << "DEBUG C-A\n";
      cout << "ERROR (non-list) : ";
      PrintSexp( root, 0 );
      cout << "\n\n";
      throw 0;
    } // if
    else if( root->left->type == LEAF && root->left->token.type != SYMBOL && root->left->token.type != QUOTE ) {
      cout << "DEBUG C-B\n";
      cout << "ERROR (attempt to apply non-function) : " << root->left->token.str << "\n\n" ;
      throw 0;
    } // else if
    else if( root->left->type == LEAF && ( root->left->token.type == SYMBOL || root->left->token.type == QUOTE ) ) {
      cout << "DEBUG C-C\n";
      Token sym = root->left->token;
      Tree * symbol = root->left;
      
      unbound = Unbound( sym, resultexpr );
      if( !unbound && resultexpr->token.type == PROCEDURE ) {
        cout << "DEBUG C-C-A\n";
        if( !isTopLevel && ( sym.str == "clean-environment" || sym.str == "define" || sym.str == "exit" ) ) {
          cout << "DEBUG C-C-A-A\n";
          if( sym.str == "clean-environment" ) {
            cout << "ERROR (level of CLEAN-ENVIRONMENT)\n\n";
            throw 0;
          } // if
          else if( sym.str == "define" ) {
            cout << "ERROR (level of DEFINE)\n\n";
            throw 0;
          } // else if
          else if( sym.str == "exit" ){
            cout << "ERROR (level of EXIT)\n\n";
            throw 0;
          } // else
           
        } // if
        else if( sym.str == "define" || sym.str == "set!" || sym.str == "let" || sym.str == "cond" || sym.str == "lambda" ) {
          // set!, let, lambda project3才有 
          cout << "DEBUG C-C-A-B\n";
          // check format;
          
          FormatErrorChecking( root );
          resultexpr = Evaluate( root );
          
          
        } // else
        else if( sym.str == "if" || sym.str == "and" || sym.str == "or" ) {
          cout << "DEBUG C-C-A-C\n";
        } // else if
        else {
          // SYM is a known function name 'abc' which is nither 
          // 'define' nor 'let' nor 'cond' nor 'lambda' 
          cout << "DEBUG C-C-A-D\n";
          resultexpr = Evaluate( root );

          return;
        } // else
        
      } // if
      else {
        cout << "DEBUG C-C-B\n";
        // SYM is 'abc' which is not the name of a known function
        if( unbound ) {
          cout << "ERROR (unbound symbol) : " << sym.str << "\n\n";
          throw 0;
        } // if
        else if( !unbound && resultexpr->token.type != PROCEDURE ) {
          Eval( symbol, resultexpr, false );
          cout << "ERROR (attempt to apply non-function) : " ;        // << sym.str << endl;
          PrintSexp( resultexpr, 0 );
          cout << "\n\n";
          throw 0;
        } // else if
        
      } // else
      
    } // esle if
    else {
      cout << "DEBUG C-D\n";
      
    } // else
    
  } // else
  
} // Eval()



int main() {

  Token token;
  char ch;
  Initialize(token);
  bool correct = true;
  InitialEnvironment();
  cout << "Welcome to OurScheme!\n\n";

  while( true ) {
    cout << "> ";
  	try {
  	  Tree * expr = new Tree;
  	  Tree * resultExpr = NULL;
      Initialize( expr, ROOT );
      
  		S_exp( correct, expr );
  		if( !correct ) {
  		  token = GetToken();
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
            << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n\n";
  		  throw 0;
      } // if
      else {
        gLine = 0;
        gColumn = 0;
      } // else
      
      if( Exit(expr) ) {
        cout << "\nThanks for using OurScheme!";
        exit(0);
      } // if
      
      //cout << expr->left->token.type << "SSS";
      
      Eval( expr, resultExpr, true );
      PrintSexp( resultExpr, 0 );
      cout << "\n";

	  } catch( int err ) {
  		while( gColumn != 0 ) {
  		  ch = GetTheNextChar();
      } // while
      
      while( !gUnDealToken.empty() ) {
        gUnDealToken.pop();
      } // while
      
      gLine = 0;
    } // catch()
  	
  } // while

} // main()
