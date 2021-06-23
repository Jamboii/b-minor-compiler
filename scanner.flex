%{
#include "token.h"
#include <limits.h>
int yyinput();
char buffer[256]; // holds the contents of our stripped/modified string or character literal
int buf_idx = 0;  // holds the latest index of the string/character literal we're assigning characters to
int null_cnt = 0; // carries the amount of null characters \0 in a string or character literal
%}
DIGIT  [0-9]
LETTER [a-zA-Z]
%x IN_COMMENT
%%
(" "|\t|\n)  /* skip whitespace */

<INITIAL>{
	"/*"	BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
	"*/"	BEGIN(INITIAL);
	[^*\n]+	// eat comment in chunks
	"*"		// eat the lone star
	\n 		yylineno++;
	<<EOF>> return TOKEN_ERROR;
}

[/][/].*   {}

array      { return TOKEN_ARRAY;    }
boolean    { return TOKEN_BOOLEAN;  }
char       { return TOKEN_CHAR;     }
if         { return TOKEN_IF;       }
else       { return TOKEN_ELSE;     }
true       { return TOKEN_TRUE;     }
false      { return TOKEN_FALSE;    }
for        { return TOKEN_FOR;      }
function   { return TOKEN_FUNCTION; }
integer    { return TOKEN_INTEGER;  } 
print      { return TOKEN_PRINT;    }
return     { return TOKEN_RETURN;   }
string     { return TOKEN_STRING;   }
void       { return TOKEN_VOID;     }
auto       { return TOKEN_AUTO;     }

\(         { return TOKEN_LEFTPARAND;  }
\)         { return TOKEN_RIGHTPARAND; }
\[         { return TOKEN_LEFTSQUARE;  }
\]         { return TOKEN_RIGHTSQUARE; }
\{         { return TOKEN_LEFTCURLY;   }
\}         { return TOKEN_RIGHTCURLY;  }

\+\+       { return TOKEN_INCREMENT;   }
\+         { return TOKEN_ADD;         }
\-\-       { return TOKEN_DECREMENT;   } 
\-         { return TOKEN_SUBTRACT;    }
\!         { return TOKEN_NOT;         }
\^         { return TOKEN_EXPONENT;    }
\*         { return TOKEN_MULTIPLY;    }
\/         { return TOKEN_DIVIDE;      }
\%         { return TOKEN_MODULUS;     }
\>\=       { return TOKEN_GE;          }
\>         { return TOKEN_GT;          }
\<\=       { return TOKEN_LE;          }
\<         { return TOKEN_LT;          }
\&\&       { return TOKEN_AND;         }
\|\|       { return TOKEN_OR;          }
\=\=       { return TOKEN_EQ;          }
\!\=       { return TOKEN_NEQ;         }
\=         { return TOKEN_ASSIGNMENT;  }
\;         { return TOKEN_SEMICOLON;   }
\:         { return TOKEN_COLON;       }
\,         { return TOKEN_COMMA;       }


\"([^\\"]|\\.)*\"                   { 
							            // Reset function variables
							            memset(buffer, 0, 256);
							            buf_idx = 0;
							            null_cnt = 0;

							            // Get rid of the quotations
							            for (int i=0;i<strlen(yytext);i++)
							            {
							                // Handle invalid escape characters
							                if (yytext[i] == '\n')
							                {
							                    // Print error message and exit for invalid escape character
							                    fprintf(stderr, "scan error: invalid character in string: \\n\n");
							                    exit(1);
							                }
							                // Handle valid escape characters
							                if ((yytext[i+1] == '\\') && (i+2 < strlen(yytext)))
							                {
							                    // Go through and replace each valid escape character with a corresponding actual escape character
							                    switch (yytext[i+2])
							                    {
							                        case 'n': { buffer[buf_idx] = '\n';             ++i; break; } // newline escape character
							                        case '0': { buffer[buf_idx] = '\0'; ++null_cnt; ++i; break; } // null escape character, increase null count
							                        default:  { buffer[buf_idx] = yytext[i+2];      ++i; break; } // any other character that follows a '\\'
							                    }
							                }
							                else
							                {
							                    // otherwise add normal character to char/string literal
							                    buffer[buf_idx] = yytext[i+1];
							                }
							                // increase buffer index to next empty character
							                buf_idx++;
							            }
							            // Assign null character to the end
							            buffer[buf_idx-2] = '\0'; ++null_cnt;
							            // Check string character limit
							            if (strlen(buffer)+null_cnt > 256)
							            {
							                // Print error message and exit for a string length that's too long
							                fprintf(stderr, "scan error: string length cannot be longer than 256 characters (includes null terminator). String length = %li\n", strlen(buffer)+null_cnt);
							                exit(1);
							            }

							            return TOKEN_STRING_LITERAL; 
                                    }

\'([^\\']|\\.)\'                    { 
							            // Reset function variables
							            memset(buffer, 0, 256);
							            buf_idx = 0;
							            null_cnt = 0;

							            // Get rid of the quotations
							            for (int i=0;i<strlen(yytext);i++)
							            {
							                // Handle invalid escape characters
							                if (yytext[i] == '\n')
							                {
							                    // Print error message and exit for invalid escape character
							                    fprintf(stderr, "scan error: invalid character in string: \\n\n");
							                    exit(1);
							                }
							                // Handle valid escape characters
							                if ((yytext[i+1] == '\\') && (i+2 < strlen(yytext)))
							                {
							                    // Go through and replace each valid escape character with a corresponding actual escape character
							                    switch (yytext[i+2])
							                    {
							                        case 'n': { buffer[buf_idx] = '\n';             ++i; break; } // newline escape character
							                        case '0': { buffer[buf_idx] = '\0'; ++null_cnt; ++i; break; } // null escape character, increase null count
							                        default:  { buffer[buf_idx] = yytext[i+2];      ++i; break; } // any other character that follows a '\\'
							                    }
							                }
							                else
							                {
							                    // otherwise add normal character to char/string literal
							                    buffer[buf_idx] = yytext[i+1];
							                }
							                // increase buffer index to next empty character
							                buf_idx++;
							            }

										return TOKEN_CHAR_LITERAL; 
									}

({LETTER}|_)(({LETTER}|{DIGIT}|_)*) { if (strlen(yytext) > 256) {printf("%li ",strlen(yytext)); fprintf(stderr,"scan error: identifier cannot be longer than 256 characters.\n"); exit(1);}
                                      return TOKEN_IDENT;
                                    }

{DIGIT}+                            { return TOKEN_NUMBER; }

.                                   { fprintf(stderr, "scan error: %s is an invalid token\n", yytext); exit(1); }

%%
int yywrap() { return 1; }
