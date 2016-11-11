#include "operator/Indexscan.hpp"
#include <cstdlib>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Indexscan::Indexscan(Table& table,unsigned indexAttribute,const Register* lowerBound,const Register* upperBound)
   : table(table),index(table.indices[indexAttribute]),lowerBound(lowerBound),upperBound(upperBound)
   // Constructor
{
   output.resize(table.getAttributeCount());
}
//---------------------------------------------------------------------------
Indexscan::~Indexscan()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Indexscan::open()
   // Open the operator
{
   bufferStart=bufferStop=0;
   if (lowerBound)
      iter=index.lower_bound(*lowerBound); else
      iter=index.begin();
   iterLimit=index.end();
}
//---------------------------------------------------------------------------
bool Indexscan::next()
   // Get the next tuple
{
   // Check the iterator
   if (iter==iterLimit)
      return false;
   // Check the next entry
   unsigned filePos=(*iter).second;
   if (upperBound) {
      if (upperBound->getState()!=(*iter).first.getState()) {
         iter=iterLimit;
         return false;
      }
      switch (upperBound->getState()) {
         case Register::State::Unbound: iter=iterLimit; return false;
         case Register::State::Int: if (upperBound->getInt()<(*iter).first.getInt()) { iter=iterLimit; return false; } break;
         case Register::State::Double: if (upperBound->getDouble()<(*iter).first.getDouble()) { iter=iterLimit; return false; } break;
         case Register::State::Bool: if (upperBound->getBool()<(*iter).first.getBool()) { iter=iterLimit; return false; } break;
         case Register::State::String: if (upperBound->getString()<(*iter).first.getString()) { iter=iterLimit; return false; } break;
      }
   }

   // Read the tuple
   bufferStart=bufferStop=0;
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
void Indexscan::close()
   // Close the operator
{
}
//---------------------------------------------------------------------------
vector<const Register*> Indexscan::getOutput() const
   // Get all produced values
{
   vector<const Register*> result;
   for (auto iter=output.begin(),limit=output.end();iter!=limit;++iter)
      result.push_back(&(*iter));
   return result;
}
//---------------------------------------------------------------------------
