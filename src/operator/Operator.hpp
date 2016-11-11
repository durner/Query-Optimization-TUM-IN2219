#ifndef H_operator_Operator
#define H_operator_Operator
//---------------------------------------------------------------------------
#include <vector>
//---------------------------------------------------------------------------
class Register;
//---------------------------------------------------------------------------
/// Operator interface
class Operator
{
   public:
   /// Constructor
   Operator();
   /// Destructor
   virtual ~Operator();

   /// Open the operator
   virtual void open() = 0;
   /// Produce the next tuple
   virtual bool next() = 0;
   /// Close the operator
   virtual void close() = 0;

   /// Get all produced values
   virtual std::vector<const Register*> getOutput() const = 0;
};
//---------------------------------------------------------------------------
#endif
