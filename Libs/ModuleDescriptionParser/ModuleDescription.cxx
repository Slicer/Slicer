/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "ModuleDescription.h"
#include "ModuleDescriptionUtilities.h"

#include <sstream>
#include <fstream>
#include <string>

//----------------------------------------------------------------------------
ModuleDescription::ModuleDescription()
{
  this->Title = "Unknown";
  this->Type = "Unknown";
  this->Description = "No description provided";
  this->Category = "Unspecified";
  this->Version = "Unspecified";
  this->DocumentationURL = "";
  this->License = "";
  this->Acknowledgements = "Thank you everyone.";
  this->Contributor = "Anonymous";
  this->Target = "";
  this->Location = "";
  this->AlternativeType = "";
  this->AlternativeTarget = "";
  this->AlternativeLocation = "";

  std::stringstream ss;
  ss << (unsigned short) -1;
  ss >> this->Index;
}

//----------------------------------------------------------------------------
ModuleDescription::ModuleDescription(const ModuleDescription &md)
{
  this->Title = md.Title;
  this->Category = md.Category;
  this->Index = md.Index;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Acknowledgements = md.Acknowledgements;
  this->Contributor = md.Contributor;
  this->Type = md.Type;
  this->Target = md.Target;
  this->Location = md.Location;
  this->AlternativeType = md.AlternativeType;
  this->AlternativeTarget = md.AlternativeTarget;
  this->AlternativeLocation = md.AlternativeLocation;
  this->ParameterGroups = md.ParameterGroups;
  this->Logo = md.Logo;
  
  this->ProcessInformation.Initialize();
}

//----------------------------------------------------------------------------
void ModuleDescription::operator=(const ModuleDescription &md)
{
  this->Title = md.Title;
  this->Category = md.Category;
  this->Index = md.Index;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Acknowledgements = md.Acknowledgements;
  this->Contributor = md.Contributor;
  this->Type= md.Type;
  this->Target = md.Target;
  this->Location = md.Location;
  this->AlternativeType= md.AlternativeType;
  this->AlternativeTarget = md.AlternativeTarget;
  this->AlternativeLocation = md.AlternativeLocation;
  this->ParameterGroups = md.ParameterGroups;
  this->ProcessInformation = md.ProcessInformation;
  this->Logo = md.Logo;
}

//----------------------------------------------------------------------------
std::ostream & operator<<(std::ostream &os, const ModuleDescription &module)
{
  os << "Title: " << module.GetTitle() << std::endl;
  os << "Category: " << module.GetCategory() << std::endl;
  os << "Index: " << module.GetIndex() << std::endl;
  os << "Description: " << module.GetDescription() << std::endl;
  os << "Version: " << module.GetVersion() << std::endl;
  os << "DocumentationURL: " << module.GetDocumentationURL() << std::endl;
  os << "License: " << module.GetLicense() << std::endl;
  os << "Contributor: " << module.GetContributor() << std::endl;
  os << "Acknowledgements: " << module.GetAcknowledgements() << std::endl;
  os << "Type: " << module.GetType() << std::endl;
  os << "Target: " << module.GetTarget() << std::endl;
  os << "Location: " << module.GetLocation() << std::endl;
  os << "Alternative Type: " << module.GetAlternativeType() << std::endl;
  os << "Alternative Target: " << module.GetAlternativeTarget() << std::endl;
  os << "Alternative Location: " << module.GetAlternativeLocation() << std::endl;
  //os << "Logo: " << module.GetLogo() << std::endl;

  os << "ProcessInformation: " << std::endl
     << *(module.GetProcessInformation());
  
  os << "ParameterGroups: " << std::endl;
  std::vector<ModuleParameterGroup>::const_iterator it = module.GetParameterGroups().begin();
  while (it != module.GetParameterGroups().end())
    {
    os << *it;
    ++it;
    }
  return os;
}


//----------------------------------------------------------------------------
bool ModuleDescription::HasParameter(const std::string& name) const
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetName() == name)
        {
        return true;
        }
      }    
    }

  return false;
}

//----------------------------------------------------------------------------
bool ModuleDescription::HasReturnParameters() const
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).IsReturnParameter())
        {
        return true;
        }
      }    
    }

  return false;
}

//----------------------------------------------------------------------------
bool ModuleDescription::SetParameterDefaultValue(const std::string& name, const std::string& value)
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetName() == name)
        {
        (*pit).SetDefault(value);
        return true;
        }
      }    
    }

  return false;
}


//----------------------------------------------------------------------------
std::string ModuleDescription::GetParameterDefaultValue(const std::string& name) const
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetName() == name)
        {
        return (*pit).GetDefault();
        }
      }    
    }

  return "";
}

//----------------------------------------------------------------------------
void ModuleDescription ::SetLogo(const ModuleLogo& logo)
{
  this->Logo = logo;
}

//----------------------------------------------------------------------------
const ModuleLogo& ModuleDescription::GetLogo() const
{
  return this->Logo;
}

//----------------------------------------------------------------------------
bool ModuleDescription ::ReadParameterFile(const std::string& filename)
{
  std::ifstream rtp;
  bool modified = false;

  rtp.open(filename.c_str());
  if (rtp.fail())
    {
    std::cout << "Parameter file " << filename << " could not be opened." << std::endl;
    return false;
    }

  std::string line;
  while (std::getline(rtp, line))
    {
    // split the line into key: value
    std::string key, value;

    std::string::size_type start = line.find_first_not_of(" \t");
    std::string::size_type stop = line.find_first_of("=", start);
    
    key = line.substr(start, stop-start);
    start = line.find_first_not_of(" \t", stop+1);
    value = line.substr(start, line.length() - start + 1);
    
    trimLeadingAndTrailing(key);
    trimLeadingAndTrailing(value);
    
    // std::cout << "key=" << key << ", value=" << value << "!" << std::endl;

    if (this->HasParameter(key))
      {
      if (value != this->GetParameterDefaultValue(key))
        {
        this->SetParameterDefaultValue(key, value);
        modified = true;

        // multiple="true" may have to be handled differently
        }
      }
    }

  rtp.close();
  return modified;
}

//----------------------------------------------------------------------------
bool ModuleDescription::
WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters)
{
  std::ofstream rtp;

  rtp.open(filename.c_str());
  if (rtp.fail())
    {
    std::cout << "Parameter file " << filename << " could not be opened for writing." << std::endl;
    return false;
    }

  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      // write out all parameters or just the ones that are not bulk parameters
      if (withHandlesToBulkParameters
          || (!withHandlesToBulkParameters 
              && ((*pit).GetTag() != "image"
                  && (*pit).GetTag() != "geometry"
                  && (*pit).GetTag() != "transform"
                  && (*pit).GetTag() != "table"
                  && (*pit).GetTag() != "measurement"
                  && (*pit).GetTag() != "point"  // point and region are special
                  && (*pit).GetTag() != "region")))
        {
        rtp << (*pit).GetName() << " = " 
            << (*pit).GetDefault() << std::endl;

        // multiple="true" may have to be handled differently
        }
      }
    }

  rtp.close();
  return true;
}
