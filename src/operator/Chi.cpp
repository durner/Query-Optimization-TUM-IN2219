#include "operator/Chi.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
void Chi::Add(const Register& a,const Register& b,Register& result)
   // Add
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setInt(a.getInt()+b.getInt()); break;
      case Register::State::Double: result.setDouble(a.getDouble()+b.getDouble()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::Div(const Register& a,const Register& b,Register& result)
   // Divide
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setInt(a.getInt()/b.getInt()); break;
      case Register::State::Double: result.setDouble(a.getDouble()/b.getDouble()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::Equal(const Register& a,const Register& b,Register& result)
   // Compare
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setBool(a.getInt()==b.getInt()); break;
      case Register::State::Double: result.setBool(a.getDouble()==b.getDouble()); break;
      case Register::State::Bool: result.setBool(a.getBool()==b.getBool()); break;
      case Register::State::String: result.setBool(a.getString()==b.getString()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::NotEqual(const Register& a,const Register& b,Register& result)
   // Compare
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setBool(a.getInt()!=b.getInt()); break;
      case Register::State::Double: result.setBool(a.getDouble()!=b.getDouble()); break;
      case Register::State::Bool: result.setBool(a.getBool()!=b.getBool()); break;
      case Register::State::String: result.setBool(a.getString()!=b.getString()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::Less(const Register& a,const Register& b,Register& result)
   // Compare
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setBool(a.getInt()<b.getInt()); break;
      case Register::State::Double: result.setBool(a.getDouble()<b.getDouble()); break;
      case Register::State::Bool: result.setBool(a.getBool()<b.getBool()); break;
      case Register::State::String: result.setBool(a.getString()<b.getString()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
void Chi::LessOrEqual(const Register& a,const Register& b,Register& result)
   // Compare
{
   if (a.getState()==b.getState()) switch (a.getState()) {
      case Register::State::Int: result.setBool(a.getInt()<=b.getInt()); break;
      case Register::State::Double: result.setBool(a.getDouble()<=b.getDouble()); break;
      case Register::State::Bool: result.setBool(a.getBool()<=b.getBool()); break;
      case Register::State::String: result.setBool(a.getString()<=b.getString()); break;
      default: result.setUnbound();
   } else result.setUnbound();
}
//---------------------------------------------------------------------------
Chi::Chi(unique_ptr<Operator>&& input,Operation op,const Register* left,const Register* right)
   : input(move(input)),op(op),left(left),right(right)
   // Constructor
{
}
//---------------------------------------------------------------------------
Chi::~Chi()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Chi::open()
   // Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Chi::next()
   // Get the next tuple
{
   // Produce a tuple
   if (!input->next())
      return false;

   // Calculate the value
   op(*left,*right,output);

   return true;
}
//---------------------------------------------------------------------------
void Chi::close()
   // Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Chi::getOutput() const
   // Get all produced values
{
   vector<const Register*> result=input->getOutput();
   result.push_back(&output);
   return result;
}
//---------------------------------------------------------------------------
