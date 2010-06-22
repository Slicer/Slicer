#include <iostream>
#include "PrettyPrintTable.h"

int main(int, char **)
{
  PrettyPrintTable p;

  p.setTablePad(5);
  p.add(0, 0, "String");

  p.add(0, 1, "SecondColumn");

  p.add(0, 2, "4C");

  p.add(0, 3, "5C");

  p.add(1, 0, "Integers");

  p.add(1, 1, 1, "%d"); // Similar to %d

  p.add(1, 2, 2, "%d");

  p.add(1, 3, 3, "%d");

  p.add(1, 0, "ZeroPadInt");

  p.add(1, 1, 1, "%02d"); // Similar to %02d in printf

  p.add(1, 2, 2, "%02d");

  p.add(1, 3, 3, "%02d");

  p.add(2, 0, "FloatingPoint");

  p.add(2, 1, 1.0F, "%+5.2f"); // Similar to %5.2f in printf

  p.add(2, 2, 2.0F, "%+5.2f");

  p.add(2, 3, 3.0F, "%+5.2f");

  p.Print(std::cout);

  p.rightJustify();

  p.Print(std::cout);
}
