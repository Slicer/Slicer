/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#include "ModuleParameter.h"

void
splitString (const std::string &text,
             const std::string &separators,
             std::vector<std::string> &words)
{
  const std::string::size_type n = text.length();
  std::string::size_type start = text.find_first_not_of(separators);
  while (start < n)
    {
    std::string::size_type stop = text.find_first_of(separators, start);
    if (stop > n) stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop+1);
    }
}

ModuleParameter::ModuleParameter()
{
    this->Tag = "";
    this->Name = "";
    this->Description = "";
    this->Label = "";
    this->CPPType = "";
    this->Type = "";
    this->Reference = "";
    this->Hidden = "false";
    this->ArgType = "";
    this->StringToType = "";
    this->Default = "";
    this->Flag = "";
    this->LongFlag = "";
    this->Constraints = "";
    this->Minimum = "";
    this->Maximum = "";
    this->Step = "";
    this->Channel = "";
    this->Index = "";
    this->Multiple = "false";
    this->Aggregate = "false";
    this->FileExtensionsAsString = "";        
    this->CoordinateSystem = "";
    this->FlagAliasesAsString = "";
    this->DeprecatedFlagAliasesAsString = "";
    this->LongFlagAliasesAsString = "";
    this->DeprecatedLongFlagAliasesAsString = "";
}

ModuleParameter::ModuleParameter(const ModuleParameter& parameter)
{
  this->Tag = parameter.Tag;
  this->Name = parameter.Name;
  this->Description = parameter.Description;
  this->Label = parameter.Label;
  this->CPPType = parameter.CPPType;
  this->Type = parameter.Type;
  this->Reference = parameter.Reference;
  this->Hidden = parameter.Hidden;
  this->ArgType = parameter.ArgType;
  this->StringToType = parameter.StringToType;
  this->Default = parameter.Default;
  this->Flag = parameter.Flag;
  this->FlagAliasesAsString = parameter.FlagAliasesAsString;
  this->FlagAliases = parameter.FlagAliases;
  this->DeprecatedFlagAliasesAsString
    = parameter.DeprecatedFlagAliasesAsString;
  this->DeprecatedFlagAliases = parameter.DeprecatedFlagAliases;
  this->LongFlagAliasesAsString = parameter.LongFlagAliasesAsString;
  this->LongFlag = parameter.LongFlag;
  this->LongFlagAliases = parameter.LongFlagAliases;
  this->DeprecatedLongFlagAliasesAsString
    = parameter.DeprecatedLongFlagAliasesAsString;
  this->DeprecatedLongFlagAliases = parameter.DeprecatedLongFlagAliases;
  this->Constraints = parameter.Constraints;
  this->Minimum = parameter.Minimum;
  this->Maximum = parameter.Maximum;
  this->Step = parameter.Step;
  this->Channel = parameter.Channel;
  this->Index = parameter.Index;
  this->Multiple = parameter.Multiple;
  this->Aggregate = parameter.Aggregate;
  this->FileExtensionsAsString = parameter.FileExtensionsAsString;
  this->FileExtensions = parameter.FileExtensions;
  this->Elements = parameter.Elements;
  this->CoordinateSystem = parameter.CoordinateSystem;
}

void ModuleParameter::operator=(const ModuleParameter& parameter)
{
  this->Tag = parameter.Tag;
  this->Name = parameter.Name;
  this->Description = parameter.Description;
  this->Label = parameter.Label;
  this->CPPType = parameter.CPPType;
  this->Type = parameter.Type;
  this->Reference = parameter.Reference;
  this->Hidden = parameter.Hidden;
  this->ArgType = parameter.ArgType;
  this->StringToType = parameter.StringToType;
  this->Default = parameter.Default;
  this->Flag = parameter.Flag;
  this->FlagAliasesAsString = parameter.FlagAliasesAsString;
  this->FlagAliases = parameter.FlagAliases;
  this->DeprecatedFlagAliasesAsString
    = parameter.DeprecatedFlagAliasesAsString;
  this->DeprecatedFlagAliases = parameter.DeprecatedFlagAliases;
  this->LongFlag = parameter.LongFlag;
  this->LongFlagAliasesAsString = parameter.LongFlagAliasesAsString;
  this->LongFlagAliases = parameter.LongFlagAliases;
  this->DeprecatedLongFlagAliasesAsString = parameter.DeprecatedLongFlagAliasesAsString;
  this->DeprecatedLongFlagAliases = parameter.DeprecatedLongFlagAliases;
  this->Constraints = parameter.Constraints;
  this->Minimum = parameter.Minimum;
  this->Maximum = parameter.Maximum;
  this->Step = parameter.Step;
  this->Channel = parameter.Channel;
  this->Index = parameter.Index;
  this->Multiple = parameter.Multiple;
  this->Aggregate = parameter.Aggregate;
  this->FileExtensionsAsString = parameter.FileExtensionsAsString;
  this->FileExtensions = parameter.FileExtensions;
  this->Elements = parameter.Elements;
  this->CoordinateSystem = parameter.CoordinateSystem;
}

const std::vector<std::string> &
ModuleParameter::GetFileExtensions() const
{
  return this->FileExtensions;
}

void
ModuleParameter::SetFileExtensionsAsString(const std::string& ext)
{
  this->FileExtensionsAsString = ext;

  this->FileExtensions.clear();
  splitString(this->FileExtensionsAsString, std::string(","), this->FileExtensions);
}

void
ModuleParameter::SetFlagAliasesAsString(const std::string& aliases)
{
  this->FlagAliasesAsString = aliases;

  this->FlagAliases.clear();
  splitString(this->FlagAliasesAsString, std::string(","), this->FlagAliases);
}

void
ModuleParameter::SetDeprecatedFlagAliasesAsString(const std::string& aliases)
{
  this->DeprecatedFlagAliasesAsString = aliases;

  this->DeprecatedFlagAliases.clear();
  splitString(this->DeprecatedFlagAliasesAsString, std::string(","), this->DeprecatedFlagAliases);
}

void
ModuleParameter::SetLongFlagAliasesAsString(const std::string& aliases)
{
  this->LongFlagAliasesAsString = aliases;

  this->LongFlagAliases.clear();
  splitString(this->LongFlagAliasesAsString, std::string(","), this->LongFlagAliases);
}

void
ModuleParameter::SetDeprecatedLongFlagAliasesAsString(const std::string& aliases)
{
  this->DeprecatedLongFlagAliasesAsString = aliases;

  this->DeprecatedLongFlagAliases.clear();
  splitString(this->DeprecatedLongFlagAliasesAsString, std::string(","), this->DeprecatedLongFlagAliases);
}

std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter)
{ 
  std::vector<std::string>::const_iterator fit;  

  os << "    Parameter" << std::endl;
  os << "      " << "Tag: " << parameter.GetTag() << std::endl;
  os << "      " << "Name: " << parameter.GetName() << std::endl;
  os << "      " << "Description: " << parameter.GetDescription() << std::endl;
  os << "      " << "Label: " << parameter.GetLabel() << std::endl;
  os << "      " << "Type: " << parameter.GetType() << std::endl;
  os << "      " << "Reference: " << parameter.GetReference() << std::endl;
  os << "      " << "Hidden: " << parameter.GetHidden() << std::endl;
  os << "      " << "CPPType: " << parameter.GetCPPType() << std::endl;
  os << "      " << "ArgType: " << parameter.GetArgType() << std::endl;
  os << "      " << "StringToType: " << parameter.GetStringToType() << std::endl;
  os << "      " << "Default: " << parameter.GetDefault() << std::endl;
  os << "      " << "Elements: ";
  std::vector<std::string>::const_iterator eit;  
  for (eit = parameter.GetElements().begin();
       eit != parameter.GetElements().end(); ++eit)
    {
    if (eit != parameter.GetElements().begin())
      {
      os << ", ";
      }
    os << *eit;
    }
  os << std::endl;
  os << "      " << "Constraints: " << parameter.GetConstraints() << std::endl;
  os << "      " << "Minimum: " << parameter.GetMinimum() << std::endl;
  os << "      " << "Maximum: " << parameter.GetMaximum() << std::endl;
  os << "      " << "Step: " << parameter.GetStep() << std::endl;
  os << "      " << "Flag: " << parameter.GetFlag() << std::endl;
  os << "      " << "FlagAliasesAsString: " << parameter.GetFlagAliasesAsString() << std::endl;
  os << "      " << "Flag aliases: ";
  for (fit = parameter.GetFlagAliases().begin();
       fit != parameter.GetFlagAliases().end(); ++fit)
    {
    if (fit != parameter.GetFlagAliases().begin())
      {
      os << ", ";
      }
    os << *fit;
    }
  os << std::endl;
  os << "      " << "DeprecatedFlagAliasesAsString: " << parameter.GetDeprecatedFlagAliasesAsString() << std::endl;
  os << "      " << "Deprecated Flag aliases: ";
  for (fit = parameter.GetDeprecatedFlagAliases().begin();
       fit != parameter.GetDeprecatedFlagAliases().end(); ++fit)
    {
    if (fit != parameter.GetDeprecatedFlagAliases().begin())
      {
      os << ", ";
      }
    os << *fit;
    }
  os << std::endl;
  os << "      " << "LongFlag: " << parameter.GetLongFlag() << std::endl;
  os << "      " << "LongFlagAliasesAsString: " << parameter.GetLongFlagAliasesAsString() << std::endl;
  os << "      " << "LongFlag aliases: ";
  for (fit = parameter.GetLongFlagAliases().begin();
       fit != parameter.GetLongFlagAliases().end(); ++fit)
    {
    if (fit != parameter.GetLongFlagAliases().begin())
      {
      os << ", ";
      }
    os << *fit;
    }
  os << std::endl;
  os << "      " << "DeprecatedLongFlagAliasesAsString: " << parameter.GetDeprecatedLongFlagAliasesAsString() << std::endl;
  os << "      " << "Deprecated LongFlag aliases: ";
  for (fit = parameter.GetDeprecatedLongFlagAliases().begin();
       fit != parameter.GetDeprecatedLongFlagAliases().end(); ++fit)
    {
    if (fit != parameter.GetDeprecatedLongFlagAliases().begin())
      {
      os << ", ";
      }
    os << *fit;
    }
  os << std::endl;
  os << "      " << "Channel: " << parameter.GetChannel() << std::endl;
  os << "      " << "Index: " << parameter.GetIndex() << std::endl;
  os << "      " << "Multiple: " << parameter.GetMultiple() << std::endl;
  os << "      " << "Aggregate: " << parameter.GetAggregate() << std::endl;
  os << "      " << "FileExtensionsAsString: " << parameter.GetFileExtensionsAsString() << std::endl;
  os << "      " << "FileExtensions: ";
  for (fit = parameter.GetFileExtensions().begin();
       fit != parameter.GetFileExtensions().end(); ++fit)
    {
    if (fit != parameter.GetFileExtensions().begin())
      {
      os << ", ";
      }
    os << *fit;
    }
  os << std::endl;
  os << "      " << "CoordinateSystem: " << parameter.GetCoordinateSystem() << std::endl;
  return os;
}

