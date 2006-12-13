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

ModuleParameter::ModuleParameter(const ModuleParameter& parameter)
{
  this->Tag = parameter.Tag;
  this->Name = parameter.Name;
  this->Description = parameter.Description;
  this->Label = parameter.Label;
  this->CPPType = parameter.CPPType;
  this->Type = parameter.Type;
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
  this->Elements = parameter.Elements;
  this->CoordinateSystem = parameter.CoordinateSystem;
}

std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter)
{ 
  os << "    Parameter" << std::endl;
  os << "      " << "Tag: " << parameter.GetTag() << std::endl;
  os << "      " << "Name: " << parameter.GetName() << std::endl;
  os << "      " << "Description: " << parameter.GetDescription() << std::endl;
  os << "      " << "Label: " << parameter.GetLabel() << std::endl;
  os << "      " << "Type: " << parameter.GetType() << std::endl;
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
  os << "      " << "CoordinateSystem: " << parameter.GetCoordinateSystem() << std::endl;
  return os;
}

