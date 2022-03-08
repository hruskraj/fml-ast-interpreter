#include "json.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using json = nlohmann::json;

class AST {
public:
  virtual int eval() = 0;
  virtual std::string to_string() {
    return std::string("");
  };
  virtual ~AST() = default;
};

class AST_Top : public AST {
private:
  std::vector<AST *> nodes;
public:
  AST_Top(std::vector<AST *> nodes) : nodes(nodes) {}

  int eval() {
    for(AST * node : nodes){
      node->eval();
    }
    return 1;
  }
  
  ~AST_Top() {
    for (AST * item : nodes)
      delete item;
  }
};

class AST_Print : public AST {
private:
  std::string format;
  std::vector<AST *> arguments;
public:
  AST_Print(std::string format, std::vector<AST *> arguments) : format(format), arguments(arguments) {}
  ~AST_Print(){
    for (AST * argument : arguments)
      delete argument;
  }

  int eval() {
    int i = 0, j = 0;
    while (i < format.size()){
      if (format[i] == '\\'){
        switch (format[++i]){
          case '~':
            std::cout << "~";
            break;
          case 'n':
            std::cout << std::endl;
            break;
          case '"':
            std::cout << "\"";
            break;
          case 'r':
            std::cout << "\r";
            break;
          case 't':
            std::cout << "\t";
            break;
          case '\\':
            std::cout << "\\";
            break;
          default:
            std::cerr << "Unexpected character in formatting string\n";
        }
        ++i;
      }
      else if (format[i] == '~'){
        std::cout << arguments[j++]->to_string();
        ++i;
      }
      else{
        std::cout << format[i++];
      }
    }
    return 1;
  }
};

class AST_Null : public AST {
public:
  AST_Null() = default;
  ~AST_Null() = default;
  int eval(){
    return 0;
  }

  std::string to_string() override {
    return std::string("null");
  }
};

class AST_Integer : public AST {
private:
  int x;
public:
  AST_Integer(int x) : x(x) {}
  ~AST_Integer() = default;
  int eval(){
    return x;
  }
  std::string to_string() override {
    return std::to_string(x);
  }
};

class AST_Bool : public AST {
private:
  bool val;
public:
  AST_Bool(bool val) : val(val) {}
  ~AST_Bool() = default;
  int eval(){
    return (int) val;
  }

  std::string to_string() override {
    return val ? std::string("true") : std::string("false");
  }
};

class Interpreter{
private:
  AST_Top * m_top;
  
  AST * parse(json & command){
    if (command == "Null"){
      return new AST_Null();
    }
    for (auto & x : command.items()){
      if (x.key() == "Integer"){
        return new AST_Integer(x.value());
      }
      else if (x.key() == "Null"){
        return new AST_Null();
      }
      else if (x.key() == "Boolean"){
        return new AST_Bool(x.value());
      }
      else if (x.key() == "Print"){
        std::vector<AST *> arguments;
        for (auto & arg : x.value()["arguments"]){
          std::cout << arg << std::endl;
          arguments.push_back(parse(arg));
        }
        return new AST_Print(x.value()["format"], arguments);
      }
      else{
        std::cout << "Unknown node: " << x.key() << std::endl;
      }
    }
    return nullptr;
  }
public:
  Interpreter(json top){
    std::vector<AST *> data;
    std::cout << top["Top"] << std::endl;
    for (auto & el : top["Top"].items()){
      //for (auto & el2 : el.value().items()){
        std::cout << el.value() << std::endl;
        data.push_back(parse(el.value()));
      //}
    }
    m_top = new AST_Top(data);
  }

  ~Interpreter(){
    delete m_top;
  }

  void interpret(){
    m_top->eval();
  }
};

int main() {
  std::ifstream istream("program.json");
  json program;
  istream >> program;
  
  Interpreter interpreter(program);
  interpreter.interpret();
  
  return 0;
}
