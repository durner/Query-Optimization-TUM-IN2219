#ifndef H_operator_Projection
#define H_operator_Projection
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
/// A projection
class Projection : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// The output
   std::vector<const Register*> output;

   public:
   /// Constructor
   Projection(std::unique_ptr<Operator>&& input,const std::vector<const Register*>& output);
   /// Destructor
   ~Projection();

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
