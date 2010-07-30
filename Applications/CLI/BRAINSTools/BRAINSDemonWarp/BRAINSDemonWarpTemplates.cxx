#include "BRAINSDemonWarpTemplates.h"

// This function prints the valid pixel types.
void PrintDataTypeStrings(void)
{
  // Prints the Input and output data type strings.
  std::cout << "uchar" << std::endl;
  std::cout << "short" << std::endl;
  std::cout << "ushort" << std::endl;
  std::cout << "int" << std::endl;
  std::cout << "float" << std::endl;
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes
                            // too many segments in one object file for the
                            // intel compiler
  std::cout << "uint" << std::endl;
  std::cout << "double" << std::endl;
#endif
}

// This function compares strings ignoring case.
int CompareNoCase(const std::string & s, const std::string & s2)
{
  // Compare strings.
  std::string::const_iterator p = s.begin();
  std::string::const_iterator p2 = s2.begin();

  while ( p != s.end() && p2 != s2.end() )
    {
    if ( toupper(*p) != toupper(*p2) )
      {
      return ( toupper(*p) < toupper(*p2) ) ? -1 : 1;
      }
    p++;
    p2++;
    }

  return ( s2.size() == s.size() ) ? 0 : ( s.size() < s2.size() ) ? -1 : 1;
}
