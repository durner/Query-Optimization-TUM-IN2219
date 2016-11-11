#include "Database.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Database::Database()
   : dirty(false)
   // Constructor
{
}
//---------------------------------------------------------------------------
Database::~Database()
   // Destructor
{
   close();
}
//---------------------------------------------------------------------------
static string getParent(const std::string& name)
   // Find the parent directory
{
   if (name.rfind('/')!=string::npos)
      return name.substr(0,name.rfind('/')+1);
   if (name.rfind('\\')!=string::npos)
      return name.substr(0,name.rfind('\\')+1);
   return "";
}
//---------------------------------------------------------------------------
void Database::open(const std::string& name)
   // Open an existing database
{
   close();

   repoFile=name;
   baseDir=getParent(name);
   read();
}
//---------------------------------------------------------------------------
void Database::create(const std::string& name)
   // Create a new database
{
   close();

   repoFile=name;
   baseDir=getParent(name);
   dirty=true;
}
//---------------------------------------------------------------------------
bool Database::isDirty() const
   // Dirty?
{
   if (dirty)
      return true;
   for (auto iter=tables.begin(),limit=tables.end();iter!=limit;++iter)
      if ((*iter).second.isDirty())
         return true;
   return false;
}
//---------------------------------------------------------------------------
void Database::close()
   // Close the database
{
   if (isDirty())
      write();
}
//---------------------------------------------------------------------------
void Database::read()
   // Read the metadata information
{
   ifstream in(repoFile);
   if (!in.is_open()) {
      cerr << "unable to read " << repoFile << endl;
      throw;
   }

   string tableName;
   while (getline(in,tableName)) {
      Table& table=tables[tableName];
      table.file=baseDir+tableName;
      table.indexFile=baseDir+tableName+".idx";
      table.read(in);
   }
}
//---------------------------------------------------------------------------
void Database::write()
   // Write the metadata inforamtion
{
   ofstream out(repoFile);
   if (!out.is_open()) {
      cerr << "unable to write " << repoFile << endl;
      throw;
   }

   for (auto iter=tables.begin(),limit=tables.end();iter!=limit;++iter) {
      const std::string& tableName=(*iter).first;
      Table& table=(*iter).second;
      out << tableName << endl;
      table.write(out);
   }
   dirty=false;
}
//---------------------------------------------------------------------------
Table& Database::createTable(const std::string& name)
   // Create a table
{
   if (tables.count(name)) {
      cerr << "table " << name << " already exists" << endl;
      throw;
   }

   string tableFile=baseDir+name,indexFile=tableFile+".idx";
   remove(tableFile.c_str());
   { ofstream out(tableFile); if (!out.is_open()) { cerr << "unable to create " << tableFile << endl; throw; } }
   remove(indexFile.c_str());
   { ofstream out(indexFile); if (!out.is_open()) { cerr << "unable to create " << indexFile << endl; throw; } }

   Table& table=tables[name];
   table.file=tableFile;
   table.indexFile=indexFile;
   dirty=true;

   return table;
}
//---------------------------------------------------------------------------
void Database::dropTable(const std::string& name)
   // Drop a table
{
   if (!tables.count(name)) {
      cerr << "table " << name << " not found" << endl;
      throw;
   }

   Table& table=tables[name];
   remove(table.file.c_str());
   remove(table.indexFile.c_str());

   tables.erase(name);
   dirty=true;
}
//---------------------------------------------------------------------------
Table& Database::getTable(const std::string& name)
   // Get a table
{
   if (!tables.count(name)) {
      cerr << "table " << name << " not found" << endl;
      throw;
   }
   return tables[name];
}
//---------------------------------------------------------------------------
void Database::runStats()
   // Update the statistics
{
   for (auto iter=tables.begin(),limit=tables.end();iter!=limit;++iter)
      ((*iter).second).runStats();
}
//---------------------------------------------------------------------------
