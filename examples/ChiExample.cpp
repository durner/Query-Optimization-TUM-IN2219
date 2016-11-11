#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"
#include "operator/Printer.hpp"
#include "operator/Chi.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
int main()
{
   Database db;
   db.open("data/uni");
   Table& studenten=db.getTable("studenten");

   unique_ptr<Tablescan> scan(new Tablescan(studenten));
   const Register* semester=scan->getOutput("semester");
   const Register* name=scan->getOutput("name");

   // find all students where semester num. is not 2
   Register two; two.setInt(2);
   unique_ptr<Chi> chi(new Chi(move(scan),Chi::NotEqual,semester,&two));
   const Register* chiResult=chi->getResult();

   unique_ptr<Selection> select(new Selection(move(chi),chiResult));
   unique_ptr<Projection> project(new Projection(move(select),{name}));

   Printer out(move(project));

   out.open();
   while (out.next());
   out.close();

}
//---------------------------------------------------------------------------
