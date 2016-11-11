#ifndef H_Table
#define H_Table
//---------------------------------------------------------------------------
#include "Attribute.hpp"
#include <fstream>
#include <map>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
class Database;
class Tablescan;
class Indexscan;
//---------------------------------------------------------------------------
/// A database table
class Table
{
   private:
   /// The file
   std::string file;
   /// The index file
   std::string indexFile;
   /// The cardinality
   unsigned cardinality;
   /// The attributes
   std::vector<Attribute> attributes;
   /// The indices
   std::vector<std::map<Register,unsigned>> indices;
   /// The I/O interface
   std::fstream io;
   /// Unwritten changes?
   bool dirty;

   friend class Database;
   friend class Tablescan;
   friend class Indexscan;

   void operator=(const Table&) = delete;

   public:
   /// Constructor
   Table();
   /// Constructor
   Table(const std::string& file,const std::string& indexFile);
   /// Copy-Constructor
   Table(const Table& source);
   /// Destructor
   ~Table();

   //// Dirty?
   bool isDirty() const { return dirty; }
   /// Add an attribute
   void addAttribute(const std::string& name,Attribute::Type type,bool key);
   /// Read
   void read(std::istream& in);
   /// Write
   void write(std::ostream& out);

   /// Insert a new tuple
   void insertValues(const std::vector<Register>& values);
   /// Update the statistics
   void runStats();

   /// The cardinality
   unsigned getCardinality() { return cardinality; }
   /// The number of attributes
   unsigned getAttributeCount() { return attributes.size(); }
   /// A specific attribute
   const Attribute& getAttribute(unsigned index) const { return attributes[index]; }
   /// Search for a specific attribute
   int findAttribute(const std::string& name);
};
//---------------------------------------------------------------------------
#endif
