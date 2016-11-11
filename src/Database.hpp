#ifndef H_Database
#define H_Database
//---------------------------------------------------------------------------
#include "Table.hpp"
#include <string>
#include <map>
//---------------------------------------------------------------------------
/// Manager for the database instance
class Database
{
   private:
   /// The database schema file
   std::string repoFile;
   /// The base directory
   std::string baseDir;
   /// All tables
   std::map<std::string,Table> tables;
   /// Unwritten changes?
   bool dirty;

   /// Dirty?
   bool isDirty() const;
   /// Read the metadata information
   void read();
   /// Write the metadata information
   void write();

   Database(const Database&) = delete;
   Database& operator=(const Database&) = delete;

   public:
   /// Constructor
   Database();
   /// Destructor
   ~Database();

   /// Open an existing database
   void open(const std::string& name);
   /// Create a new datavase
   void create(const std::string& name);
   /// Close the database
   void close();

   /// Create a new table
   Table& createTable(const std::string& name);
   /// Drop a table
   void dropTable(const std::string& name);
   /// Has a table?
   bool hasTable(const std::string& name) const { return tables.count(name); }
   /// Get a table
   Table& getTable(const std::string& name);

   /// Update the statistics
   void runStats();
};
//---------------------------------------------------------------------------
#endif
