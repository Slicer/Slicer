#include "ModuleDescriptionUtilities.h"
#include <string>

void
replaceSubWithSub(std::string& s, const char *o, const char  *n)
{
  if (s.size())
    {
    std::string from(o), to(n);
    std::string::size_type start = 0;
    while ((start = s.find(from, start)) != std::string::npos)
      {
      s.replace(start, from.size(), to);
      start += to.size();
      }
    }
}

void
trimLeading(std::string& s, const char* extraneousChars)
{
  if (s.size())
    {
    std::string::size_type pos = s.find_first_not_of(extraneousChars);
    if (pos != std::string::npos)
      {
      s = s.substr(pos);
      }
    else
      {
      s = "";
      }
    }
}

void
trimTrailing(std::string& s, const char* extraneousChars)
{
  if (s.size())
    {
    std::string::size_type pos = s.find_last_not_of(extraneousChars);
    if (pos != std::string::npos)
      {
      s = s.substr(0, pos + 1);
      }
    else
      {
      s = "";
      }
    }
}

void
trimLeadingAndTrailing(std::string& s, const char* extraneousChars)
{
  trimLeading(s, extraneousChars);
  trimTrailing(s, extraneousChars);
}

