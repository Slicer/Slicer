/*=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   GenerateCLP
  Module:    $URL: http://svn.na-mic.org:8000/svn/NAMICSandBox/trunk/CommandLineAPI/GenerateCLP.cxx $
  Date:      $Date: 2006-04-21 15:51:08 -0400 (Fri, 21 Apr 2006) $
  Version:   $Revision: 957 $

=========================================================================*/

/* Generate command line processing code from an xml description
   Usage: GenerateCLP input_xml_file output_include_file

   This program generates source code that processes command line
   arguments. The arguments are described in an xml file. The output
   include file defines a macro PARSE_ARGS. This macro generates the
   declarations for each command line argument and generates code to
   parse the command line. In addition to user specified coammnd line
   arguments, cod eto echo the xml file and code to print the command
   line arguments is also generated.

   Typical usage is:
   GenerateCLP foo.xml fooCLP.h

   foo.cxx contains:
   #include fooCLP.h
   int main (int argc, char *argv[])
   {
     PARSE_ARGS;
         .
         . Code to implement foo
         .
   }

   This first version of the code uses the TCLAP Templated C++ Command
   Line Parser (http://tclap.sourceforge.net). TCLAP is attractive
   because it is implemented entirely in header files. Other command line
   parsing may be supported in the future.

  GenerateCLP uses the expat XML parser (http://expat.sourceforge.net)
  to process the XML file.

  The generated C++ code relies on the kwsys library
  (http://www.cmake.org) to provide a portable implementaion of string
  streams.
*/


#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "expat.h"
#include <string>
#include <vector>

#include "GenerateCLP.h"

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
  CommandLineArg *m_Current;             /* The current command line */
                                         /* argument */
  bool m_Debug;                          /* Debug flag */
  bool m_Error;                          /* Error detected */
  ParserState():m_Debug(false),m_Error(false){};
};
  
/* Generate the preamble to the code. This includes the required
 * include files and code to process comma separated arguments.
 */
void GeneratePre(std::ofstream &, int, char *[]);

/* Generate the last statements. This defines the PARSE_ARGS macro */
void GeneratePost(std::ofstream &);

/* Generate the code that echos the XML file that describes the
 * command line arguments.
 */
void GenerateXML(std::ofstream &, std::string);

/* Generate the code that uses TCLAP to parse the command line
 * arguments.
 */
void GenerateTCLAP(std::ofstream &, ParserState &);

/* Generate code to echo the command line arguments and their values. */
void GenerateEchoArgs(std::ofstream &, ParserState &);

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


/***************************
 * expat callbacks to process the XML
 ***************************/
static void
startElement(void *userData, const char *name, const char **)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  CommandLineArg *arg = ps->m_Current;
  if (ps->m_Debug) std::cout << name << std::endl;

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

static void
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
    if (ps->m_Debug) std::cout << "--------------------" << ps->m_LastTag << std::endl;
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

static void
charData(void *userData, const char *s, int len)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  if (len)
    {
    std::string str(s,len);
    ps->m_LastTag += str;
    if (ps->m_Debug) std::cout << "|" << str << "|" << std::endl;
    }
}

int
main(int argc, char *argv[])
{
#include "GenerateCLP.clp"

  ParserState parserState;

  XML_Parser parser = XML_ParserCreate(NULL);
  int done;

  parserState.m_Current = 0;

  XML_SetUserData(parser, static_cast<void *>(&parserState));
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charData);
  std::ifstream fin(InputXML.c_str(),std::ios::in);
  if (fin.fail())
    {
    std::cerr << argv[0] << ": Cannot open " << InputXML << " for input" << std::endl;
    perror(argv[0]);
    return EXIT_FAILURE;
    }

  // Get the length of the file
  fin.seekg (0, std::ios::end);
  size_t len = fin.tellg();
  fin.seekg (0, std::ios::beg);
  char * XML = new char[len];
  do {
  fin.read (XML, len);
    done = true;
    if (XML_Parse(parser, XML, len, done) == 0) {
      fprintf(stderr,
              "%s at line %d\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
      return 1;
    }
  } while (!done);
  fin.close();
  XML_ParserFree(parser);

// Print each command line arg
  std::cerr << "GenerateCLP: Found " << parserState.m_AllArgs.size() << " command line arguments" << std::endl;
  if (parserState.m_Error)
    {
    std::cerr << "GenerateCLP: One or more errors detected. Code generation aborted." << std::endl;
    return EXIT_FAILURE;
    }
// Do the hard stuff
  std::ofstream sout(OutputCxx.c_str(),std::ios::out);
  GeneratePre(sout, argc, argv);
  GenerateXML(sout, InputXML);
  GenerateTCLAP(sout, parserState);
  GenerateEchoArgs(sout, parserState);
  GeneratePost(sout);

  return (EXIT_SUCCESS);
}

void GeneratePre(std::ofstream &sout, int argc, char *argv[])
{
  sout << "// This file was automatically generated by:" << std::endl;
  sout << "// ";
  for (int i = 0; i < argc; i++)
    {
    sout << " " << argv[i];
    }
  sout << std::endl;
  sout << "//" << std::endl;
  sout << "#include <stdio.h>" << std::endl;
  sout << "#include <string.h>" << std::endl;
  sout << "#include <stdlib.h>" << std::endl;
  sout << "" << std::endl;
  sout << "#include <iostream>" << std::endl;
  sout << "#include \"tclap/CmdLine.h\"" << std::endl;
  sout << "#include <itksys/ios/sstream>" << std::endl;
  sout << "" << std::endl;
  sout << "void" << std::endl;
  sout << "splitString (std::string &text," << std::endl;
  sout << "             std::string &separators," << std::endl;
  sout << "             std::vector<std::string> &words)" << std::endl;
  sout << "{" << std::endl;
  sout << "  int n = text.length();" << std::endl;
  sout << "  int start, stop;" << std::endl;
  sout << "  start = text.find_first_not_of(separators);" << std::endl;
  sout << "  while ((start >= 0) && (start < n))" << std::endl;
  sout << "    {" << std::endl;
  sout << "    stop = text.find_first_of(separators, start);" << std::endl;
  sout << "    if ((stop < 0) || (stop > n)) stop = n;" << std::endl;
  sout << "    words.push_back(text.substr(start, stop - start));" << std::endl;
  sout << "    start = text.find_first_not_of(separators, stop+1);" << 
    std::endl;
  sout << "    }" << std::endl;
  sout << "}" << std::endl;

}

void GenerateXML(std::ofstream &sout, std::string XMLFile)
{
  std::string EOL(" \\");
  char linec[2048];
  std::ifstream fin(XMLFile.c_str(),std::ios::in);

  sout << "#define GENERATE_XML \\" << std::endl;
  // Generate special section to produce xml description
  sout << "  if (argc >= 2 && (strcmp(argv[1],\"--xml\") == 0))" << EOL << std::endl;
  sout << "    {" << EOL << std::endl;

  while (!fin.eof())
    {
    fin.getline (linec, 2048);
    // replace quotes with escaped quotes
    std::string line(linec);
    std::string cleanLine;
    for (size_t j = 0; j < line.length(); j++)
      {
      if (line[j] == '\"')
        {
        cleanLine += "\\\"";
        }
      else
        {
        cleanLine += line[j];
        }
      }
    sout << "std::cout << \"" << cleanLine << "\" << std::endl;" << EOL << std::endl;
    }
  sout << "    return EXIT_SUCCESS;" << EOL << std::endl;
  sout << "    }" << std::endl;

}

void GenerateEchoArgs(std::ofstream &sout, ParserState &ps)
{
  std::string EOL(" \\");
  sout << "#define GENERATE_ECHOARGS \\" << std::endl;

  sout << "if (echoSwitch)" << EOL << std::endl;
  sout << "{" << EOL << std::endl;
  sout << "std::cout << \"Command Line Arguments\" << std::endl;" << EOL << std::endl;
  for (unsigned int i = 0; i < ps.m_AllArgs.size(); i++)
    {
    if (ps.m_AllArgs[i].NeedsTemp())
      {
      sout << "std::cout << "
           << "\"    "
           << ps.m_AllArgs[i].m_Variable
           << ": \";"
           << EOL << std::endl;
      sout << "for (unsigned int _i =0; _i < "
           << ps.m_AllArgs[i].m_Variable
           << ".size(); _i++)"
           << EOL << std::endl;
      sout << "{"
           << EOL << std::endl;
      sout << "std::cout << "
           << ps.m_AllArgs[i].m_Variable
           << "[_i]"
           << " << \", \";"
           << EOL << std::endl;
      sout << "}"
           << EOL << std::endl;
      sout << "std::cout <<std::endl;"
           << EOL << std::endl;

      }
    else
      {
      sout << "std::cout << "
           << "\"    "
           << ps.m_AllArgs[i].m_Variable
           << ": \" << "
           << ps.m_AllArgs[i].m_Variable
           << " << std::endl;"
           << EOL << std::endl;
      }
    }
  sout << "}" << std::endl;
}

void GenerateTCLAP(std::ofstream &sout, ParserState &ps)
{

  std::string EOL(" \\");
  sout << "#define GENERATE_TCLAP \\" << std::endl;

  // Add a switch argument to echo command line arguments
  CommandLineArg echoSwitch;
  echoSwitch.m_CxxType = "bool";
  echoSwitch.m_Variable = "echoSwitch";
  echoSwitch.m_LongFlag = "--echo";
  echoSwitch.m_Description = "Echo the command line arguments";
  echoSwitch.m_Default = "false";
  ps.m_AllArgs.push_back (echoSwitch);

  // Add a switch argument to produce xml output
  CommandLineArg xmlSwitch;
  xmlSwitch.m_CxxType = "bool";
  xmlSwitch.m_Variable = "xmlSwitch";
  xmlSwitch.m_LongFlag = "--xml";
  xmlSwitch.m_Description = "Produce xml description of command line arguments";
  xmlSwitch.m_Default = "false";
  ps.m_AllArgs.push_back (xmlSwitch);
  
  // First pass generates argument declarations
  for (unsigned int i = 0; i < ps.m_AllArgs.size(); i++)
    {
    if (ps.m_AllArgs[i].NeedsQuotes())
      {
      sout << "    "
           << "std::string"
           << " "
           << ps.m_AllArgs[i].m_Variable;
      if (ps.m_AllArgs[i].NeedsTemp())
        {
        sout << "Temp";
        }

      if (!ps.m_AllArgs[i].HasDefault())
        {    
        sout << ";"
             << EOL << std::endl;
        }
      else
        {
        sout << " = "
             << "\""
             << ps.m_AllArgs[i].m_Default
             << "\""
             << ";"
             << EOL << std::endl;
        }
      if (ps.m_AllArgs[i].NeedsTemp())
        {
        sout << "    "
             << ps.m_AllArgs[i].m_CxxType
             << " "
             << ps.m_AllArgs[i].m_Variable
             << ";"
             << EOL << std::endl;
        }
      }
    else
      {
      sout << "    "
           << ps.m_AllArgs[i].m_CxxType
           << " "
           << ps.m_AllArgs[i].m_Variable;
      if (!ps.m_AllArgs[i].HasDefault())
        {    
        sout << ";"
             << EOL << std::endl;
        }
      else
        {
        sout << " = "
             << ps.m_AllArgs[i].m_Default
             << ";"
             << EOL << std::endl;
        }
      }
    }

  sout << "try" << EOL << std::endl;
  sout << "  {" << EOL << std::endl;
  sout << "    TCLAP::CmdLine commandLine (" << EOL << std::endl;
  sout << "      argv[0]," << EOL << std::endl;
  sout << "      " << "\"" << ps.m_Description << "\"," << EOL << std::endl;
  sout << "      " << "\"$Revision: 957 $\" );" << EOL << std::endl << EOL << std::endl;
  sout << "      itksys_ios::ostringstream msg;" << EOL << std::endl;

  // Second pass generates argument declarations
  for (unsigned int i = 0; i < ps.m_AllArgs.size(); i++)
    {
    sout << "    msg.str(\"\");";
    sout << "msg << "
         << "\""
         << ps.m_AllArgs[i].m_Description;
    if (ps.m_AllArgs[i].m_Default.empty())
      {
      sout << "\";";
      }
    else
      {
      sout << " (default: \" "
           << "<< ";
      if (ps.m_AllArgs[i].HasDefault())
        {
        sout << ps.m_AllArgs[i].m_Variable;
        }
      else
        {
        sout << "\"None\"";
        }

      if (ps.m_AllArgs[i].NeedsTemp() && ps.m_AllArgs[i].HasDefault())
        {
        sout << "Temp";
        }
      sout << " << "
           << "\")"
           << "\";"
           << EOL << std::endl;
      }

    if (ps.m_AllArgs[i].m_CxxType == "bool")
      {
      sout << "    TCLAP::SwitchArg "
           << ps.m_AllArgs[i].m_Variable
           << "Arg" << "(\""
           << ps.m_AllArgs[i].m_ShortFlag.replace(0,1,"")
           << "\", \"" 
           << ps.m_AllArgs[i].m_LongFlag.replace(0,2,"")
           << "\", msg.str(), "
           << false
           << ", "
           << "commandLine);"
           << EOL << std::endl << EOL << std::endl;
      }
    else
      {
      if (ps.m_AllArgs[i].m_ShortFlag.empty() && ps.m_AllArgs[i].m_LongFlag.empty())
        {
        sout << "    TCLAP::UnlabeledValueArg<";
        sout << ps.m_AllArgs[i].m_CxxType;
        sout << "> "
             << ps.m_AllArgs[i].m_Variable
             << "Arg" << "(\""
             << ps.m_AllArgs[i].m_Variable
             << "\", msg.str(), "
             << !ps.m_AllArgs[i].HasDefault()
             << ", "
             << ps.m_AllArgs[i].m_Variable;
        sout << ", "
             << "\""
             << ps.m_AllArgs[i].m_CxxType
             << "\""
             << ", "
             << "commandLine);"
             << EOL << std::endl << EOL << std::endl;
        }
      else
        {
        sout << "    TCLAP::ValueArg<";
        if (ps.m_AllArgs[i].NeedsTemp())
          {
          sout << "std::string";
          }
        else
          {
          sout << ps.m_AllArgs[i].m_CxxType;
          }
        sout << "> "
             << ps.m_AllArgs[i].m_Variable
             << "Arg" << "(\""
             << ps.m_AllArgs[i].m_ShortFlag.replace(0,1,"")
             << "\", \"" 
             << ps.m_AllArgs[i].m_LongFlag.replace(0,2,"")
             << "\", msg.str(), "
             << false
             << ", "
             << ps.m_AllArgs[i].m_Variable;
        if (ps.m_AllArgs[i].NeedsTemp())
          {
          sout << "Temp";
          }
        sout << ", "
             << "\""
             << ps.m_AllArgs[i].m_CxxType
             << "\""
             << ", "
             << "commandLine);"
             << EOL << std::endl << EOL << std::endl;
        }
      }
    }
  sout << "    commandLine.parse ( argc, (char**) argv );" << EOL << std::endl;
  
  // Third pass generates access to arguments
  for (unsigned int i = 0; i < ps.m_AllArgs.size(); i++)
    {
    sout << "    "
         << ps.m_AllArgs[i].m_Variable;
    if (ps.m_AllArgs[i].NeedsTemp())
      {
      sout << "Temp";
      }
    sout << " = "
         << ps.m_AllArgs[i].m_Variable
         << "Arg.getValue();"
         << EOL << std::endl;
    }

// Finally, for any arrays, split the strings into words
  for (unsigned int i = 0; i < ps.m_AllArgs.size(); i++)
    {
    if (ps.m_AllArgs[i].NeedsTemp())
      {
      sout << "      {" << EOL << std::endl;
      sout << "      std::vector<std::string> words;"
           << EOL << std::endl;
      sout << "      std::string sep(\",\");"
           << EOL << std::endl;
      sout << "      splitString(" 
           << ps.m_AllArgs[i].m_Variable
           << "Temp"
           << ", "
           << "sep, "
           << "words);"
           << EOL << std::endl;
      sout << "      for (unsigned int j = 0; j < words.size(); j++)"
           << EOL << std::endl;
      sout << "        {"
           << EOL << std::endl;
      sout << "        " 
           << ps.m_AllArgs[i].m_Variable << ".push_back("
           << ps.m_AllArgs[i].m_StringToType
           << "(words[j].c_str()));"
           << EOL << std::endl;
      sout << "        }"
           << EOL << std::endl;
      sout << "      }"
           << EOL << std::endl;
      }
    }

  // Wrapup the block and generate the catch block
  sout << "  }" << EOL << std::endl;
  sout << "catch ( TCLAP::ArgException e )" << EOL << std::endl;
  sout << "  {" << EOL << std::endl;
  sout << "  std::cerr << \"error: \" << e.error() << \" for arg \" << e.argId() << std::endl;" << EOL << std::endl;
  sout << "  exit ( EXIT_FAILURE );" << EOL << std::endl;
  sout << "    }" << std::endl;
}

void GeneratePost(std::ofstream &sout)
{
  sout << "#define PARSE_ARGS GENERATE_XML;GENERATE_TCLAP;GENERATE_ECHOARGS" << std::endl;
}
