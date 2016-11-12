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
  if (argc < 2)
    return -1;

  QueryParser qp("data/uni", argv[1]);

  std::cout << "Query correct?: " << qp.ParseSQLQuery() << std::endl;
}
//---------------------------------------------------------------------------
