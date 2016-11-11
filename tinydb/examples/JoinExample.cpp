#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"
#include "operator/Printer.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
int main()
{
   Database db;
   db.open("data/uni");
   Table& professoren=db.getTable("professoren");
   Table& vorlesungen=db.getTable("vorlesungen");

   unique_ptr<Tablescan> scanProfessoren(new Tablescan(professoren));
   unique_ptr<Tablescan> scanVorlesungen(new Tablescan(vorlesungen));

   const Register* name=scanProfessoren->getOutput("name");
   const Register* persnr=scanProfessoren->getOutput("persnr");
   const Register* titel=scanVorlesungen->getOutput("titel");
   const Register* gelesenvon=scanVorlesungen->getOutput("gelesenvon");

   unique_ptr<CrossProduct> cp(new CrossProduct(move(scanProfessoren),move(scanVorlesungen)));
   unique_ptr<Selection> select(new Selection(move(cp),persnr,gelesenvon));
   unique_ptr<Projection> project(new Projection(move(select),{name,titel}));
   Printer out(move(project));

   out.open();
   while (out.next());
   out.close();
}
//---------------------------------------------------------------------------
