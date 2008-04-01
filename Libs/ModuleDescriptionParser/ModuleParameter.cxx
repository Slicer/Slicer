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
  int n = text.length();
  int start, stop;
  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
    {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) stop = n;
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
  this->LongFlag = parameter.LongFlag;
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
  this->LongFlag = parameter.LongFlag;
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

std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter)
{ 
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
  os << "      " << "LongFlag: " << parameter.GetLongFlag() << std::endl;
  os << "      " << "Channel: " << parameter.GetChannel() << std::endl;
  os << "      " << "Index: " << parameter.GetIndex() << std::endl;
  os << "      " << "Multiple: " << parameter.GetMultiple() << std::endl;
  os << "      " << "Aggregate: " << parameter.GetAggregate() << std::endl;
  os << "      " << "FileExtensionsAsString: " << parameter.GetFileExtensionsAsString() << std::endl;
  os << "      " << "FileExtensions: ";
  std::vector<std::string>::const_iterator fit;  
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

