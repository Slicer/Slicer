#include "ModuleDescriptionParser.h"

#include "ModuleDescription.h"

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "expat.h"

/*********************
 * Utility procedures to trim leading and trailing characters
 *********************/
void
trimLeading(std::string& s, const char* extraneousChars = " \t\n")
{
  if (s.size())
    {
    s = s.substr(s.find_first_not_of(extraneousChars));
    }
}

void
trimTrailing(std::string& s, const char* extraneousChars = " \t\n")
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
  XML_Parser Parser;                           /* The XML parser */
  std::vector<std::string> LastData;           /* The last tag
                                                * processed by expat
                                                * at each depth */
  ModuleDescription CurrentDescription;
  ModuleParameterGroup *CurrentGroup;          /* The parameter group */
  ModuleParameter *CurrentParameter;           /* The current parameter */
  std::stack<std::string> OpenTags;            /* The current tag */
  bool Debug;                                  /* Debug flag */
  bool Error;                                  /* Error detected */
  std::string ErrorDescription;                /* Error description */
  int ErrorLine;                               /* Error line number */
  int Depth;                                   /* The depth of the tag */

  ParserState():Debug(false),Error(false),Depth(-1),LastData(10){};
};

/***************************
 * expat callbacks to process the XML
 ***************************/
void
startElement(void *userData, const char *element, const char **attrs)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  ModuleParameter *parameter = ps->CurrentParameter;
  ModuleParameterGroup *group = ps->CurrentGroup;
  std::string name(element);

  ps->Depth++;
  // Check for a valid module description file
  //  
  if (ps->Depth == 0 && (name != "executable") )
    {
    std::string error("ModuleDescriptionParser Error: <executable> must be the outer most tag. " + name + std::string(" was found instead."));
    ps->ErrorDescription = error;
    ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
    ps->Error = true;
    return;
    }
  else if (ps->Depth != 0 && (name == "executable"))
    {
    std::string error("ModuleDescriptionParser Error: <executable> was found inside another tag <" + ps->OpenTags.top() + ">.");
    ps->ErrorDescription = error;
    ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
    ps->Error = true;
    return;
    }

  // Clear the last tag for this depth
  ps->LastData[ps->Depth].clear();

  if (name == "parameters")
    {
    if (ps->Depth != 1)
      {
      std::string error("ModuleDescriptionParser Error: <parameters> can only be inside <executable> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      return;
      }
    else
      {
      group = new ModuleParameterGroup;
      int attrCount = XML_GetSpecifiedAttributeCount(ps->Parser);
      if (attrCount == 2 && (strcmp(attrs[0], "advanced") == 0))
        {
        group->SetAdvanced(attrs[1]);
        }
      }
    }
  else if (name == "integer")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + std::string(name) + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("int");
    }
  else if (name == "float")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + std::string(name) + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("float");
    }
  else if (name == "double")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + std::string(name) + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("double");
    }
  else if (name == "string")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::string");
    }
  else if (name == "boolean")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("bool");
    }
  else if (name == "integer-vector")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::vector<int>");
    parameter->SetStringToType("atoi");
    }
  else if (name == "float-vector")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::vector<float>");
    parameter->SetStringToType("atof");
    }
  else if (name == "double-vector")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::vector<double>");
    parameter->SetStringToType("atof");
    }
  else if (name == "string-enumeration")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::string");
    }
  else if (name == "integer-enumeration")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("int");
    }
  else if (name == "float-enumeration")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("float");
    }
  else if (name == "double-enumeration")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("double");
    }
  else if (name == "file")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::string");
    }
  else if (name == "directory")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::string");
    }
  else if (name == "image")
    {
    if (!group || (ps->OpenTags.top() != "parameters"))
      {
      std::string error("ModuleDescriptionParser Error: <" + name + "> can only be used inside <parameters> but was found inside <" + ps->OpenTags.top() + ">");
      ps->ErrorDescription = error;
      ps->ErrorLine = XML_GetCurrentLineNumber(ps->Parser);
      ps->Error = true;
      ps->OpenTags.push(name);
      return;
      }
    parameter = new ModuleParameter;
    parameter->SetTag(name);
    parameter->SetType("std::string");
    }
  ps->CurrentParameter = parameter;
  ps->CurrentGroup = group;
  ps->OpenTags.push(name);
}

void
endElement(void *userData, const char *element)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  ModuleParameter *parameter = ps->CurrentParameter;
  ModuleParameterGroup *group = ps->CurrentGroup;
  std::string name(element);

  if (name == "parameters" && ps->Depth == 1)
    {
    ps->CurrentDescription.AddParameterGroup(*ps->CurrentGroup);
    ps->CurrentGroup = 0;
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "integer"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "float"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "double"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "string"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name =="boolean"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "file"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "directory"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "image"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "integer-vector"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "float-vector"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "double-vector"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "string-enumeration"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "integer-enumeration"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "float-enumeration"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (group && parameter && (name == "double-enumeration"))
    {
    ps->CurrentGroup->AddParameter(*parameter);
    ps->CurrentParameter = 0;
    }
  else if (parameter && (name == "flag"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetShortFlag(temp);
    }
  else if (parameter && (name == "longflag"))
    {
    std::string temp = ps->LastData[ps->Depth];
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
  else if (parameter && (name == "name"))
    {
    std::string temp = std::string(ps->LastData[ps->Depth]);
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetName(temp);
    }
  else if ((group || parameter) && (name == "label"))
    {
    std::string temp = ps->LastData[ps->Depth];
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
  else if (name == "category")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetCategory(temp);
    }
  else if (name == "title")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetTitle(temp);
    }
  else if (name == "version")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetVersion(temp);
    }
  else if (name == "documentationurl")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetDocumentationURL(temp);
    }
  else if (name == "license")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetLicense(temp);
    }
  else if (name == "contributor")
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    ps->CurrentDescription.SetContributor(temp);
    }
  else if (name ==  "description")
    {
    std::string temp = ps->LastData[ps->Depth];
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
  else if (parameter && (name == "element"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->GetElements().push_back(temp);
    }
  else if (parameter && (name == "default"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetDefault(temp);
    }
  else if (parameter && (name == "channel"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetChannel(temp);
    }
  else if (parameter && (name == "index"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetIndex(temp);
    }
  else if (parameter && (name == "constraints"))
    {
    parameter->SetConstraints(name);
    }
  else if (parameter && (name == "minimum"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetMinimum(temp);
    }
  else if (parameter && (name == "maximum"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetMaximum(temp);
    }
  else if (parameter && (name == "step"))
    {
    std::string temp = ps->LastData[ps->Depth];
    trimLeading(temp);
    trimTrailing(temp);
    parameter->SetStep(temp);
    }
  ps->OpenTags.pop();
  ps->Depth--;
}

void
charData(void *userData, const char *s, int len)
{
  ParserState *ps = reinterpret_cast<ParserState *>(userData);
  if (len)
    {
    std::string str(s,len);
    ps->LastData[ps->Depth] += str;
    }
}

int
ModuleDescriptionParser::Parse( const std::string& xml, ModuleDescription& description)
{
// Check the first line if the XML
  if (strncmp(xml.c_str(),"<?xml ", 6) != 0)
    {
    std::string required("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    std::cerr << "ModuleDesriptionParser: first line must be " << std::endl;
    std::cerr << required << std::endl;
    return 1;
    }

  ParserState parserState;
  parserState.CurrentDescription = description;
  
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;

  parserState.Parser = parser;
  parserState.CurrentParameter = 0;
  parserState.CurrentGroup = 0;

  XML_SetUserData(parser, static_cast<void *>(&parserState));
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charData);

  // Get the length of the file
  done = true;
  int status = 0;
  if (XML_Parse(parser, xml.c_str(), xml.size(), done) == 0)
    {
    std::cerr << XML_ErrorString(XML_GetErrorCode(parser))
              << " at line "
              << XML_GetCurrentLineNumber(parser)
              << std::endl;
    status = 1;
    }
  else
    {
    if (parserState.Error)
      {
      std::cerr << parserState.ErrorDescription
                << " at line "
                <<  parserState.ErrorLine
                << std::endl;
      status = 1;
      }
    }

  XML_ParserFree(parser);

  description = parserState.CurrentDescription;
  return status;

}
