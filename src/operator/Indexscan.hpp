#ifndef H_operator_IndexScan
#define H_operator_IndexScan
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include "Table.hpp"
//---------------------------------------------------------------------------
/// An indexscan operator
class Indexscan : public Operator
{
   private:
   /// The buffer size
   static const unsigned bufferSize = 4096;

   /// The table
   Table& table;
   /// The index
   std::map<Register,unsigned>& index;
   /// The iterator over the index
   std::map<Register,unsigned>::const_iterator iter,iterLimit;
   /// The bounds
   const Register* lowerBound,*upperBound;
   /// Buffer pointers
   unsigned bufferStart,bufferStop;
   /// Construction helper
   std::string buf;
   /// The output
   std::vector<Register> output;
   /// A small buffer
   char buffer[bufferSize];

   public:
   /// Constructor
   Indexscan(Table& scale,unsigned indexAttribute,const Register* lowerBounds,const Register* upperBounds);
   /// Destructor
   ~Indexscan();

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
