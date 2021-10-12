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

using namespace std;

int gLine = 0, gColumn = 0;

enum TokenType {
  EOF_, NONE, LP, RP, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL
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
      token.type = EOF_;
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
            << gLine << " Column " << gColumn - token.str.size() + 1 << " is >>" << token.str << "<<\n";
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

/*
void Preorder(Tree *current){
    if (current != NULL) {                          // if current != NULL
        // cout << "BBB " << current->type << " BBB";
        cout << current->token.str << " \n";   // V
        Preorder(current->left);       // L
        Preorder(current->right);      // R
    }
}
*/

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
  
  if ( node->left == NULL && node->right == NULL ) {
    // 這是一個ATOM
    PrintFormat(node);
  } // if
  else {
    cout << "( ";
    m+=2;
    
    if( current != NULL ) {
      if( current->left != NULL ) {
        PrintSexp( current->left, m );
      } // if
    } // if
    
    while( current->right != NULL ) {
      current = current -> right;
      if( current->left != NULL ) {
        for( int i = 0 ; i < m; i++ ) {
          cout << " ";
        } // for
        PrintSexp( current->left, m );
      } // if
      
    } // while
    
    if( current->token.type != NIL ) {
      for( int i = 0 ; i < m; i++ ) {
        cout << " ";
      } // for
      
      cout << ".\n";
      for( int i = 0 ; i < m; i++ ) {
        cout << " ";
      } // for
      
      PrintFormat(current);
    } // if
    
    m-=2;
    for( int i = 0 ; i < m; i++ ) {
      cout << " ";
    } // for
    
    cout << ")\n";
  } // else
  
} // PrintSexp()

int main() {
  
  Token token;
  char ch;
  Initialize(token);
  bool correct = true;

  cout << "Welcome to OurScheme!\n\n";

  while( true ) {
    cout << "> ";
  	try {
  	  Tree * expr = new Tree;
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
      
      PrintSexp( expr, 0 );
      //cout << expr->left->token.str << endl;
      //cout << expr->right->left->token.str << endl;
      //cout << expr->right->right->left->token.str << endl;
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
