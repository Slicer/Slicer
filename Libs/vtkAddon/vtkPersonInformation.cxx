/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkPersonInformation.h"

#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>

vtkStandardNewMacro(vtkPersonInformation);

#define vtkGetSetStringInformationCxxMacro(key) \
  bool vtkPersonInformation::Set##key(const std::string& value) \
  { \
    this->SetCustomString(#key, value); \
    return true; \
  } \
  std::string vtkPersonInformation::Get##key() \
  { \
    return this->GetCustomString(#key); \
  }

#define vtkGetSetValidatedStringInformationCxxMacro(key) \
  bool vtkPersonInformation::Set##key(const std::string& value) \
  { \
    if (!this->Is##key##Valid(value)) \
    { \
      return false; \
    } \
    this->SetCustomString(#key, value); \
    return true; \
  } \
  std::string vtkPersonInformation::Get##key() \
  { \
    return this->GetCustomString(#key); \
  }

vtkGetSetStringInformationCxxMacro(Name);
vtkGetSetValidatedStringInformationCxxMacro(Email);
vtkGetSetStringInformationCxxMacro(Login);
vtkGetSetStringInformationCxxMacro(Organization);
vtkGetSetStringInformationCxxMacro(OrganizationRole);
vtkGetSetStringInformationCxxMacro(ProcedureRole);

//----------------------------------------------------------------------------
vtkPersonInformation::vtkPersonInformation()
= default;

//----------------------------------------------------------------------------
vtkPersonInformation::~vtkPersonInformation()
= default;

//----------------------------------------------------------------------------
void vtkPersonInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  for (std::map<std::string, std::string>::iterator it = this->Data.begin();
    it != this->Data.end(); ++it)
    {
    os << indent << it->first << ": " << it->second << "\n";
  }
}

//----------------------------------------------------------------------------
void vtkPersonInformation::DeepCopy(vtkPersonInformation* source)
{
  if (!source)
    {
    return;
    }
  this->Data = source->Data;
}

//----------------------------------------------------------------------------
void vtkPersonInformation::SetCustomString(const std::string& key, const std::string& value)
{
  if (key.empty())
    {
    vtkErrorMacro("vtkPersonInformation::SetCustomString failed: key must not be empty");
    return;
    }
  if (value.empty())
    {
    std::map<std::string, std::string>::iterator it = this->Data.find(key);
    if (it == this->Data.end())
      {
      // no change
      return;
      }
    this->Data.erase(it);
    this->Modified();
    return;
    }
  std::string currentValue = this->Data[key];
  if (value.compare(currentValue) == 0)
    {
    // no change
    return;
    }
  this->Data[key] = value;
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkPersonInformation::GetCustomString(const std::string& key)
{
  std::map<std::string, std::string>::iterator it = this->Data.find(key);
  if (it == this->Data.end())
    {
    return "";
    }
  return it->second;
}

//----------------------------------------------------------------------------
std::string vtkPersonInformation::GetAsString()
{
  std::string output;
  for (std::map<std::string, std::string>::iterator it = this->Data.begin();
    it != this->Data.end(); ++it)
    {
    if (!output.empty())
      {
      output += ";";
      }
    output += this->EncodeString(it->first) + ":" + this->EncodeString(it->second);
    }
  return output;
}

//----------------------------------------------------------------------------
bool vtkPersonInformation::SetFromString(const std::string& data)
{
  this->Data.clear();
  std::stringstream keyValuePairs(data);
  std::string keyValuePair;
  while (std::getline(keyValuePairs, keyValuePair, ';'))
    {
    int colonIndex = keyValuePair.find(':');
    std::string key = this->DecodeString(keyValuePair.substr(0, colonIndex));
    std::string value = this->DecodeString(keyValuePair.substr(colonIndex + 1));
    this->SetCustomString(key, value);
    }
  return true;
}

//----------------------------------------------------------------------------
std::string vtkPersonInformation::EncodeString(const std::string& value)
{
  std::string output = value;
  // encode percent sign and semicolon (semicolon is a special character because it separates attributes)
  vtksys::SystemTools::ReplaceString(output, "%", "%25");
  vtksys::SystemTools::ReplaceString(output, ":", "%3A");
  vtksys::SystemTools::ReplaceString(output, ";", "%3B");
  return output;
}

//----------------------------------------------------------------------------
std::string vtkPersonInformation::DecodeString(const std::string& value)
{
  std::string output = value;
  // decode percent sign and semicolon (semicolon is a special character because it separates attributes)
  vtksys::SystemTools::ReplaceString(output, "%3B", ";");
  vtksys::SystemTools::ReplaceString(output, "%3A", ":");
  vtksys::SystemTools::ReplaceString(output, "%25", "%");
  return output;
}

//----------------------------------------------------------------------------
bool vtkPersonInformation::IsEmailValid(const std::string& value)
{
  // email is valid if it contains @ character or empty
  if (value.empty())
    {
    return true;
    }
  return (value.find("@") != std::string::npos);
}

//----------------------------------------------------------------------------
void vtkPersonInformation::GetKeys(vtkStringArray *keys)
{
  if (!keys)
    {
    vtkErrorMacro("vtkPersonInformation::GetKeys failed: keys is invalid");
    return;
    }
  keys->Reset();
  for (std::map<std::string, std::string>::iterator it = this->Data.begin();
    it != this->Data.end(); ++it)
    {
    keys->InsertNextValue(it->first);
    }
}
