/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QSettings>
#include <QDir>
#include <QDebug>

// SlicerQt includes
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerCoreApplication.h" // For disableCurrentSettings()

//-----------------------------------------------------------------------------
class qSlicerCoreCommandOptionsPrivate: public ctkPrivate<qSlicerCoreCommandOptions>
{
public:
  qSlicerCoreCommandOptionsPrivate();

  bool DisableCLIModule;
  bool DisableLoadableModule;
  bool DisplayVersionAndExit;
  bool DisplayProgramPathAndExit;
  bool DisplayHomePathAndExit;
};

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptionsPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptionsPrivate::qSlicerCoreCommandOptionsPrivate()
{
  this->DisableCLIModule = false;
  this->DisableLoadableModule = false;
  this->DisplayVersionAndExit = false;
  this->DisplayProgramPathAndExit = false;
  this->DisplayHomePathAndExit = false;
}

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions::qSlicerCoreCommandOptions(QSettings* _settings):Superclass(_settings)
{
  CTK_INIT_PRIVATE(qSlicerCoreCommandOptions);
}

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions::~qSlicerCoreCommandOptions()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerCoreCommandOptions, bool, disableCLIModule, DisableCLIModule);
CTK_GET_CXX(qSlicerCoreCommandOptions, bool, disableLoadableModule, DisableLoadableModule);
CTK_GET_CXX(qSlicerCoreCommandOptions, bool, displayVersionAndExit, DisplayVersionAndExit);
CTK_GET_CXX(qSlicerCoreCommandOptions, bool, displayProgramPathAndExit, DisplayProgramPathAndExit);
CTK_GET_CXX(qSlicerCoreCommandOptions, bool, displayHomePathAndExit, DisplayHomePathAndExit);

//-----------------------------------------------------------------------------
QString qSlicerCoreCommandOptions::tempDirectory()const
{
  return this->settings()->value("temp-directory", QVariant(QDir::tempPath())).toString();
}

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptions::initialize()
{
  CTK_D(qSlicerCoreCommandOptions);
  
  this->Superclass::initialize();

  this->addBooleanArgument("--disable-cli-module", 0, &d->DisableCLIModule,
                           "Disables the loading of Command Line Modules.");

  this->addBooleanArgument("--disable-loadable-module", 0, &d->DisableLoadableModule,
                           "Disables the loading of Loadable Modules.");

  this->addBooleanArgument("--version", 0, &d->DisplayVersionAndExit,
                           "Displays version information and exits.");

  this->addBooleanArgument("--program-path", 0, &d->DisplayProgramPathAndExit,
                           "Displays application program path and exits.");

  this->addBooleanArgument("--home", 0, &d->DisplayHomePathAndExit,
                           "Displays home path and exits.");
}

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptions::disableCurrentSettings()
{
  qSlicerCoreApplication::application()->disableSettings();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::wrongArgument(const char* argument)
{
  qWarning() << "Unknown argument:" << argument;
  return false;
}
