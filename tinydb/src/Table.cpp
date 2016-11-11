#include "Table.hpp"
#include "operator/Tablescan.hpp"
#include <set>
#include <sstream>
#include <cstdlib>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Table::Table()
   : cardinality(0),dirty(false)
   // Constructor
{
}
//---------------------------------------------------------------------------
Table::Table(const string& file,const string& indexFile)
   : file(file),indexFile(indexFile),cardinality(0),dirty(false)
   // Constructor
{
}
//---------------------------------------------------------------------------
Table::Table(const Table& source)
   : file(source.file),indexFile(source.indexFile),cardinality(source.cardinality),
     attributes(source.attributes),indices(source.indices),dirty(source.dirty)
   // Copy-Constructor
{
}
//---------------------------------------------------------------------------
Table::~Table()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Table::addAttribute(const string& name,Attribute::Type type,bool key)
{
   if (findAttribute(name)>=0) {
      cerr << "attribute " << name << " already defined" << endl;
      throw;
   }

   Attribute a;
   a.name=name;
   a.type=type;
   a.key=key;
   attributes.push_back(a);
   indices.resize(indices.size()+1);

   dirty=true;
}
//---------------------------------------------------------------------------
static void readIndex(istream& in,map<Register,unsigned>& index,Attribute::Type type,unsigned cardinality)
   // Read an index
{
   unsigned count;
   in.read(reinterpret_cast<char*>(&count),sizeof(count));
   if (count!=cardinality) {
      cerr << "index corruption encountered, reload the database" << endl;
      throw;
   }
   index.clear();

   switch (type) {
      case Attribute::Type::Int:
         for (unsigned index2=0;index2<count;++index2) {
            int val; unsigned pos;
            in.read(reinterpret_cast<char*>(&val),sizeof(val)); in.read(reinterpret_cast<char*>(&pos),sizeof(pos));
            Register r; r.setInt(val);
            index[r]=pos;
         }
         break;
      case Attribute::Type::Double:
         for (unsigned index2=0;index2<count;++index2) {
            double val; unsigned pos;
            in.read(reinterpret_cast<char*>(&val),sizeof(val)); in.read(reinterpret_cast<char*>(&pos),sizeof(pos));
            Register r; r.setDouble(val);
            index[r]=pos;
         }
         break;
      case Attribute::Type::Bool:
         for (unsigned index2=0;index2<count;++index2) {
            bool val; unsigned pos;
            in.read(reinterpret_cast<char*>(&val),sizeof(val)); in.read(reinterpret_cast<char*>(&pos),sizeof(pos));
            Register r; r.setBool(val);
            index[r]=pos;
         }
         break;
      case Attribute::Type::String:
         for (unsigned index2=0;index2<count;++index2) {
            unsigned valLen,pos; string val;
            in.read(reinterpret_cast<char*>(&valLen),sizeof(valLen));
            val.resize(valLen);
            in.read(const_cast<char*>(val.data()),valLen);
            in.read(reinterpret_cast<char*>(&pos),sizeof(pos));
            Register r; r.setString(val);
            index[r]=pos;
         }
         break;
   }
}
//---------------------------------------------------------------------------
void Table::read(istream& in)
   // Read
{
   string line;
   getline(in,line);
   cardinality=atoi(line.c_str());

   attributes.clear();
   while (getline(in,line)) {
      if (line=="") break;
      Attribute a;
      stringstream s(line);
      a.read(s);
      attributes.push_back(a);
   }

   indices.resize(attributes.size());
   ifstream indicesIn(indexFile,ios_base::in|ios_base::binary);
   if (!indicesIn.is_open()) {
      cerr << "unable to open " << indexFile << endl;
      throw;
   }
   for (unsigned index=0,limit=attributes.size();index<limit;++index)
      if (attributes[index].key||attributes[index].index)
         readIndex(indicesIn,indices[index],attributes[index].type,cardinality);
   indicesIn.close();

   io.open(file,ios_base::in|ios_base::out|ios_base::binary);
   if (!io.is_open()) {
      cerr << "unable to open " << file << endl;
      throw;
   }
}
//---------------------------------------------------------------------------
static void writeIndex(ostream& out,map<Register,unsigned>& index,Attribute::Type type)
   // Write an index
{
   unsigned count=index.size();
   out.write(reinterpret_cast<char*>(&count),sizeof(count));

   switch (type) {
      case Attribute::Type::Int:
         for (auto iter=index.begin(),limit=index.end();iter!=limit;++iter) {
            int val=(*iter).first.getInt(); unsigned pos=(*iter).second;
            out.write(reinterpret_cast<char*>(&val),sizeof(val)); out.write(reinterpret_cast<char*>(&pos),sizeof(pos));
         }
         break;
      case Attribute::Type::Double:
         for (auto iter=index.begin(),limit=index.end();iter!=limit;++iter) {
            double val=(*iter).first.getDouble(); unsigned pos=(*iter).second;
            out.write(reinterpret_cast<char*>(&val),sizeof(val)); out.write(reinterpret_cast<char*>(&pos),sizeof(pos));
         }
         break;
      case Attribute::Type::Bool:
         for (auto iter=index.begin(),limit=index.end();iter!=limit;++iter) {
            bool val=(*iter).first.getBool(); unsigned pos=(*iter).second;
            out.write(reinterpret_cast<char*>(&val),sizeof(bool)); out.write(reinterpret_cast<char*>(&pos),sizeof(pos));
         }
         break;
      case Attribute::Type::String:
         for (auto iter=index.begin(),limit=index.end();iter!=limit;++iter) {
            const string& val=(*iter).first.getString(); unsigned pos=(*iter).second;
            unsigned valLen=val.length();
            out.write(reinterpret_cast<char*>(&valLen),sizeof(valLen));
            out.write(val.data(),valLen);
            out.write(reinterpret_cast<char*>(&pos),sizeof(pos));
         }
         break;
   }
}
//---------------------------------------------------------------------------
void Table::write(ostream& out)
   // Write
{
   out << cardinality << endl;
   for (auto iter=attributes.begin(),limit=attributes.end();iter!=limit;++iter)
      (*iter).write(out);
   out << endl;

   ofstream indicesOut(indexFile,ios_base::out|ios_base::binary);
   if (!indicesOut.is_open()) {
      cerr << "unable to write " << indexFile << endl;
      throw;
   }
   for (unsigned index=0,limit=attributes.size();index<limit;++index)
      if (attributes[index].key||attributes[index].index)
         writeIndex(indicesOut,indices[index],attributes[index].type);
   indicesOut.close();

   dirty=false;
}
//---------------------------------------------------------------------------
void Table::insertValues(const vector<Register>& values)
   // Insert a new tuple
{
   // Check key constraints first
   for (int index=0,limit=attributes.size();index<limit;++index)
      if (attributes[index].key&&indices[index].count(values[index])) {
         cerr << "key constraint for " << attributes[index].name << " violated" << endl;
         throw;
      }

   // Seek to the end of file and write the new tuple
   io.seekg(0, ios::end);
   unsigned pos=io.tellg();
   for (unsigned index=0,limit=attributes.size();index<limit;++index) {
      if (index) io << ';';
      const Register& v=values[index];
      switch (v.getState()) {
         case Register::State::Unbound: io << "NULL"; break;
         case Register::State::Int: io << v.getInt(); break;
         case Register::State::Double: io << v.getDouble(); break;
         case Register::State::Bool: io << (v.getBool()?"true":"false"); break;
         case Register::State::String:
            for (auto iter=v.getString().begin(),limit=v.getString().end();iter!=limit;++iter) {
               char c=(*iter);
               if ((c==':')||(c=='\\'))
                  io << '\\';
               io << c;
            }
            break;
      }
   }
   io << endl;

   // Update the indices
   for (int index=0,limit=attributes.size();index<limit;++index)
      if (attributes[index].key)
         indices[index][values[index]]=pos;

   cardinality++;
   dirty=true;
}
//---------------------------------------------------------------------------
void Table::runStats()
   // Update the statistics
{
   // Should use a group by operator instead...
   vector<set<Register>> stats;
   stats.resize(attributes.size());

   // Perform the scan
   Tablescan scan(*this);
   vector<const Register*> output=scan.getOutput();
   scan.open();
   while (scan.next())
      for (unsigned index=0,limit=stats.size();index<limit;++index)
         stats[index].insert(*output[index]);
   scan.close();

   // Update the statistics
   for (unsigned index=0,limit=attributes.size();index<limit;++index) {
      attributes[index].uniqueValues=stats[index].size();
      switch (attributes[index].type) {
         case Attribute::Type::Int: attributes[index].size=4; break;
         case Attribute::Type::Double: attributes[index].size=8; break;
         case Attribute::Type::Bool: attributes[index].size=1; break;
         case Attribute::Type::String:
            attributes[index].size=0;
            for (auto iter=stats[index].begin(),limit=stats[index].end();iter!=limit;++iter)
               attributes[index].size+=(*iter).getString().length();
            if (!stats[index].empty())
               attributes[index].size/=stats[index].size();
            break;
      }
      if (!stats[index].empty()) {
         attributes[index].minValue=*stats[index].begin();
         attributes[index].maxValue=*(--stats[index].end());
      }
   }
   dirty=true;
}
//---------------------------------------------------------------------------
int Table::findAttribute(const string& name)
   // Search for a specific attribute
{
   for (unsigned index=0,limit=attributes.size();index<limit;++index)
      if (attributes[index].getName()==name)
         return index;
   return -1;
}
//---------------------------------------------------------------------------
