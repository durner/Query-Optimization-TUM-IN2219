#ifndef H_operator_CrossProduct
#define H_operator_CrossProduct
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
/// A cross product
class CrossProduct : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> left,right;
   /// Read the left side?
   bool readLeft;

   public:
   /// Constructor
   CrossProduct(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right);
   /// Destructor
   ~CrossProduct();

   /// Open the operator
   void open();
   /// Get the next tuple
   bool next();
   /// Close the operator
   void close();

   /// Get all produced values
   std::vector<const Register*> getOutput() const;
};
//---------------------------------------------------------------------------
#endif
