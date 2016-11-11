#ifndef H_operator_HashJoin
#define H_operator_HashJoin
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include "Register.hpp"
#include <memory>
#include <unordered_map>
//---------------------------------------------------------------------------
/// A hash join
class HashJoin : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> left,right;
   /// The registers
   const Register* leftValue,*rightValue;
   /// The copy mechanism
   std::vector<Register*> leftRegs;
   /// The hashtable
   std::unordered_multimap<Register,std::vector<Register>,Register::hash> table;
   /// Iterator
   std::unordered_multimap<Register,std::vector<Register>,Register::hash>::const_iterator iter,iterLimit;

   public:
   /// Constructor
   HashJoin(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right,const Register* leftValue,const Register* rightValue);
   /// Destructor
   ~HashJoin();

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
