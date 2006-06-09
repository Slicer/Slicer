#include "ModuleDescriptionParser.h"

#include "ModuleDescription.h"

#include <iostream>
#include <string>
#include "expat.h"

/*********************
 * Utility procedures to trim leading and trailing characters
 *********************/
void
trimLeading(std::string& s, char* extraneousChars = " \t\n")
{
  if (s.size())
    {
    s = s.substr(s.find_first_not_of(extraneousChars));
    }
}

void
trimTrailing(std::string& s, char* extraneousChars = " \t\n")
{
  if (s.size())
    {
    s = s.substr(0, s.find_last_not_of(extraneousChars)+1);
    }
}

/*
CommandLineArg: A class to hold data parsed from the xml argument
descriptions.
*/
class CommandLineArg
{
public:
  std::string m_Variable;        /* The name of the C++ variable */
  std::string m_ShortFlag;       /* The short flag for the argument */
  std::string m_LongFlag;        /* The long flag for the arugment */
  std::string m_Description;     /* A description for the argument */
  std::string m_Default;         /* The defaulf for the argument */
  std::string m_CxxType;         /* The C++ type for the argument */
  std::string m_StringToType;    /* The function to convert the string */
                                 /* to the C++ type. */
  CommandLineArg()
  {
    std::string nodefault = "-NODEFAULT-";
    m_Default = nodefault;
  }

  /* Comma separated arguments need a temporary variable to store the
   * string
   */
  bool NeedsTemp()
  {
    return (m_CxxType == "std::vector<int>" ||
            m_CxxType == "std::vector<float>" ||
            m_CxxType == "std::vector<double>");
  }
  /* Some types need quotes in the initialization. */
  bool NeedsQuotes()
  {
    return (m_CxxType == "std::vector<int>" ||
            m_CxxType == "std::vector<float>" ||
            m_CxxType == "std::vector<double>" ||
            m_CxxType == "std::string");
  }
  bool HasDefault()
  {
    return (m_Default != "-NODEFAULT-");
  }
};

/* ParserState: A class to keep state information for the parser. This
 * is passed to the expat code as user data.
 */
class ParserState
{
public:
  std::vector<CommandLineArg> m_AllArgs; /* A vector of command line */
                                         /* arguments */
  std::string m_LastTag;                 /* The last tag processed by */
                                         /* expat */
  std::string m_Description;             /* Global descripton */

  ModuleDescription m_ModuleDescription;
  CommandLineArg *m_Current;             /* The current command line */
                                         /* argument */
  bool m_Debug;                          /* Debug flag */
  bool m_Error;                          /* Error detected */
  ParserState():m_Debug(false),m_Error(false){};
};

/***************************
 * expat callbacks to process the XML
 ***************************/
void
startElement(void *userData, const char *name, const char **)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  CommandLineArg *arg = ps->m_Current;

  ps->m_LastTag.clear();

  if (strcmp(name, "integer") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "int";
    }
  else if (strcmp(name, "float") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "float";
    }
  else if (strcmp(name, "double") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "double";
    }
  else if (strcmp(name, "string") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::string";
    }
  else if (strcmp(name, "boolean") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "bool";
    }
  else if (strcmp(name, "integer-vector") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::vector<int>";
    arg->m_StringToType = "atoi";
    }
  else if (strcmp(name, "float-vector") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::vector<float>";
    arg->m_StringToType = "atof";
    }
  else if (strcmp(name, "double-vector") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::vector<double>";
    arg->m_StringToType = "atof";
    }
  else if (strcmp(name, "file") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::string";
    }
  else if (strcmp(name, "directory") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::string";
    }
  else if (strcmp(name, "image") == 0)
    {
    arg = new CommandLineArg;
    arg->m_CxxType = "std::string";
    }
  ps->m_Current = arg;
}

void
endElement(void *userData, const char *name)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  CommandLineArg *arg = ps->m_Current;

  if (strcmp(name, "integer") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "float") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "double") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "string") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "boolean") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "file") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "directory") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "image") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "integer-vector") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "float-vector") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "double-vector") == 0)
    {
    ps->m_AllArgs.push_back(*arg);
    ps->m_Current = 0;
    }
  else if (strcmp(name, "flag") == 0)
    {
    arg->m_ShortFlag = ps->m_LastTag;
    trimLeading(arg->m_ShortFlag);
    trimTrailing(arg->m_ShortFlag);
    }
  else if (strcmp(name, "longflag") == 0)
    {
    arg->m_LongFlag = ps->m_LastTag;
    trimLeading(arg->m_LongFlag);
    trimTrailing(arg->m_LongFlag);
    if (arg->m_LongFlag.find("-",2) != std::string::npos)
      {
      std::cerr << "GenerateCLP: flags cannot contain \"-\" : " << arg->m_LongFlag << std::endl;
      ps->m_Error = true;
      }
    if (arg->m_Variable.empty())
      {
      arg->m_Variable = std::string(ps->m_LastTag);
      trimLeading(arg->m_Variable);
      trimTrailing(arg->m_Variable);
      trimLeading(arg->m_Variable,"-");
      }
    }
  else if (strcmp(name, "name") == 0)
    {
    arg->m_Variable = std::string(ps->m_LastTag);
    trimLeading(arg->m_Variable);
    trimTrailing(arg->m_Variable);
    }
  else if (strcmp(name, "description") == 0)
    {
    if (arg)
      {
      arg->m_Description = ps->m_LastTag;
      trimLeading(arg->m_Description);
      trimTrailing(arg->m_Description);
      }
    else
      {
      if (ps->m_Description.empty())
        {
        ps->m_Description = ps->m_LastTag;
        trimLeading(ps->m_Description);
        trimTrailing(ps->m_Description);
        }
      }
    }
  else if (strcmp(name, "default") == 0)
    {
    arg->m_Default = ps->m_LastTag;
    trimLeading(arg->m_Default);
    trimTrailing(arg->m_Default);
    }
}

void
charData(void *userData, const char *s, int len)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  if (len)
    {
    std::string str(s,len);
    ps->m_LastTag += str;
    }
}

int
ModuleDescriptionParser::Parse( const std::string& xml, ModuleDescription& description)
{
  ParserState parserState;
  parserState.m_ModuleDescription = description;
  
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;

  parserState.m_Current = 0;

  XML_SetUserData(parser, static_cast<void *>(&parserState));
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charData);

  // Get the length of the file
  done = true;
  if (XML_Parse(parser, xml.c_str(), xml.size(), done) == 0)
    {
    fprintf(stderr,
            "%s at line %d\n",
            XML_ErrorString(XML_GetErrorCode(parser)),
            XML_GetCurrentLineNumber(parser));
    return 1;
    }
  
  XML_ParserFree(parser);

  return 0;

}
