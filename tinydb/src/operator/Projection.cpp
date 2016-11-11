#include "operator/Projection.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Projection::Projection(unique_ptr<Operator>&& input,const vector<const Register*>& output)
   : input(move(input)),output(output)
   // Constructor
{
}
//---------------------------------------------------------------------------
Projection::~Projection()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Projection::open()
   // Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Projection::next()
   // Get the next tuple
{
   return input->next();
}
//---------------------------------------------------------------------------
void Projection::close()
   // Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Projection::getOutput() const
   // Get all produced values
{
   return output;
}
//---------------------------------------------------------------------------
