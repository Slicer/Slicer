/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#ifndef __ModuleParameter_h
#define __ModuleParameter_h

#include "ModuleDescriptionParserWin32Header.h"

#include <string>
#include <vector>
#include <iostream>

/** \class ModuleParameter
 *  \brief Class to describe a single parameter to a module.
 *
 * ModuleParameter describes a single parameters to a
 * module. Information on the parameter type, name, flag, label,
 * description, channel, index, default, and constraints can be
 * stored.
 *
 */
class ModuleDescriptionParser_EXPORT ModuleParameter
{
public:
  ModuleParameter();
  virtual ~ModuleParameter() {}
  ModuleParameter(const ModuleParameter& parameter);

  void operator=(const ModuleParameter& parameter);
  
  virtual void SetTag(const std::string &tag)
  {
    this->Tag = tag;
  }

  virtual const std::string& GetTag() const
  {
    return this->Tag;
  }
  
  virtual void SetCPPType(const std::string &type)
  {
    this->CPPType = type;
  }

  virtual const std::string& GetCPPType() const 
  {
    return this->CPPType;
  }
  
  virtual void SetType(const std::string &type)
  {
    this->Type = type;
  }

  virtual const std::string& GetType() const 
  {
    return this->Type;
  }

  virtual void SetReference(const std::string &ref)
  {
    this->Reference = ref;
  }

  virtual const std::string& GetReference() const
  {
    return this->Reference;
  }

  virtual void SetHidden(const std::string &hidden)
  {
    this->Hidden = hidden;
  }

  virtual const std::string& GetHidden() const
  {
    return this->Hidden;
  }

  // Simple return types are parameters on output channel with no
  // flags and without a specified index 
  virtual bool IsReturnParameter() const
  {
    // could check for tag == float, int, float-vector, ...
    if (this->Channel == "output" 
        && !this->IsFlagParameter() && !this->IsIndexParameter())
      {
      return true;
      }
    return false;
  }
  
  // Has a flag or a long flag?
  virtual bool IsFlagParameter() const
  {
    return (this->Flag != "" || this->LongFlag != "");
  }

  // Is an index type?
  virtual bool IsIndexParameter() const
  {
    return (this->Index != "");
  }

  virtual void SetArgType(const std::string &argType)
  {
    this->ArgType = argType;
  }

  virtual const std::string& GetArgType() const 
  {
    return this->ArgType;
  }
  
  virtual void SetStringToType(const std::string &stringToType)
  {
    this->StringToType = stringToType;
  }

  virtual const std::string &GetStringToType() const
  {
    return this->StringToType;
  }
  
  virtual void SetName(const std::string &name)
  {
    this->Name = name;
  }

  virtual const std::string& GetName() const
  {
    return this->Name;
  }
  
  virtual void SetLongFlag(const std::string &longFlag)
  {
    this->LongFlag = longFlag;
  }

  virtual const std::string& GetLongFlag() const
  {
    return this->LongFlag;
  }
  
  virtual void SetLongFlagAliasesAsString(const std::string &aliases);

  virtual const std::string& GetLongFlagAliasesAsString() const
  {
    return this->LongFlagAliasesAsString;
  }

  virtual const std::vector<std::string> &GetLongFlagAliases() const
  {
    return this->LongFlagAliases;
  }

  virtual void SetDeprecatedLongFlagAliasesAsString(const std::string &aliases);

  virtual const std::string& GetDeprecatedLongFlagAliasesAsString() const
  {
    return this->DeprecatedLongFlagAliasesAsString;
  }
  
  virtual const std::vector<std::string> &GetDeprecatedLongFlagAliases() const
  {
    return this->DeprecatedLongFlagAliases;
  }

  virtual void SetLabel(const std::string &label) 
  {
    this->Label = label;
  }
  
  virtual const std::string& GetLabel() const
  {
    return this->Label;
  }

  virtual void SetConstraints(const std::string &constraints)
  {
    this->Constraints = constraints;
  }
  
  virtual const std::string& GetConstraints() const
  {
    return this->Constraints;
  }

  virtual void SetMaximum(const std::string &maximum)
  {
    this->Maximum = maximum;
  }
  
  virtual const std::string& GetMaximum() const 
  {
    return this->Maximum;
  }

  virtual void SetMinimum(const std::string &minimum) 
  {
    this->Minimum = minimum;
  }
  
  virtual const std::string& GetMinimum() const 
  {
    return this->Minimum;
  }

  virtual void SetStep(const std::string &step) 
  {
    this->Step = step;
  }
  
  virtual const std::string& GetStep() const
  {
    return this->Step;
  }

  virtual void SetDescription(const std::string &description)
  {
    this->Description = description;
  }

  virtual const std::string& GetDescription() const
  {
    return this->Description;
  }
  
  virtual void SetChannel(const std::string &channel)
  {
    this->Channel = channel;
  }

  virtual const std::string& GetChannel() const
  {
    return this->Channel;
  }
  
  virtual void SetIndex(const std::string &index)
  {
    this->Index = index;
  }

  virtual const std::string& GetIndex() const
  {
    return this->Index;
  }
  
  virtual void SetDefault(const std::string &def)
  {
    this->Default = def;
  }

  virtual const std::string& GetDefault() const
  {
    return this->Default;
  }
  
  virtual void SetFlag(const std::string &flag)
  {
    this->Flag = flag;
  }

  virtual const std::string& GetFlag() const
  {
    return this->Flag;
  }

  virtual void SetFlagAliasesAsString(const std::string &aliases);

  virtual const std::string& GetFlagAliasesAsString() const
  {
    return this->FlagAliasesAsString;
  }
  
  virtual const std::vector<std::string> &GetFlagAliases() const
  {
    return this->FlagAliases;
  }

  virtual void SetDeprecatedFlagAliasesAsString(const std::string &aliases);

  virtual const std::string& GetDeprecatedFlagAliasesAsString() const
  {
    return this->DeprecatedFlagAliasesAsString;
  }

  virtual const std::vector<std::string> &GetDeprecatedFlagAliases() const
  {
    return this->DeprecatedFlagAliases;
  }
  
  virtual void SetMultiple(const std::string &multiple)
  {
    this->Multiple = multiple;
  }

  virtual const std::string& GetMultiple() const
  {
    return this->Multiple;
  }

  virtual void SetAggregate(const std::string &aggregate)
  {
    this->Aggregate = aggregate;
  }

  virtual const std::string& GetAggregate() const
  {
    return this->Aggregate;
  }
  
  virtual void SetFileExtensionsAsString(const std::string &extensions);

  virtual const std::string& GetFileExtensionsAsString() const
  {
    return this->FileExtensionsAsString;
  }

  virtual const std::vector<std::string> &GetFileExtensions() const;

  virtual void SetCoordinateSystem(const std::string &coordinateSystem)
  {
    this->CoordinateSystem = coordinateSystem;
  }

  virtual const std::string& GetCoordinateSystem() const
  {
    return this->CoordinateSystem;
  }

  virtual const std::vector<std::string> &GetElements() const
  {
    return this->Elements;
  }

  virtual std::vector<std::string> &GetElements()
  {
    return this->Elements;
  }

  
protected:

  
private:
  std::string Tag;
  std::string Name;
  std::string Description;
  std::string Label;
  std::string CPPType;
  std::string Type;
  std::string Reference;
  std::string Hidden;
  std::string ArgType;
  std::string StringToType;
  std::string Default;
  std::string Flag;
  std::string LongFlag;
  std::string Constraints;
  std::string Minimum;
  std::string Maximum;
  std::string Step;
  std::string Channel;
  std::string Index;
  std::string Multiple;
  std::string Aggregate;
  std::string FileExtensionsAsString;
  mutable std::vector<std::string> FileExtensions;
  std::string CoordinateSystem;
  std::vector<std::string> Elements;
  std::string FlagAliasesAsString;
  std::string DeprecatedFlagAliasesAsString;
  std::string LongFlagAliasesAsString;
  std::string DeprecatedLongFlagAliasesAsString;
  std::vector<std::string> FlagAliases;
  std::vector<std::string> DeprecatedFlagAliases;
  std::vector<std::string> LongFlagAliases;
  std::vector<std::string> DeprecatedLongFlagAliases;
};

ModuleDescriptionParser_EXPORT std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter);


#endif
