#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include "operator/Selection.hpp"
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
   const Register* name=scan->getOutput("name");
   const Register* semester=scan->getOutput("semester");
   Register two; two.setInt(2);
   Selection select(move(scan),semester,&two);

   select.open();
   while (select.next())
      cout << name->getString() << endl;
   select.close();
}
//---------------------------------------------------------------------------
