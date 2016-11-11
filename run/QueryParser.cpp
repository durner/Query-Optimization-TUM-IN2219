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
using namespace parser;
//---------------------------------------------------------------------------

int main(int argc, char** argv) {
  QueryParser qp("data/uni", argv[0]);

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
