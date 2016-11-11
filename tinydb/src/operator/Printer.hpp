#ifndef H_operator_Printer
#define H_operator_Printer
//---------------------------------------------------------------------------
#include "Operator.hpp"
#include <memory>
//---------------------------------------------------------------------------
/// Prints tuple attributes
class Printer : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// Registers to print
   std::vector<const Register*> toPrint;

   public:
   /// Constructor
   explicit Printer(std::unique_ptr<Operator>&& input);
   /// Constructor
   Printer(std::unique_ptr<Operator>&& input,const std::vector<const Register*>& toPrint);

   /// Open the operator
   void open();
   /// Get the next ruple
   bool next();
   /// CLose the operator
   void close();

   /// Get all produced values
   std::vector<const Register*> getOutput() const;
};
//---------------------------------------------------------------------------
#endif
