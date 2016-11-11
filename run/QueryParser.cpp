#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"
#include "operator/Printer.hpp"
#include "operator/Chi.hpp"
#include "QueryParser.hpp"
#include <iostream>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

int main(int argc, char** argv) {
  std::cout << "Have " << argc << " arguments:" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << std::endl;

  Database db;
  db.open("data/uni");
  try {
    db.getTable("studenten");
  }
  catch (const std::exception& e){
    return -1;
  }
  return 0;
}
//---------------------------------------------------------------------------
