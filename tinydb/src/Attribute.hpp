#ifndef H_Attribute
#define H_Attribute
//---------------------------------------------------------------------------
#include "Register.hpp"
#include <string>
#include <iostream>
//---------------------------------------------------------------------------
class Table;
//---------------------------------------------------------------------------
/// An table attribute
class Attribute
{
   public:
   /// Possible types
   enum class Type : unsigned { Int, Double, Bool, String };

   private:
   /// Name of the attribute
   std::string name;
   /// Type of the attribute
   Type type;
   /// Average size
   double size;
   /// Number of unique values
   unsigned uniqueValues;
   /// Minimum value
   Register minValue;
   /// Maximum value
   Register maxValue;
   /// Key attribute?
   bool key;
   /// Index available?
   bool index;

   friend class Table;

   public:
   /// Constructor
   Attribute();
   /// Destructor
   ~Attribute();

   /// The name
   const std::string& getName() const { return name; }
   /// The type
   Type getType() const { return type; }
   /// The average size
   double getSize() const { return size; }
   /// The number of unique values
   unsigned getUniqueValues() const { return uniqueValues; }
   /// The minimum value
   const Register& getMinValue() { return minValue; }
   /// The maximum value
   const Register& getMaxValue() { return maxValue; }
   /// Key attribute?
   bool getKey() const { return key; }
   /// Index available?
   bool getIndex() const { return index; }

   /// Read
   void read(std::istream& in);
   /// Write
   void write(std::ostream& out);
};
//---------------------------------------------------------------------------
#endif
