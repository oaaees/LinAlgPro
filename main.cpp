#include "Nums.h"
using namespace std;

int main (){
    cout << "Tipos de datos básicos: \n";

    shared_ptr<Exp> A = parse("<escalar, 3.14159>");
    shared_ptr<Exp> B = parse("<complejo, 3.14-0.14i>");
    shared_ptr<Exp> C = parse("<vector, [1,2,3,4,5,6]>");
    shared_ptr<Exp> D = parse("<matriz, [[1,2],[4,5],[7,8]]>");

    cout << A->to_string() << endl << endl;
    cout << B->to_string() << endl << endl;
    cout << C->to_string() << endl << endl;
    cout << D->to_string() << endl << endl;

    cout << "Operaciones: \n";

    shared_ptr<Exp> E = parse("<suma,<complejo, 3.14-0.14i>,<complejo, 3.14-0.14i>>");
    shared_ptr<Exp> F = parse("<multesc,<matriz, [[2,2], [3,3]]>,<escalar, 4>>");

    Environment env;
    cout << E->to_string() << endl;
    cout << E->eval(env)->to_string() << endl << endl;
    cout << F->to_string() << endl;
    cout << F->eval(env)->to_string() << endl << endl;

    return 0;
}
