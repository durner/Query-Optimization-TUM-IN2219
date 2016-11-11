#include "operator/HashJoin.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
HashJoin::HashJoin(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right,const Register* leftValue,const Register* rightValue)
   : left(move(left)),right(move(right)),leftValue(leftValue),rightValue(rightValue)
   // Constructor
{
   vector<const Register*> lr(this->left->getOutput());
   for (auto iter=lr.begin(),limit=lr.end();iter!=limit;++iter)
      leftRegs.push_back(const_cast<Register*>(*iter));
}
//---------------------------------------------------------------------------
HashJoin::~HashJoin()
   // Destructor
{
}
//---------------------------------------------------------------------------
void HashJoin::open()
   // Open the operator
{
   left->open();
   right->open();
   table.clear();
}
//---------------------------------------------------------------------------
bool HashJoin::next()
   // Get the next tuple
{
   // First pass? Hash the left side
   if (table.empty()) {
      while (left->next()) {
         vector<Register> values;
         values.reserve(leftRegs.size());
         for (auto iter=leftRegs.begin(),limit=leftRegs.end();iter!=limit;++iter)
            values.push_back(**iter);
         table.insert(make_pair(*leftValue,move(values)));
      }
      if (table.empty()) return false;
      iter=iterLimit=table.end();
   }

   // Read the right hand side
   while (true) {
      // More matches?
      if (iter!=iterLimit) {
         const vector<Register>& values=(*iter).second;
         auto reader=values.begin();
         for (auto iter2=leftRegs.begin(),limit2=leftRegs.end();iter2!=limit2;++iter2,++reader)
            **iter2=*reader;
         ++iter;
         return true;
      }
      // Examine the next tuple
      if (!right->next()) {
         right->close();
         return false;
      }
      // Probe the hash table
      auto range=table.equal_range(*rightValue);
      iter=range.first; iterLimit=range.second;
   }
}
//---------------------------------------------------------------------------
void HashJoin::close()
   // Close the operator
{
   if (!table.empty()) {
      right->close();
      table.clear();
   }
   left->close();
}
//---------------------------------------------------------------------------
vector<const Register*> HashJoin::getOutput() const
   // Get all produced values
{
   vector<const Register*> result=left->getOutput(),other=right->getOutput();
   for (auto iter=other.begin(),limit=other.end();iter!=limit;++iter)
      result.push_back(*iter);
   return result;
}
//---------------------------------------------------------------------------
