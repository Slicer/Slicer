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

/* ParserState: A class to keep state information for the parser. This
 * is passed to the expat code as user data.
 */
class ParserState
{
public:
  std::string LastTag;                         /* The last tag processed by expat */
  ModuleDescription CurrentDescription;
  ModuleParameterGroup *CurrentGroup;          /* The parameter group */
  ModuleParameter *CurrentParameter;           /* The current parameter */
  bool Debug;                                  /* Debug flag */
  bool Error;                                  /* Error detected */
  ParserState():Debug(false),Error(false){};
};

/***************************
 * expat callbacks to process the XML
 ***************************/
void
startElement(void *userData, const char *name, const char **)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  ModuleParameter *parameter = ps->CurrentParameter;
  ModuleParameterGroup *group = ps->CurrentGroup;

  ps->LastTag.clear();

  if (strcmp(name, "parameters") == 0)
    {
    group = new ModuleParameterGroup;
    }
  else if (strcmp(name, "integer") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("int");
    }
  else if (strcmp(name, "float") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("float");
    }
  else if (strcmp(name, "double") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("double");
    }
  else if (strcmp(name, "string") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::string");
    }
  else if (strcmp(name, "boolean") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("bool");
    }
  else if (strcmp(name, "integer-vector") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::vector<int>");
    parameter->SetStringToType("atoi");
    }
  else if (strcmp(name, "float-vector") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::vector<float>");
    parameter->SetStringToType("atof");
    }
  else if (strcmp(name, "double-vector") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::vector<double>");
    parameter->SetStringToType("atof");
    }
  else if (strcmp(name, "file") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::string");
    }
  else if (strcmp(name, "directory") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::string");
    }
  else if (strcmp(name, "image") == 0)
    {
    parameter = new ModuleParameter;
    parameter->SetType("std::string");
    }
  ps->CurrentParameter = parameter;
  ps->CurrentGroup = group;
}

void
endElement(void *userData, const char *name)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  ModuleParameter *parameter = ps->CurrentParameter;
  ModuleParameterGroup *group = ps->CurrentGroup;

  if (strcmp(name, "parameters") == 0)
    {
    ps->CurrentDescription.AddParameterGroup(*ps->CurrentGroup);
    ps->CurrentGroup = 0;
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "integer") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "float") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "double") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "string") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "boolean") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "file") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "directory") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "image") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "integer-vector") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "float-vector") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "double-vector") == 0)
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (strcmp(name, "flag") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetShortFlag(temp);
    }
  else if (strcmp(name, "longflag") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    if (temp.find("-",2) != std::string::npos)
      {
      std::cerr << "GenerateCLP: flags cannot contain \"-\" : " << temp << std::endl;
      ps->Error = true;
      }
    parameter->SetLongFlag(temp);
    if (parameter->GetName().empty())
      {
      parameter->SetName(temp);
      }
    }
  else if (strcmp(name, "name") == 0)
    {
    std::string temp = std::string(ps->LastTag);
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetName(temp);
    }
  else if (strcmp(name, "label") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    if (group && !parameter)
      {
      group->SetLabel(temp);
      }
    else
      {
      parameter->SetLabel(temp);
      }
    }
  else if (strcmp(name, "category") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetCategory(temp);
    }
  else if (strcmp(name, "title") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetTitle(temp);
    }
  else if (strcmp(name, "version") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetVersion(temp);
    }
  else if (strcmp(name, "documentationurl") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetDocumentationURL(temp);
    }
  else if (strcmp(name, "license") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetLicense(temp);
    }
  else if (strcmp(name, "contributor") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetContributor(temp);
    }
  else if (strcmp(name, "description") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    if (!group && !parameter)
      {
      ps->CurrentDescription.SetDescription(temp);
      }
    else if (group && !parameter)
      {
      group->SetDescription(temp);
      }
    else
      {
      parameter->SetDescription(temp);
      }
    }
  else if (strcmp(name, "default") == 0)
    {
    std::string temp = ps->LastTag;
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetDefault(temp);
    }
}

void
charData(void *userData, const char *s, int len)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  if (len)
    {
    std::string str(s,len);
    ps->LastTag += str;
    }
}

int
ModuleDescriptionParser::Parse( const std::string& xml, ModuleDescription& description)
{
  ParserState parserState;
  parserState.CurrentDescription = description;
  
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;

  parserState.CurrentParameter = 0;
  parserState.CurrentGroup = 0;

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

  description = parserState.CurrentDescription;
  return 0;

}
