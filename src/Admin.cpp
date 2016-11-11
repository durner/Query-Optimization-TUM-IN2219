#include "Database.hpp"
#include "Table.hpp"
#include "Attribute.hpp"
#include "operator/Printer.hpp"
#include "operator/Tablescan.hpp"
#include <iostream>
#include <cstdlib>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
static void initDB(const string& name)
   // Create a new database
{
   Database db;
   db.create(name);
   db.close();

   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void createTable(const string& dbName,const string& tableName,int count,char* rest[])
   // Create a new table
{
   Database db;
   db.open(dbName);
   Table& table=db.createTable(tableName);

   for (int index=0;index<count;index++) {
      string name=rest[index++];
      bool key=false;
      if ((index<count)&&(string(rest[index])=="key")) { key=true; index++; }
      if (index>=count) {
         cerr << "invalid attribute specification" << endl;
         return;
      }
      Attribute::Type type;
      string t=rest[index];
      if (t=="int") type=Attribute::Type::Int; else
      if (t=="double") type=Attribute::Type::Double; else
      if (t=="bool") type=Attribute::Type::Bool; else
      if (t=="string") type=Attribute::Type::String; else
         { cerr << "invalid attribute type " << t << endl; return; }
      table.addAttribute(name,type,key);
   }
   db.close();

   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void dropTable(const string& dbName,const string& tableName)
   // Remove a table
{
   Database db;
   db.open(dbName);
   db.dropTable(tableName);
   db.close();
   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void insertValues(const string& dbName,const string& tableName,unsigned count,char* rest[])
   // Insert values into a database
{
   Database db;
   db.open(dbName);
   if (!db.hasTable(tableName)) {
      cerr << "unknown table " << tableName << endl;
      return;
   }
   Table& table=db.getTable(tableName);
   if (count!=table.getAttributeCount()) {
      cerr << "the table has " << table.getAttributeCount() << " columns, " << count << " values were provided" << endl;
      return;
   }

   vector<Register> values;
   for (unsigned index=0;index<count;index++) {
      Register r;
      const Attribute& a=table.getAttribute(index);
      switch (a.getType()) {
         case Attribute::Type::Int: r.setInt(atoi(rest[index])); break;
         case Attribute::Type::Double: r.setDouble(atof(rest[index])); break;
         case Attribute::Type::Bool: r.setBool(string(rest[index])=="true"); break;
         default: r.setString(rest[index]);
      }
      values.push_back(r);
   }
   table.insertValues(values);
   db.close();
   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void bulkload(const string& dbName,const string& tableName,int count,char* rest[])
   // Bulkload
{
   Database db;
   db.open(dbName);
   if (!db.hasTable(tableName)) {
      cerr << "unknown table " << tableName << endl;
      return;
   }
   Table& table=db.getTable(tableName);
   if (count!=1) {
      cerr << "source file expected" << endl;
      return;
   }

   ifstream in(rest[0]);
   if (!in.is_open()) {
      cerr << "unable to open " << rest[0] << endl;
      return;
   }
   vector<Register> values;
   values.resize(table.getAttributeCount());
   string line;
   while (getline(in,line)) {
      auto last=line.begin(); auto writer=0u;
      for (auto iter=line.begin(),limit=line.end();iter!=limit;++iter)
         if ((*iter)=='|') {
            string d=string(last,iter);
            switch (table.getAttribute(writer).getType()) {
               case Attribute::Type::Int: values[writer].setInt(atoi(d.c_str())); break;
               case Attribute::Type::Double: values[writer].setDouble(atof(d.c_str())); break;
               case Attribute::Type::Bool: values[writer].setBool(d=="true"); break;
               default: values[writer].setString(d);
            }
            ++writer; last=iter+1;
         }
      table.insertValues(values);
   }
   db.close();
   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void dumpTable(const string& dbName,const string& tableName)
   // Show the content of a table
{
   Database db;
   db.open(dbName);
   if (!db.hasTable(tableName)) {
      cerr << "unknown table " << tableName << endl;
      return;
   }
   Table& table=db.getTable(tableName);

   std::unique_ptr<Tablescan> scan(new Tablescan(table));
   Printer p(move(scan));
   p.open();
   while (p.next());
   p.close();

   db.close();
   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void runStats(const string& dbName)
   // Update the statistics
{
   Database db;
   db.open(dbName);
   db.runStats();
   db.close();
   cout << "ok" << endl;
}
//---------------------------------------------------------------------------
static void showHelp(const char* argv0)
   // Display a short help
{
   cout << "usage: " << argv0 << " [cmd] [db] [arg(s)]" << endl
        << "known commands:" << endl
        << "initdb [db] - creates a new database" << endl
        << "createtable [db] [table] [attributes] - creates a new table" << endl
        << "droptable [db] [table] - deletes a table" << endl
        << "insertvalues [db] [table] [values] - insert values into a table" << endl
        << "dumptable [db] [table] - show the content of a table" << endl
        << "runstats [db] - update the statistics" << endl;
}
//---------------------------------------------------------------------------
int main(int argc,char* argv[])
   // Entry point
{
   if (argc<3) { showHelp(argv[0]); return 1; }

   string cmd=argv[1],db=argv[2];
   if (cmd=="initdb") initDB(db); else
   if (cmd=="runstats") runStats(db); else {
      if (argc<4) {
         cerr << "no table specified!" << endl;
         showHelp(argv[0]);
         return 1;
      }
      string table=argv[3];
      if (cmd=="createtable") createTable(db,table,argc-4,argv+4); else
      if (cmd=="droptable") dropTable(db,table); else
      if (cmd=="insertvalues") insertValues(db,table,argc-4,argv+4); else
      if (cmd=="bulkload") bulkload(db,table,argc-4,argv+4); else
      if (cmd=="dumptable") dumpTable(db,table); else
         { showHelp(argv[0]); return 1; }
   }
}
//---------------------------------------------------------------------------
