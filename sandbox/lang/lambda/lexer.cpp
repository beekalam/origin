
#include "lexer.hpp"

Lexer::Lexer(symbol_table* t, const String& buf)
  : table{t}
  , first{buf.data()}
  , last{buf.data() + buf.size()}
  , tok{}
  , loc{1, 1}
{ }

// Lex the next token out of the buffer, returning it.
Token 
Lexer::operator()()
{
  // Pre-lex the token, skipping any initial whitespace and/or comments,
  // advancing to the first non-ws character. Note that we may reach EoF in
  // the process.
  //
  // TODO: There may be some cases where emitting WS tokens are useful.
  // but that's future work. 
  consume_ws();

  // If we reached the end of the line, return an EoF token.
  if (is_eof()) {
    make_eof();
  } else {
    // Otherwise, match against the language's symbols.
    //
    // Keep a pointer to the start of the symbol so we can reconstruct the
    // string later.
    const Char* start = first; 
    switch(*first) {
    case '(':  make_punctuation(Symbol::Lparen);    break;
    case ')':  make_punctuation(Symbol::Rparen);    break;
    case '\\': make_punctuation(Symbol::Backslash); break;
    case '.':  make_punctuation(Symbol::Dot);       break;
    case ';':  make_punctuation(Symbol::Semicolon); break;
    case '=':  make_punctuation(Symbol::Equal);     break;
    default: {
      // NOTE: This will handle keywords gracefully since keywords are hashed
      // into the same symbol table. Note that we could be much more efficient
      // about doing this by building a state-machine to recognize keywords
      // explicitly rather than relying on the symbol table. It's just more
      // work.
      if (is_identifier())
        make_identifer(start, first);
      else
        make_error();
    }
    }
  }

  return tok;
}

// Lexing support

// Returns true when we reach the end of file.
bool 
Lexer::is_eof() const { return first == last; }

// Returns true if the currenct character compares equal to c.
bool 
Lexer::is_char(char c) const { return !is_eof() && (*first == c); }

// Returns true if the current character is a letter. 
bool 
Lexer::is_letter() const { return !is_eof() && isalpha(*first); }

// Returns true if the current character is a digit.
bool 
Lexer::is_digit() const { return !is_eof() && isdigit(*first); }

// Returns true if c is either a letter or digit. If it is, assign that
// character to c and advance.
bool 
Lexer::is_alphanumeric() const { return !is_eof() && isalnum(*first); }

// An identifier (variable) in the untyped lambda calculus is a string
// of alphanumeric characters that cannot start with a digit. An underscore
// may be used in any position.
bool 
Lexer::is_identifier()
{
  if (is_letter() || is_char('_')) {
    ++first;
    while (is_alphanumeric() || is_char('_'))
      ++first;
    return true;
  }
  return false;
}


// Comments
//
// FIXME: Implement comment parsing! Are there comments in untyped lambda
// calculus? Not really sure. I could make "--" to the EoL a comment.


// Whitespace

// Consume whitespace and return true if any whitespace was consumed.
bool 
Lexer::consume_ws()
{
  bool result;
  while (!is_eof() && (consume_horizontal_ws() || consume_vertical_ws()))
    result = true;
  return result;
}

// Recognize and consume horizontal whitespace (' ' or tab).
bool 
Lexer::consume_horizontal_ws() 
{ 
  switch (*first) {
  case ' ':
  case '\t':
    ++loc.column;
    ++first;
    return true;
  }
  return false;
}

// Recognize and consume vertical whitespace, incrementing the line
// count when we find some. If the line number is incremented, then reset
// the current column count to 1.
//
// FIXME: What about DOS or Windows EoF? How should I expect to handle a
// carriage return? Vertical newlines?
//
// FIXME: Sometimes I need to know about the end of line!
bool 
Lexer::consume_vertical_ws()
{
  switch(*first) {
  case '\r':
  case '\n':
  case '\v':
    ++loc.line;
    loc.column = 1;
    ++first;
    return true;
  }
  return false;
}


// Token construction
// These functions fix information for the current token prior to
// returning it from the lex function. In particular, they bind the token
// the current source location and register the lexed content with the
// current symbol table.

// FIXME: Fnd 

// Return the eof token.
void 
Lexer::make_eof()
{
  tok.sym = table->get(Symbol::Eof);
}

// Return a punctuation token.
//
// FIXME: This will, in the future, require that we know the length of
// the punctuator. For now, all tokens are length 1, so its easy.
void 
Lexer::make_punctuation(Symbol::Kind kind)
{
  tok.loc = loc;
  tok.sym = table->get(kind);
  ++first;
  ++loc.column;
}

// Enter the given substring as an identifier in the table. If the identifier
// already exists, return the current string.
void 
Lexer::make_identifer(const Char* first, const Char* last)
{
  tok.loc = loc;
  tok.sym = table->put(Symbol::Identifier, first, last);
  first += tok.spelling().size();
  loc.column += tok.spelling().size();
}

void 
Lexer::make_error()
{
  tok.loc = loc;
  tok.sym = table->get(first, first + 1);
  ++first;
  ++loc.column;
}
