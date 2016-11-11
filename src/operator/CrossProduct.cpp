#include "operator/CrossProduct.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
CrossProduct::CrossProduct(unique_ptr<Operator>&& left,unique_ptr<Operator>&& right)
   : left(move(left)),right(move(right)),readLeft(true)
   // Constructor
{
}
//---------------------------------------------------------------------------
CrossProduct::~CrossProduct()
   // Destructor
{
}
//---------------------------------------------------------------------------
void CrossProduct::open()
   // Open the operator
{
   left->open();
   readLeft=true;
}
//---------------------------------------------------------------------------
bool CrossProduct::next()
   // Get the next tuple
{
   while (true) {
      // Read the left side?
      if (readLeft) {
         if (!left->next())
            return false;
         readLeft=false;
         right->open();
      }

      // Read the right side
      if (!right->next()) {
         right->close();
         readLeft=true;
         continue;
      }

      // Got a pair
      return true;
   }
}
//---------------------------------------------------------------------------
void CrossProduct::close()
   // Close the operator
{
   if (!readLeft) {
      right->close();
      readLeft=true;
   }
   left->close();
}
//---------------------------------------------------------------------------
vector<const Register*> CrossProduct::getOutput() const
   // Get all produced values
{
   vector<const Register*> result=left->getOutput(),other=right->getOutput();
   for (auto iter=other.begin(),limit=other.end();iter!=limit;++iter)
      result.push_back(*iter);
   return result;
}
//---------------------------------------------------------------------------
