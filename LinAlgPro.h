#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

bool is_str_num(string s){
    for(int i = 0; i < s.size(); i++){
        if ((s[i] < '0' || s[i] > '9') && (s[i] != '+' && s[i] != '-' && s[i] != '.')) return false;
    }

    return true;
}

string remove_whites(string s){
    string res = "";

    for(int i = 0; i < s.size(); i++){
        if(s[i] != ' ' && s[i] != '\n') res += s[i];
    }

    return res;
}

struct Environment;

struct Exp
{
  enum class ExpType
  {
    ESCALAR, COMPLEJO, VECTOR, MATRIZ, SUMA, MULTESC
  };
  
  ExpType type;

  Exp(ExpType t)
    : type(t)
  {
    // empty
  }

  ExpType get_type() const
  {
    return type;
  }
  
  virtual ~Exp()
  {
    // empty
  }

  virtual shared_ptr<Exp> clone() = 0;

  virtual shared_ptr<Exp> eval(Environment &) = 0;

  virtual string to_string() const = 0;
};

using VarList = vector<tuple<string, shared_ptr<Exp>>>;

struct Environment : public VarList
{
  Environment()
    : VarList()
  {
    // Empty
  }
  
  Environment(const Environment & env)
  {
    for (auto t : env)
      this->push_back(make_tuple(std::get<0>(t), std::get<1>(t)->clone()));
  }
  
  ~Environment()
  {
    this->erase(this->begin(), this->end());
  }
};

shared_ptr<Exp> envlookup(Environment & env, const string & var_name)
{
    tuple<string, shared_ptr<Exp>> * a = nullptr;

    for(auto t : env)
    {
        if( get<0>(t) == var_name){
            a = &t;
            break;
        }
    }
  
  if (a == nullptr)
    return nullptr;

  return get<1>(*a)->clone();
};

struct Escalar : public Exp
{
  double value;

  Escalar()
    : Exp(ExpType::ESCALAR), value(0)
  {
    // empty
  }
  
  Escalar(double v)
    : Exp(ExpType::ESCALAR), value(v)
  {
    // empty
  }

  ~Escalar(){}
  
  shared_ptr<Exp> clone() override
  {
    return make_shared<Escalar>(value);
  }
  
  shared_ptr<Exp> eval(Environment &) override
  {
    return clone();
  }

  string to_string() const override
  {
    stringstream s;
    s << "<escalar, " << value << ">";
    return s.str();
  }
};

struct Complejo : public Exp
{
  tuple<double, double> value;

  Complejo()
    : Exp(ExpType::COMPLEJO), value(make_tuple(0.0, 0.0))
  {
    // empty
  }
  
  Complejo(tuple<double, double> v)
    : Exp(ExpType::COMPLEJO), value(v)
  {
    // empty
  }

  ~Complejo(){}


  shared_ptr<Exp> clone() override
  {
    return make_shared<Complejo>(value);
  }
  
  shared_ptr<Exp> eval(Environment &) override
  {
    return clone();
  }

  string to_string() const override
  {
    stringstream s;
    s << "<complejo, " << get<0>(value);
    if (get<1>(value) > 0) s << "+";
    s << get<1>(value) << "i>";
    return s.str();
  }
};

struct Vector : public Exp
{
  vector<double> value;

  Vector()
    : Exp(ExpType::VECTOR), value({0.0})
  {
    // empty
  }
  
  Vector(vector<double> v)
    : Exp(ExpType::VECTOR), value(v)
  {
    // empty
  }

  ~Vector(){}

  shared_ptr<Exp> clone() override
  {
    return make_shared<Vector>(value);
  }
  
  shared_ptr<Exp> eval(Environment &) override
  {
    return clone();
  }

  string to_string() const override
  {
    stringstream s;
    s << "<Vector, [";

    for(int i = 0; i < value.size(); i++){
      s << value[i];
      if (i != value.size() - 1) s << ",";
    }

    s << "]>"; 

    return s.str();
  }
};

struct Matriz : public Exp
{
  vector<vector<double>> value;

  Matriz()
    : Exp(ExpType::MATRIZ), value({{0.0}})
  {
    // empty
  }
  
  Matriz(vector<vector<double>> v)
    : Exp(ExpType::MATRIZ), value(v)
  {
    // empty
  }

  ~Matriz(){}

  shared_ptr<Exp> clone() override
  {
    return make_shared<Matriz>(value);
  }
  
  shared_ptr<Exp> eval(Environment &) override
  {
    return clone();
  }

  string to_string() const override
  {
    stringstream s;
    s << "<Matriz,";

    for(int i = 0; i < value.size(); i++){
        s << "\n";
        for(int j = 0; j < value[i].size(); j++){
            s << value[i][j] << " " ;
        }
    }

    s << ">"; 

    return s.str();
  }
};

struct Suma : public Exp
{
  shared_ptr<Exp> e1;
  shared_ptr<Exp> e2;

  Suma()
    : Exp(ExpType::SUMA), e1(nullptr), e2(nullptr)
  {
    // empty
  }

  Suma(shared_ptr<Exp> _e1, shared_ptr<Exp> _e2)
    : Exp(ExpType::SUMA), e1(_e1), e2(_e2)
  {
    // empty
  }

  ~Suma(){}

  shared_ptr<Exp> clone() override
  {
    return make_shared<Suma>(e1->clone(), e2->clone());
  }
  
  shared_ptr<Exp> eval(Environment & env) override
  {
    shared_ptr<Exp> ee1 = e1->eval(env);
    shared_ptr<Exp> ee2 = e2->eval(env);

    if(ee1->get_type() == ee2->get_type()){
      if(ee1->get_type() == ExpType::ESCALAR){
        // Si ambos son escalares
        double val = dynamic_pointer_cast<Escalar>(ee1)->value + dynamic_pointer_cast<Escalar>(ee2)->value;
        return make_shared<Escalar>(val);
      } else if (ee1->get_type() == ExpType::COMPLEJO){
        //Si ambos son complejos
        double real = get<0>(dynamic_pointer_cast<Complejo>(ee1)->value) + get<0>(dynamic_pointer_cast<Complejo>(ee2)->value);
        double imaginary = get<1>(dynamic_pointer_cast<Complejo>(ee1)->value) + get<1>(dynamic_pointer_cast<Complejo>(ee2)->value);
        return make_shared<Complejo>(make_tuple(real, imaginary));
      } else if (ee1->get_type() == ExpType::MATRIZ){
        //Si ambos son matrices
        vector<vector<double>> A = dynamic_pointer_cast<Matriz>(ee1)->value;
        vector<vector<double>> B = dynamic_pointer_cast<Matriz>(ee2)->value;
        vector<vector<double>> C = {};

        if(A.size() != B.size()) return nullptr;

        for(int i = 0; i < A.size(); i++){
          if(A[i].size() != B[i].size()) return nullptr;
          vector<double> row = {};
          for(int j = 0; j < A[i].size(); j++){
            row.push_back(A[i][j] + B[i][j]);
          }
          C.push_back(row);
        }

        return make_shared<Matriz>(C);
      }
    }
    
    return nullptr;
  }

  string to_string() const override
  {
    stringstream s;
    s << "<Suma, " << e1->to_string() << ", " << e2->to_string() << ">";
    return s.str();
  }
};

struct MultEsc : public Exp
{
  shared_ptr<Exp> e1;
  shared_ptr<Exp> e2;

  MultEsc()
    : Exp(ExpType::MULTESC), e1(nullptr), e2(nullptr)
  {
    // empty
  }

  MultEsc(shared_ptr<Exp> _e1, shared_ptr<Exp> _e2)
    : Exp(ExpType::MULTESC), e1(_e1), e2(_e2)
  {
    // empty
  }

  ~MultEsc(){}

  shared_ptr<Exp> clone() override
  {
    return make_shared<MultEsc>(e1->clone(), e2->clone());
  }
  
  shared_ptr<Exp> eval(Environment & env) override
  {
    shared_ptr<Exp> ee1 = e1->eval(env);
    shared_ptr<Exp> ee2 = e2->eval(env);

    if(ee2->get_type() == ExpType::ESCALAR){
      if(ee1->get_type() == ExpType::ESCALAR){
        // Si ambos son escalares
        double val = dynamic_pointer_cast<Escalar>(ee1)->value * dynamic_pointer_cast<Escalar>(ee2)->value;
        return make_shared<Escalar>(val);
      } else if (ee1->get_type() == ExpType::COMPLEJO){
        //Si e1 es complejo
        double real = get<0>(dynamic_pointer_cast<Complejo>(ee1)->value) * dynamic_pointer_cast<Escalar>(ee2)->value;
        double imaginary = get<1>(dynamic_pointer_cast<Complejo>(ee1)->value) * dynamic_pointer_cast<Escalar>(ee2)->value;
        return make_shared<Complejo>(make_tuple(real, imaginary));
      } else if (ee1->get_type() == ExpType::MATRIZ){
        //Si e2 es una matriz
        vector<vector<double>> A = dynamic_pointer_cast<Matriz>(ee1)->value;
        vector<vector<double>> C = {};

        for(int i = 0; i < A.size(); i++){
          vector<double> row = {};
          for(int j = 0; j < A[i].size(); j++){
            row.push_back(A[i][j] * dynamic_pointer_cast<Escalar>(ee2)->value);
          }
          C.push_back(row);
        }

        return make_shared<Matriz>(C);
      }
    }
    
    return nullptr;
  }

  string to_string() const override
  {
    stringstream s;
    s << "<MultEsc, " << e1->to_string() << ", " << e2->to_string() << ">";
    return s.str();
  }
};

shared_ptr<Exp> parse(const string & p, int & pos){
    if (pos >= p.size())
        return nullptr;

    size_t end_name = p.find_first_of(',', pos);
    string name = p.substr(pos + 1, end_name - pos - 1);
    pos = end_name + 1;

    if (name == "escalar") // Es una expresión entera
    {
        shared_ptr<Escalar> ret = make_shared<Escalar>();
        
        // extraigo el valor entero
        size_t end_val = p.find_first_of('>', end_name);

        if (p[end_val] != '>'){
          return nullptr;
        }

        string val = p.substr(end_name + 1, end_val - end_name - 1);

        if (not is_str_num(val)){
            return nullptr;
        }

        ret->value = stod(val);

        pos = end_val + 1;

        return ret;
    } else if (name == "complejo") {
        shared_ptr<Complejo> ret = make_shared<Complejo>();
        
        // extraigo el valor entero
        size_t end_val = p.find_first_of('>', end_name);

        if (p[end_val] != '>'){
            return nullptr;
        }

        string val = p.substr(end_name + 1, end_val - end_name - 1);

        size_t end_real = 0;

        if (val.find_first_of('+') != val.npos){
            end_real = val.find_first_of('+');
        } else if (val.find_first_of('-') != val.npos){
            end_real = val.find_first_of('-');
        }

        string val_real = val.substr(0, end_real);

        if (not is_str_num(val_real)){
            return nullptr;
        }

        string val_imaginary = val.substr(end_real, val.size() - end_real - 1);

        if (not is_str_num(val_imaginary) || val.back() != 'i'){
            return nullptr;
        }

        double real = stod(val_real);
        double imaginary = stod(val_imaginary);
        tuple<double, double> value = make_tuple(real, imaginary);

        ret->value = value;

        pos = end_val + 1;

        return ret;        
    } else if (name == "vector") {
        shared_ptr<Vector> ret = make_shared<Vector>();
        size_t end_val = p.find_first_of('>', end_name);

        if (p[end_val] != '>'){
            return nullptr;
        }

        string val_string = p.substr(end_name + 1, end_val - end_name - 1);
        vector<double> value; 

        if (val_string.front() != '['){
            return nullptr;
        } else {
          val_string = val_string.substr(1);
          
          while (!val_string.empty()){
            size_t end_single_value = val_string.find_first_of(',');
            if (end_single_value == val_string.npos) end_single_value = val_string.size() - 1;
            string single_value_string = val_string.substr(0, end_single_value);
            value.push_back(stod(single_value_string));
            val_string = val_string.substr(end_single_value + 1);
          }
        }

        ret->value = value;
        pos = end_val + 1;

        return ret; 


    } else if (name == "matriz") {
        shared_ptr<Matriz> ret = make_shared<Matriz>();
        size_t end_val = p.find_first_of('>', end_name);

        if (p[end_val] != '>'){
            return nullptr;
        }

        string val_string = p.substr(end_name + 1, end_val - end_name - 1);
        vector<vector<double>> matrix_value; 

        if (val_string.front() != '['){
            return nullptr;
        } else {
            val_string = val_string.substr(1);
            while(!val_string.empty()){
                size_t end_row = val_string.find_first_of(']');
                string row_string = val_string.substr(0, end_row + 1);
                vector<double> row_vector;

                if(row_string.front() != '['){
                    return nullptr;
                } else {
                    row_string = row_string.substr(1);
                    while(!row_string.empty()){
                        size_t end_single_value = row_string.find_first_of(',');
                        if (end_single_value == row_string.npos) end_single_value = row_string.size() - 1;
                        string single_value_string = row_string.substr(0, end_single_value);
                        double single_value = stod(single_value_string);

                        row_vector.push_back(single_value);
                        row_string = row_string.substr(end_single_value + 1);
                    }
                }

                matrix_value.push_back(row_vector);   
                val_string = val_string.substr(end_row + 2);
            }
        }

        ret->value = matrix_value;
        pos = end_val + 1;

        return ret; 

    } else if (name == "suma") {
      shared_ptr<Suma> ret = make_shared<Suma>();

	    ret->e1 = parse(p, pos);

	    if (ret->e1 == nullptr || p[pos] != ','){
	      return nullptr;
	    }

	    ++pos;
      
	    ret->e2 = parse(p, pos);

	    if (ret->e2 == nullptr || p[pos] != '>'){
	      return nullptr;
	    }
      
	    ++pos;

	    return ret;
    } else if (name == "multesc") {
      shared_ptr<MultEsc> ret = make_shared<MultEsc>();

	    ret->e1 = parse(p, pos);

	    if (ret->e1 == nullptr || p[pos] != ','){
	      return nullptr;
	    }

	    ++pos;
      
	    ret->e2 = parse(p, pos);

	    if (ret->e2 == nullptr || p[pos] != '>'){
	      return nullptr;
	    }
      
	    ++pos;

	    return ret;
    } else {
        return nullptr;
    }
}

shared_ptr<Exp> parse(const string & prog)
{
  if (prog.empty())
    return nullptr;
  
  int p = 0;
  shared_ptr<Exp> ret = parse(remove_whites(prog), p);

  if (ret == nullptr)
    throw logic_error("bad formed expression");

  return ret;
}