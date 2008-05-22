/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#include <string>
#include "ModuleFactory.h"
#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"

void WarningMessage(const char *msg);
void ErrorMessage(const char *msg);
void InformationMessage(const char *msg);
void DebugMessage(const char *msg);
void SplashMessage(const char *msg);

//
// Test the module factory
//

int main (int argc, char *argv[])
{

  if (argc < 2)
    {
    std::cout << "Usage: " << argv[0] << " search_path" << std::endl;
    return EXIT_FAILURE;
    }

  itksys::Directory myDir;
  myDir.Load(argv[1]);
  for ( unsigned long i = 0; i < myDir.GetNumberOfFiles(); i++)
    {
    if (strcmp (myDir.GetFile(i), "..") == 0) continue;

    ModuleFactory moduleFactory;
    std::string searchPath(argv[1]);
    searchPath += myDir.GetFile(i);
    if (!itksys::SystemTools::FileIsDirectory(searchPath.c_str())) continue;
    moduleFactory.SetSearchPaths( searchPath );
    moduleFactory.SetWarningMessageCallback( WarningMessage );
    moduleFactory.SetErrorMessageCallback( ErrorMessage );
    moduleFactory.SetInformationMessageCallback( InformationMessage );
    moduleFactory.SetModuleDiscoveryMessageCallback( SplashMessage );
    moduleFactory.Scan();

    // Get the module names
    std::vector<std::string> moduleNames = moduleFactory.GetModuleNames();
    
    std::vector<std::string>::const_iterator mit;
    mit = moduleNames.begin();
    while (mit != moduleNames.end())
      {
      std::cout << "Found module: " << *mit << std::endl;
      ++mit;
      }
    }
  return EXIT_SUCCESS;
}

void WarningMessage(const char *msg)
{
  std::cout << "Warning: " << msg << std::endl;
}

void ErrorMessage(const char *msg)
{
  std::cout << "Error: " << msg << std::endl;
}

void InformationMessage(const char *msg)
{
  std::cout << "Information: " << msg << std::endl;
}

void DebugMessage(const char *msg)
{
  std::cout << "Debug: " << msg << std::endl;
}

void SplashMessage(const char *msg)
{
  std::cout << "Splash: " << msg << std::endl;
}
