#include "operator/Printer.hpp"
#include "Register.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Printer::Printer(unique_ptr<Operator>&& input)
   : input(move(input)),toPrint(this->input->getOutput())
   // Constructor
{
}
//---------------------------------------------------------------------------
Printer::Printer(unique_ptr<Operator>&& input,const vector<const Register*>& toPrint)
   : input(move(input)),toPrint(toPrint)
   // Constructor
{
}
//---------------------------------------------------------------------------
void Printer::open()
   // Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Printer::next()
   // Get the next tuple
{
   // Produce the next tuple
   if (!input->next())
      return false;

   // Print the entries
   for (unsigned index=0,limit=toPrint.size();index<limit;++index) {
      if (index) cout << ' ';
      const Register& r=*toPrint[index];
      switch (r.getState()) {
         case Register::State::Unbound: cout << "null"; break;
         case Register::State::Int: cout << r.getInt(); break;
         case Register::State::Double: cout << r.getDouble(); break;
         case Register::State::Bool: cout << (r.getBool()?"true":"false"); break;
         case Register::State::String: cout << r.getString(); break;
      }
   }
   cout << endl;

   return true;
}
//---------------------------------------------------------------------------
void Printer::close()
   // Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Printer::getOutput() const
   // Get all produced values
{
   return input->getOutput();
}
//---------------------------------------------------------------------------
