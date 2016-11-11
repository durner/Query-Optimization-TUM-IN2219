#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
int main()
{
   Database db;
   db.open("data/uni");
   Table& studenten=db.getTable("studenten");

   Tablescan scan(studenten);
   const Register* name=scan.getOutput("name");

   scan.open();
   while (scan.next())
      cout << name->getString() << endl;
   scan.close();
}
//---------------------------------------------------------------------------
