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
   arguments, code to echo the xml file and code to print the command
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
#include <itksys/SystemTools.hxx>

#include "GenerateCLP.h"
#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"
#include "ModuleParameterGroup.h"
#include "ModuleParameter.h"

/* A useful string utility */
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

/* Comma separated arguments need a temporary variable to store the
 * string
 */
bool NeedsTemp(const ModuleParameter &parameter)
{
  std::string type = parameter.GetCPPType();
  std::string multi = parameter.GetMultiple();
  return (((type == "std::vector<int>" ||
           type == "std::vector<float>" ||
           type == "std::vector<double>" ||
           type == "std::vector<std::string>") ||
           type == "std::vector<std::vector<float> >"));
}
/* Some types need quotes in the initialization. */
bool NeedsQuotes(const ModuleParameter &parameter)
{
  std::string type = parameter.GetCPPType();
  std::string multi = parameter.GetMultiple();
  return (((type == "std::vector<int>" ||
           type == "std::vector<float>" ||
           type == "std::vector<double>" ||
           type == "std::vector<std::string>" ||
           type == "std::string") &&
           multi != "true") ||
           type == "std::vector<std::vector<float> >"
);
}
bool IsEnumeration(const ModuleParameter &parameter)
{
  std::string type = parameter.GetTag();
  return (type == "string-enumeration" ||
          type == "integer-enumeration" ||
          type == "float-enumeration" ||
          type == "double-enumeration" );
}

bool IsVectorOfVectors(const ModuleParameter &parameter)
{
  std::string type = parameter.GetCPPType();
  return (type == "std::vector<std::vector<float> >");
}

bool HasDefault(const ModuleParameter &parameter)
{
  return (parameter.GetDefault().size() > 0 && parameter.GetMultiple() != "true");
}


/* Generate the preamble to the code. This includes the required
 * include files and code to process comma separated arguments.
 */
void GeneratePre(std::ofstream &, ModuleDescription &, int, char *[]);

/* Generate the last statements. This defines the PARSE_ARGS macro */
void GeneratePost(std::ofstream &, ModuleDescription &);

/* Generate the code that echos the XML file that describes the
 * command line arguments.
 */
void GeneratePluginEntryPoints(std::ofstream &, ModuleDescription &, std::vector<std::string> &, std::string);
void GeneratePluginProcedures(std::ofstream &, ModuleDescription &, std::vector<std::string> &, std::string);
void GenerateLOGO(std::ofstream &, ModuleDescription &, std::vector<std::string> &, std::string);
void GenerateXML(std::ofstream &, ModuleDescription &, std::string);

/* Generate the code that uses TCLAP to parse the command line
 * arguments.
 */
void GenerateTCLAP(std::ofstream &, ModuleDescription &);

/* Generate code to echo the command line arguments and their values. */
void GenerateEchoArgs(std::ofstream &, ModuleDescription &);

/** Generate code to decode the address of a process information
 * structure */
void GenerateProcessInformationAddressDecoding(std::ofstream &sout, ModuleDescription &);

int
main(int argc, char *argv[])
{
  PARSE_ARGS;
  ModuleDescription module;
  ModuleDescriptionParser parser;

  // Read the XML file
  std::ifstream fin(InputXML.c_str(),std::ios::in|std::ios::binary);
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
  char * XML = new char[len+1];
  fin.read (XML, len);
  XML[len] = '\0';

  // Parse the module description
  std::cerr << "GenerateCLP";
  for (int i = 1; i < argc; i++)
    {
    std::cerr << " " << argv[i];
    }
  std::cerr << std::endl;

  if (parser.Parse(XML, module))
    {
    std::cerr << "GenerateCLP: One or more errors detected. Code generation aborted." << std::endl;
    return EXIT_FAILURE;
    }

  // Print each command line arg
  std::cerr << "GenerateCLP: Found " << module.GetParameterGroups().size() << " parameters groups" << std::endl;
  std::vector<ModuleParameterGroup>::const_iterator git;
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    std::cerr << "GenerateCLP: Group \"" << (*git).GetLabel() << "\" has " << (*git).GetParameters().size() << " parameters" << std::endl;
    }

  // Do the hard stuff
  std::ofstream sout(OutputCxx.c_str(),std::ios::out);
  if (sout.fail())
    {
    std::cerr << argv[0] << ": Cannot open " << OutputCxx << " for output" << std::endl;
    perror(argv[0]);
    return EXIT_FAILURE;
    }
  GeneratePre(sout, module, argc, argv);
  if (logoFiles.size() > 0 && !itksys::SystemTools::FileExists(logoFiles[0].c_str()))
    {
    std::cerr << argv[0] << ": Cannot open " << logoFiles[0] << " as a logo file" << std::endl;
    return EXIT_FAILURE;
    }
  GeneratePluginEntryPoints(sout, module, logoFiles, InputXML);
  GeneratePluginProcedures(sout, module, logoFiles, InputXML);
  GenerateLOGO(sout, module, logoFiles, InputXML);
  GenerateXML(sout, module, InputXML);
  GenerateTCLAP(sout, module);
  GenerateEchoArgs(sout, module);
  GenerateProcessInformationAddressDecoding(sout, module);
  GeneratePost(sout, module);
  sout.close();

  return (EXIT_SUCCESS);
}

void GeneratePre(std::ofstream &sout, ModuleDescription &module, int argc, char *argv[])
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
  sout << "#include \"ModuleProcessInformation.h\"" << std::endl;
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
  sout << "    start = text.find_first_not_of(separators, stop+1);" << std::endl;
  sout << "    }" << std::endl;
  sout << "}" << std::endl;

}

void GeneratePluginEntryPoints(std::ofstream &sout, ModuleDescription &module, std::vector<std::string> &logos, std::string XMLFile)
{
  sout << std::endl;
  if (logos.size() == 1)
    {
    sout << "#include \"" << logos[0] << "\"" << std::endl;
    }
  sout << "#ifdef main" << std::endl;
  sout << "// If main defined as a preprocessor symbol, redefine it to the expected entry point." << std::endl;
  sout << "#undef main" << std::endl;
  sout << "#define main ModuleEntryPoint" << std::endl;
  sout << "#ifdef WIN32" << std::endl;
  sout << "#define Module_EXPORT __declspec(dllexport)" << std::endl;
  sout << "#else" << std::endl;
  sout << "#define Module_EXPORT " << std::endl;
  sout << "#endif" << std::endl;
  sout << std::endl;

  sout << "extern \"C\" {" << std::endl;
  sout << "  Module_EXPORT char *GetXMLModuleDescription();" << std::endl;
  sout << "  Module_EXPORT int ModuleEntryPoint(int, char*[]);" << std::endl;
  if (logos.size() == 1)
    {
    sout << "  Module_EXPORT unsigned char *GetModuleLogo(int *width, int *height, int *pixel_size, unsigned long *bufferLength, int *options);" << std::endl;
    }
  sout << "}" << std::endl;
  sout << "#endif" << std::endl;
  sout << std::endl;
}

void GeneratePluginProcedures(std::ofstream &sout, ModuleDescription &module, std::vector<std::string> &logos, std::string XMLFile)
{
  if (logos.size() == 1)
    {
    std::string logo = logos[0];
    std::string fileName = itksys::SystemTools::GetFilenameWithoutExtension (logo);
    sout << "unsigned char *GetModuleLogo(int *width," << std::endl;
    sout << "                             int *height," << std::endl;
    sout << "                             int *pixel_size," << std::endl;
    sout << "                             unsigned long *length," << std::endl;
    sout << "                             int *options)" << std::endl;
    sout << "{" << std::endl;

    sout << "  *width = image_" << fileName << "_width;" << std::endl;
    sout << "  *height = image_" << fileName << "_height;" << std::endl;
    sout << "  *pixel_size = image_" << fileName << "_pixel_size;" << std::endl;
    sout << "  *length = image_" << fileName << "_length;" << std::endl;
    sout << "  *options = 0;" << std::endl;
    sout << "  return const_cast<unsigned char *>(image_" << fileName << ");" << std::endl;
    sout << "}" << std::endl;
    }

  char linec[2048];
  std::ifstream fin(XMLFile.c_str(),std::ios::in);

  sout << "char *GetXMLModuleDescription()" << std::endl;
  sout << "  {" << std::endl;
  sout << "  std::string xml;" << std::endl;

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
    sout << "  xml += \"" << cleanLine << "\\n\";" << std::endl;
    }
  sout << "  char *xmlChar = new char[xml.size()+1];" << std::endl;
  sout << "  memcpy (xmlChar, xml.c_str(), xml.size());" << std::endl;
  sout << "  xmlChar[xml.size()] = '\\0';" << std::endl;
  sout << "  return xmlChar;" << std::endl;
  sout << "  }" << std::endl;

  fin.close();
}

void GenerateLOGO(std::ofstream &sout, ModuleDescription &module, std::vector<std::string> &logos, std::string XMLFile)
{
  if (logos.size() > 0)
    {
    std::string EOL(" \\");

    sout << "#define GENERATE_LOGO \\" << std::endl;
    // Generate special section to produce logo description
    sout << "  if (argc >= 2 && (strcmp(argv[1],\"--logo\") == 0))" << EOL << std::endl;
    sout << "    {" << EOL << std::endl;
    sout << "    int width, height, pixel_size, options;    " << EOL << std::endl;
    sout << "    unsigned long length; " << EOL << std::endl;
    sout << "    unsigned char *logo = GetModuleLogo(&width, &height, &pixel_size, &length, &options); " << EOL << std::endl;
    sout << "    std::cout << \"LOGO\" << std::endl; " << EOL << std::endl;
    sout << "    std::cout << width << std::endl; " << EOL << std::endl;
    sout << "    std::cout << height << std::endl; " << EOL << std::endl;
    sout << "    std::cout << pixel_size << std::endl; " << EOL << std::endl;
    sout << "    std::cout << length << std::endl; " << EOL << std::endl;
    sout << "    std::cout << options << std::endl; " << EOL << std::endl;
    sout << "    std::cout << logo << std::endl; " << EOL << std::endl;
    sout << "    return EXIT_SUCCESS; " << EOL << std::endl;
    sout << "    }" << std::endl;
    }
  else
    {
    sout << "#define GENERATE_LOGO" << std::endl;
    }
}

void GenerateXML(std::ofstream &sout, ModuleDescription &module, std::string XMLFile)
{
  std::string EOL(" \\");

  sout << "#define GENERATE_XML \\" << std::endl;
  // Generate special section to produce xml description
  sout << "  if (argc >= 2 && (strcmp(argv[1],\"--xml\") == 0))" << EOL << std::endl;
  sout << "    {" << EOL << std::endl;
  sout << "    std::cout << GetXMLModuleDescription();" << EOL << std::endl;
  sout << "    return EXIT_SUCCESS;" << EOL << std::endl;
  sout << "    }" << std::endl;
}

void GenerateEchoArgs(std::ofstream &sout, ModuleDescription &module)
{
  std::string EOL(" \\");
  sout << "#define GENERATE_ECHOARGS \\" << std::endl;

  sout << "if (echoSwitch)" << EOL << std::endl;
  sout << "{" << EOL << std::endl;
  sout << "std::cout << \"Command Line Arguments\" << std::endl;" << EOL << std::endl;
  std::vector<ModuleParameterGroup>::const_iterator git;
  std::vector<ModuleParameter>::const_iterator pit;
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    for (pit = git->GetParameters().begin();
         pit != git->GetParameters().end();
         ++pit)
      {
      if (NeedsTemp(*pit) && pit->GetMultiple() != "true")
        {
        sout << "std::cout << "
             << "\"    "
             << pit->GetName()
             << ": \";"
             << EOL << std::endl;
        sout << "for (unsigned int _i =0; _i < "
             << pit->GetName()
             << ".size(); _i++)"
             << EOL << std::endl;
        sout << "{"
             << EOL << std::endl;
        sout << "std::cout << "
             << pit->GetName()
             << "[_i]"
             << " << \", \";"
             << EOL << std::endl;
        sout << "}"
             << EOL << std::endl;
        sout << "std::cout <<std::endl;"
             << EOL << std::endl;
        
        }
      else if (NeedsTemp(*pit) && pit->GetMultiple() == "true")
        {
        sout << "for (unsigned int _i= 0; _i < " << pit->GetName() << "Temp.size(); _i++)" << EOL << std::endl;
        sout << "{" << EOL << std::endl;
        sout << "std::cout << \"" << pit->GetName() << "[\" << _i << \"]: \";" << EOL << std::endl;
        sout << "std::vector<std::string> words;" << EOL << std::endl;
        sout << "words.clear();" << EOL << std::endl;
        sout << "      std::string sep(\",\");" << EOL << std::endl;
        sout << "splitString(" << pit->GetName() << "Temp[_i], sep, words);" << EOL << std::endl;
        sout << "for (unsigned int _j= 0; _j < words.size(); _j++)" << EOL << std::endl;
        sout << "{" << EOL << std::endl;
        sout << "std::cout <<  words[_j] << \" \";" << EOL << std::endl;
        sout << "}" << EOL << std::endl;
        sout << "std::cout << std::endl;" << EOL << std::endl;
        sout << "}" << EOL << std::endl;
        }
      else if (pit->GetMultiple() == "true")
        {
        sout << "std::cout << "
             << "\"    "
             << pit->GetName()
             << ": \";"
             << EOL << std::endl;
        sout << "for (unsigned int _i =0; _i < "
             << pit->GetName()
             << ".size(); _i++)"
             << EOL << std::endl;
        sout << "{"
             << EOL << std::endl;
        sout << "std::cout << "
             << pit->GetName()
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
             << pit->GetName()
             << ": \" << "
             << pit->GetName()
             << " << std::endl;"
             << EOL << std::endl;
        }
      }
    }
  sout << "}" << std::endl;
}

void GenerateTCLAP(std::ofstream &sout, ModuleDescription &module)
{

  std::string EOL(" \\");
  sout << "#define GENERATE_TCLAP \\" << std::endl;

  ModuleParameterGroup autoParameters;

  // Add a switch argument to echo command line arguments
  ModuleParameter echoSwitch;
  echoSwitch.SetTag("boolean");
  echoSwitch.SetCPPType("bool");
  echoSwitch.SetName("echoSwitch");
  echoSwitch.SetLongFlag("echo");
  echoSwitch.SetDescription("Echo the command line arguments");
  echoSwitch.SetDefault("false");
  autoParameters.AddParameter(echoSwitch);

  // Add a switch argument to produce xml output
  ModuleParameter xmlSwitch;
  xmlSwitch.SetTag("boolean");
  xmlSwitch.SetCPPType("bool");
  xmlSwitch.SetName("xmlSwitch");
  xmlSwitch.SetLongFlag("xml");
  xmlSwitch.SetDescription("Produce xml description of command line arguments");
  xmlSwitch.SetDefault("false");
  autoParameters.AddParameter(xmlSwitch);

  // Add an argument to accept an address for storing process
  // information
  ModuleParameter processInformationAddressArg;
  processInformationAddressArg.SetTag("string");
  processInformationAddressArg.SetCPPType("std::string");
  processInformationAddressArg.SetName("processInformationAddressString");
  processInformationAddressArg.SetLongFlag("processinformationaddress");
  processInformationAddressArg.SetDescription("Address of a structure to store process information (progress, abort, etc.).");
  processInformationAddressArg.SetDefault("0");

  autoParameters.AddParameter(processInformationAddressArg);

  // Add the parameter group to the module
  module.AddParameterGroup(autoParameters);
  
  // First pass generates argument declarations
  std::vector<ModuleParameterGroup>::const_iterator git;
  std::vector<ModuleParameter>::const_iterator pit;
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    for (pit = git->GetParameters().begin();
         pit != git->GetParameters().end();
         ++pit)
      {
      if (!IsEnumeration(*pit))
        {
        sout << "    ";
        if (pit->GetMultiple() == "true")
          {
          sout << "std::vector<std::string>";
          }
        else if (NeedsQuotes(*pit))
          {
          sout << "std::string";
          }
        else
          {
          sout << pit->GetCPPType();
          }
        sout << " ";
        sout << pit->GetName();
        if (NeedsTemp(*pit))
          {
          sout << "Temp";
          }
        
        if (!HasDefault(*pit))
          {    
          sout << ";"
               << EOL << std::endl;
          }
        else
          {
          std::string defaultString = pit->GetDefault();
          if ((*pit).GetCPPType() == "bool")
            {
            defaultString = "false";
            }
          replaceSubWithSub(defaultString, "\"", "\\\"");
          sout << " = ";
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          sout << defaultString;
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          sout << ";";
          sout << EOL << std::endl;
          }
        if (NeedsTemp(*pit))
          {
          sout << "    "
               << pit->GetCPPType()
               << " "
               << pit->GetName()
               << ";"
               << EOL << std::endl;
          }
        }
      else // IsEnumeration(*pit)
        {
        sout << "    "
             << pit->GetCPPType()
             << " ";
        sout << pit->GetName();
        if (!HasDefault(*pit))
          {    
          sout << ";"
               << EOL << std::endl;
          }
        else
          {
          sout << " = ";
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          std::string defaultString = pit->GetDefault();
          if ((*pit).GetCPPType() == "bool")
            {
            defaultString = "false";
            }
          replaceSubWithSub(defaultString, "\"", "\\\"");
          sout << defaultString;
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          sout << ";"
               << EOL << std::endl;
          }
        sout << "    "
             << "std::vector<" << pit->GetCPPType() << "> "
             <<  pit->GetName() << "Allowed;"
             << EOL << std::endl;
        for (unsigned int e = 0; e < (pit->GetElements()).size(); e++)
          {
          sout << "    "
               << pit->GetName() << "Allowed.push_back(";
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          std::string element = pit->GetElements()[e];
          replaceSubWithSub(element, "\"", "\\\"");

          sout << element;
          if (NeedsQuotes(*pit))
            {
            sout << "\"";
            }
          sout << "); "
               << EOL << std::endl;
          }
          sout << "    "
            "TCLAP::ValuesConstraint<" << pit->GetCPPType() << "> "
               << pit->GetName() << "AllowedVals ("
               << pit->GetName() << "Allowed); "
               << EOL << std::endl;
        }
      }
    }
  sout << "try" << EOL << std::endl;
  sout << "  {" << EOL << std::endl;
  sout << "    std::string fullDescription(\"Description: \");" << EOL << std::endl;
  sout << "    fullDescription += \"" << module.GetDescription() << "\";" << EOL << std::endl;
  sout << "    if (\"" << module.GetContributor() << "\" != \"\")" << EOL << std::endl;
  sout << "      {" << EOL << std::endl;
  sout << "      fullDescription += \"\\nAuthor(s): " << module.GetContributor() << "\";" << EOL << std::endl;
  sout << "      }" << EOL << std::endl;
  sout << "    if (\"" << module.GetAcknowledgements() << "\" != \"\")" << EOL << std::endl;
  sout << "      {" << EOL << std::endl;
  sout << "      fullDescription += \"\\nAcknowledgements: " << module.GetAcknowledgements() << "\";" << EOL << std::endl;
  sout << "      }" << EOL << std::endl;
  sout << "    TCLAP::CmdLine commandLine (fullDescription," << EOL << std::endl;
  sout << "       " << "' '," << EOL << std::endl;
  sout << "      " << "\"" << module.GetVersion() << "\"";
  sout << " );" << EOL << std::endl << EOL << std::endl;
  sout << "      itksys_ios::ostringstream msg;" << EOL << std::endl;

  // Second pass generates TCLAP declarations
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    for (pit = git->GetParameters().begin();
         pit != git->GetParameters().end();
         ++pit)
      {
      sout << "    msg.str(\"\");";
      sout << "msg << "
           << "\""
           << pit->GetDescription();
      if (pit->GetDefault().empty())
        {
        sout << "\";";
        }
      else
        {
        sout << " (default: \" "
             << "<< ";
        if (HasDefault(*pit))
          {
          sout << pit->GetName();
          }
        else
          {
          sout << "\"None\"";
          }

        if (NeedsTemp(*pit) && HasDefault(*pit))
          {
          sout << "Temp";
          }
        sout << " << "
             << "\")"
             << "\";"
             << EOL << std::endl;
        }

      if (pit->GetCPPType() == "bool")
        {
        sout << "    TCLAP::SwitchArg "
             << pit->GetName()
             << "Arg" << "(\""
             << pit->GetFlag()
             << "\", \"" 
             << pit->GetLongFlag()
             << "\", msg.str(), "
             << "commandLine"
             << ", "
             << pit->GetName()
             << ");"
             << EOL << std::endl << EOL << std::endl;
        }
      else
        {
        if (pit->GetFlag().empty() && pit->GetLongFlag().empty())
          {
          if (pit->GetMultiple() == "true")
            {
            sout << "    TCLAP::UnlabeledMultiArg<";
            sout << pit->GetArgType();            }
          else
            {
            sout << "    TCLAP::UnlabeledValueArg<";
            sout << pit->GetCPPType();
            }
          sout << "> "
               << pit->GetName()
               << "Arg" << "(\""
               << pit->GetName()
               << "\", msg.str(), ";
          if (pit->GetMultiple() != "true")
            {
            sout << true
                 << ", "
                 << pit->GetName();
            }
          else
            {
            sout << false;
            }
          sout << ", "
               << "\""
               << pit->GetCPPType()
               << "\""
               << ", "
               << "commandLine);"
               << EOL << std::endl << EOL << std::endl;
          }
        else if (IsEnumeration(*pit))
          {
          sout << "    TCLAP::ValueArg<";
          sout << pit->GetCPPType();
          sout << "> "
               << pit->GetName()
               << "Arg" << "(\""
               << pit->GetFlag()
               << "\", \"" 
               << pit->GetLongFlag()
               << "\", msg.str(), "
               << false
               << ", "
               << pit->GetName();
          sout << ", "
               << "&" << pit->GetName() << "AllowedVals"
               << ", "
               << "commandLine);"
               << EOL << std::endl << EOL << std::endl;
          }
        else
          {
          if (pit->GetMultiple() == "true")
            {
            sout << "    TCLAP::MultiArg<";
            if (NeedsTemp(*pit))
              {
              sout << "std::string";
              }
            else
              {
              sout << pit->GetCPPType();
              }
            }
          else
            {
            sout << "    TCLAP::ValueArg<";
            if (NeedsTemp(*pit))
              {
              sout << "std::string";
              }
            else
              {
              sout << pit->GetCPPType();
              }
            }
          sout << " > "
               << pit->GetName()
               << "Arg" << "(\""
               << pit->GetFlag()
               << "\", \"" 
               << pit->GetLongFlag()
               << "\", msg.str(), "
               << false;
          if (pit->GetMultiple() != "true")
            {
            sout << ", "
                 << pit->GetName();
            if (NeedsTemp(*pit))
              {
              sout << "Temp";
              }
            }
          sout << ", "
               << "\""
               << pit->GetCPPType()
               << "\""
               << ", "
               << "commandLine);"
               << EOL << std::endl << EOL << std::endl;
          }
        }
      }
    }
  sout << "    commandLine.parse ( argc, (char**) argv );" << EOL << std::endl;
  
  // Third pass generates access to arguments
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    for (pit = git->GetParameters().begin();
         pit != git->GetParameters().end();
         ++pit)
      {
      sout << "    "
           << pit->GetName();
      if (NeedsTemp(*pit))
        {
        sout << "Temp";
        }
      sout << " = "
           << pit->GetName()
           << "Arg.getValue();"
           << EOL << std::endl;
      }
    }

// Finally, for any arrays, split the strings into words
  for (git = module.GetParameterGroups().begin();
       git != module.GetParameterGroups().end();
       ++git)
    {
    for (pit = git->GetParameters().begin();
         pit != git->GetParameters().end();
         ++pit)
      {
      if (NeedsTemp(*pit) && pit->GetMultiple() != "true")
        {
        sout << "      { " << "/* Assignment for " << pit->GetName() << " */" << EOL << std::endl;
        sout << "      std::vector<std::string> words;"
             << EOL << std::endl;
        sout << "      std::string sep(\",\");"
             << EOL << std::endl;
        sout << "      splitString(" 
             << pit->GetName()
             << "Temp"
             << ", "
             << "sep, "
             << "words);"
             << EOL << std::endl;
        sout << "      for (unsigned int _j = 0; _j < words.size(); _j++)"
             << EOL << std::endl;
        sout << "        {"
             << EOL << std::endl;
        sout << "        " 
             << pit->GetName() << ".push_back("
             << pit->GetStringToType()
             << "(words[_j].c_str()));"
             << EOL << std::endl;
        sout << "        }"
             << EOL << std::endl;
        sout << "      }"
             << EOL << std::endl;
        }
      else if (NeedsTemp(*pit) && pit->GetMultiple() == "true")
        {
        sout << "      { " << "/* Assignment for " << pit->GetName() << " */" << EOL << std::endl;
        sout << "      for (unsigned int _i = 0; _i < ";
        sout << pit->GetName() << "Temp.size(); _i++)" << EOL << std::endl;
        sout << "        {" << EOL << std::endl;
        sout << "        std::vector<std::string> words;" << EOL << std::endl;
        sout << "        std::vector<" << pit->GetArgType() << "> elements;" << EOL << std::endl;
        sout << "        words.clear();" << EOL << std::endl;
        sout << "      std::string sep(\",\");" << EOL << std::endl;
        sout << "        splitString(" << pit->GetName() << "Temp[_i], sep, words);" << EOL << std::endl;
        if (IsVectorOfVectors(*pit))
          {
          sout << "        for (unsigned int _j= 0; _j < words.size(); _j++)" << EOL << std::endl;
          sout << "          {" << EOL << std::endl;
          sout << "          elements.push_back("
               << pit->GetStringToType()
               << "(words[_j].c_str()));"
               << EOL << std::endl;
          sout << "          }" << EOL << std::endl;
          sout << "        " << pit->GetName() << ".push_back(elements);"
               << EOL << std::endl;
          }
        else
          {
          sout << "        for (unsigned int _j= 0; _j < words.size(); _j++)" << EOL << std::endl;
          sout << "          {" << EOL << std::endl;
          sout << "            " << pit->GetName() << ".push_back("
               << pit->GetStringToType()
               << "(words[_j].c_str()));"
               << EOL << std::endl;
          sout << "          }" << EOL << std::endl;
          }
        sout << "        }" << EOL << std::endl;
        sout << "      }"
             << EOL << std::endl;
        }
      }
    }
  // Wrapup the block and generate the catch block
  sout << "      }" << EOL << std::endl;
  sout << "catch ( TCLAP::ArgException e )" << EOL << std::endl;
  sout << "  {" << EOL << std::endl;
  sout << "  std::cerr << \"error: \" << e.error() << \" for arg \" << e.argId() << std::endl;" << EOL << std::endl;
  sout << "  return ( EXIT_FAILURE );" << EOL << std::endl;
  sout << "    }" << std::endl;
}

void GeneratePost(std::ofstream &sout, ModuleDescription &module)
{
  sout << "#define PARSE_ARGS GENERATE_LOGO;GENERATE_XML;GENERATE_TCLAP;GENERATE_ECHOARGS;GENERATE_ProcessInformationAddressDecoding;" << std::endl;
}

void GenerateProcessInformationAddressDecoding(std::ofstream &sout, ModuleDescription &module)
{
  std::string EOL(" \\");
  sout << "#define GENERATE_ProcessInformationAddressDecoding \\" << std::endl;

  sout << "ModuleProcessInformation *CLPProcessInformation = 0;" << EOL << std::endl;
  sout << "if (processInformationAddressString != \"\")" << EOL << std::endl;
  sout << "{" << EOL << std::endl;
  sout << "sscanf(processInformationAddressString.c_str(), \"%p\", &CLPProcessInformation);" << EOL << std::endl;
  sout << "}" << std::endl;
}
