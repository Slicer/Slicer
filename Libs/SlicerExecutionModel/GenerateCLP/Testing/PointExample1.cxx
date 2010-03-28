#include "PointExample1CLP.h"

int main (int argc, char *argv[])
{
  PARSE_ARGS;

  std::cout << "seed.size(): " << seed.size() << std::endl;
  for (unsigned int i = 0; i < seed.size(); i++)
    {
    std::cout << "Seed[" << i << "]: ";
    for (unsigned int j = 0; j < seed[i].size(); j++)
      {
      std::cout << seed[i][j] << " ";
      }
    std::cout << std::endl;
    }

  std::cout << "Region: ";
  for (unsigned int j = 0; j < region.size(); j++)
    {
    std::cout << region[j] << " ";
    }
  std::cout << std::endl;
  return 0;
}

