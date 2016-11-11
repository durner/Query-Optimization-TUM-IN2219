#include "operator/Tablescan.hpp"
#include "Table.hpp"
#include <cstdlib>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Tablescan::Tablescan(Table& table)
   : table(table),bufferStart(0),bufferStop(0),filePos(0)
   // Constructor
{
   output.resize(table.getAttributeCount());
}
//---------------------------------------------------------------------------
Tablescan::~Tablescan()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Tablescan::open()
   // Open the operator
{
   bufferStart=bufferStop=0; filePos=0;
}
//---------------------------------------------------------------------------
bool Tablescan::next()
   // Get the next tuple
{
   bool escape=false;
   for (unsigned index=0,limit=output.size();index<limit;++index) {
      buf.resize(0);
      while (true) {
         if (bufferStart>=bufferStop) {
            table.io.seekg(filePos,ios_base::beg);
            int len=table.io.readsome(buffer,bufferSize);
            if (len<1) { table.io.clear(); return false; }
            bufferStart=0;
            bufferStop=len;
            filePos+=len;
         }
         char c=buffer[bufferStart++];
         if (escape) { escape=false; buf+=c; continue; }
         if (c=='\r') continue;
         if ((c==';')||(c=='\n')) {
            Register& r=output[index];
            switch (table.attributes[index].getType()) {
               case Attribute::Type::Int: r.setInt(atoi(buf.c_str())); break;
               case Attribute::Type::Double: r.setDouble(atof(buf.c_str())); break;
               case Attribute::Type::Bool: r.setBool(buf=="true"); break;
               case Attribute::Type::String: r.setString(buf); break;
            }
            break;
         } else if (c=='\\') {
            escape=true;
         } else buf+=c;
      }
   }

   return true;
}
//---------------------------------------------------------------------------
void Tablescan::close()
   // Close the operator
{
}
//---------------------------------------------------------------------------
vector<const Register*> Tablescan::getOutput() const
   // Get all produced values
{
   vector<const Register*> result;
   for (auto iter=output.begin(),limit=output.end();iter!=limit;++iter)
      result.push_back(&(*iter));
   return result;
}
//---------------------------------------------------------------------------
const Register* Tablescan::getOutput(const std::string& name) const
   // Get one produced value
{
   int slot=table.findAttribute(name);
   if (slot<0) return 0;
   return &output[slot];
}
//---------------------------------------------------------------------------
