/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCommandOptions.h"

//-----------------------------------------------------------------------------
// qSlicerCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCommandOptions::qSlicerCommandOptions(QSettings* _settings):Superclass(_settings)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::noSplash() const
{
  return this->parsedArgs().value("no-splash").toBool();
}

//-----------------------------------------------------------------------------
void qSlicerCommandOptions::addArguments()
{
  this->Superclass::addArguments();

  this->addArgument("no-splash", "", QVariant::Bool,
                    "Disables the startup splash screen.");
}
