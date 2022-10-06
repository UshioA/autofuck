#include <exception>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;
const uint32_t STACK_SIZE = (2 << 10);
const int8_t LEX_END = -1;

uint32_t pstack[STACK_SIZE];
uint32_t flag_stack[114514];
uint32_t stackptr;
uint32_t flag_ptr;

uint32_t preg;

class Token {
public:
  enum Tk_type {
    AUTO,
    DOUBLECOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LT,
    GT,
    LARROW,
    LWORRA,
    END,
  };

  Token(Tk_type t = END, string v = "") : type(t), value(v) {}
  friend ostream &operator<<(ostream &os, const Token &t) {
    return os << t.type;
  }
  Tk_type type;

private:
  string value;
};

class Lexer {
public:
  Lexer(string text) {
    this->text = text;
    current_char = text[0];
    pos = 0;
  }

  void advance() {
    if (pos + 1 < text.length())
      current_char = text[++pos];
    else
      current_char = LEX_END;
  }

  char peek(int length = 1) {
    if (pos + length < text.length())
      return text[pos + length];
    return LEX_END;
  }

  Token Auto() {
    string res;
    if (current_char == 'a')
      res.push_back(current_char);
    else
      error();
    advance();
    if (current_char == 'u')
      res.push_back(current_char);
    else
      error();
    advance();
    if (current_char == 't')
      res.push_back(current_char);
    else
      error();
    advance();
    if (current_char == 'o')
      res.push_back(current_char);
    else
      error();
    advance();
    return Token(Token::AUTO, res);
  }

  Token dcolon() {
    string res;
    res.push_back(current_char);
    advance();
    if (current_char == ':')
      res.push_back(current_char);
    else
      error();
    advance();
    return Token(Token::DOUBLECOLON, res);
  }
  bool end() { return current_char == LEX_END; }
  void error() { abort(); };
  Token next_token() {
    while (current_char != LEX_END) {
      if (current_char == 'a') {
        return Auto();
      } else if (current_char == ':') {
        return dcolon();
      } else if (current_char == '-') {
        if (peek() == '>') {
          advance();
          advance();
          return Token(Token::LARROW);
        } else if (peek() == '<') {
          advance();
          advance();
          return Token(Token::LWORRA);
        }
        error();
      } else {
        if (sig.count(current_char)) {
          char c = current_char;
          advance();
          return sig[c];
        }
        error();
      }
    }
    return Token(Token::END);
  }

private:
  int pos;
  char current_char;
  map<char, Token> sig = {
      {'>', Token(Token::GT)},     {'<', Token(Token::LT)},
      {'{', Token(Token::LBRACE)}, {'}', Token(Token::RBRACE)},
      {'(', Token(Token::LPAREN)}, {')', Token(Token::RPAREN)},
  };
  string text;
};

struct instr {
  enum instr_type {
    PUSH,
    POP,
    ADD,
    MUL,
    DIV,
    MINUS,
    LOAD,
    BUILDFLAG,
    GOTO,
  };

  instr_type type;
  union {
    uint32_t load_reg;
    uint32_t goto_;
  };
};

instr instr_stream[114514];
uint32_t id;

uint32_t do_push() {
  pstack[stackptr++] = preg;
  cout << "Push " << preg << endl;
  return 1;
}
uint32_t do_pop() {
  preg = pstack[--stackptr];
  cout << "Pop " << preg << endl;
  return 1;
}
int do_binop(int type) {
  do_pop();
  switch (instr::instr_type(type)) {
  case instr::ADD: {
    pstack[stackptr - 1] += preg;
    cout << "Plus" << endl;
  } break;
  case instr::MUL: {
    pstack[stackptr - 1] *= preg;
    cout << "Mul" << endl;
  } break;
  case instr::DIV: {
    pstack[stackptr - 1] /= preg;
    cout << "Div" << endl;
  } break;
  case instr::MINUS: {
    pstack[stackptr - 1] -= preg;
    cout << "Minus" << endl;
  } break;
  default:
    abort();
  }
  return 1;
}
int do_load(uint32_t num) {
  preg = num;
  cout << "load " << num << endl;
  return 1;
}
int do_setflag() {
  flag_stack[flag_ptr++] = id;
  cout << "setflag" << endl;
  return 1;
}
int do_goto() {
  if (preg) {
    id = flag_stack[--flag_ptr];
    cout << "goto " << id << endl;
    return 0;
  }
  return 1;
}
int i = 0;

uint32_t count_auto(const vector<Token> &stream) {
  uint32_t c = 0;
  if (stream[i].type == Token::AUTO) {
    ++i;
    ++c;
    while (stream[i].type == Token::DOUBLECOLON &&
           stream[i + 1].type == Token::AUTO) {
      i += 2;
      ++c;

      if (c == 10)
        abort();
    }
  }
  return c;
}

uint32_t count_number_(const vector<Token> &stream, uint32_t c = 0) {
  uint32_t d = count_auto(stream);
  if (stream[i].type == Token::LPAREN) {
    i++;
    return count_number_(stream, c * 10 + d);
  } else if (stream[i].type == Token::RPAREN) {
    i++;
    return c * 10 + d;
  } else
    return c * 10 + d;
}

uint32_t count_number(const vector<Token> &stream) {
  uint32_t c = 0;
  if (stream[i].type == Token::LT) {
    ++i;
  } else
    abort();
  if (stream[i].type != Token::GT) {
    c = count_number_(stream);
  }
  ++i;
  return c;
}

void parse(vector<Token> stream) {
  while (i < stream.size()) {
    if (stream[i].type == Token::AUTO) {
      if (stream[i + 1].type == Token::DOUBLECOLON) {
        i += 2;
        instr_stream[id++] = instr{instr::PUSH};
      } else if (stream[i + 1].type == Token::LARROW) {
        i += 2;
        instr_stream[id++] = instr{instr::ADD};
      } else if (stream[i + 1].type == Token::LWORRA) {
        i += 2;
        instr_stream[id++] = instr{instr::MINUS};
      } else if (stream[i + 1].type == Token::LT) {
        ++i;
        instr_stream[id++] = instr{instr::LOAD, count_number(stream)};
      } else
        abort();
    } else if (stream[i].type == Token::DOUBLECOLON) {
      if (stream[i + 1].type == Token::AUTO) {
        i += 2;
        instr_stream[id++] = instr{instr::POP};
      } else {
        puts("nmsl");
        abort();
      }
    } else if (stream[i].type == Token::LBRACE) {
      i++;
      instr_stream[id] = instr{instr::BUILDFLAG, id};
      ++id;
    } else if (stream[i].type == Token::RBRACE) {
      i++;
      instr_stream[id++] = instr{instr::GOTO};
    } else
      abort();
  }
}

void one_step() {
  switch (instr_stream[id].type) {
  case instr::PUSH:
    id += do_push();
    break;
  case instr::POP:
    id += do_pop();
    break;
  case instr::LOAD:
    id += do_load(instr_stream[id].load_reg);
    break;
  case instr::BUILDFLAG:
    id += do_setflag();
    break;
  case instr::GOTO:
    id += do_goto();
    break;
  case instr::ADD:
  case instr::DIV:
  case instr::MUL:
  case instr::MINUS:
    id += do_binop(instr_stream[id].type);
    break;
  }
}

int main() {
  string a;
  while (cin >> a) {
    id = 0;
    i = 0;
    Lexer l(a);
    vector<Token> str;
    while (!l.end()) {
      auto t = l.next_token();
      str.push_back(t);
    }
    parse(str);
    uint32_t max_id = id;
    id = 0;
    one_step();
    while (id < max_id) {
      one_step();
    }
  }
}