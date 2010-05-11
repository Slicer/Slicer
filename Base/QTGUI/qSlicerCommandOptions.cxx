/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCommandOptions.h"

//-----------------------------------------------------------------------------
class qSlicerCommandOptionsPrivate: public ctkPrivate<qSlicerCommandOptions>
{
public:
  qSlicerCommandOptionsPrivate();

  bool NoSplash;

};

//-----------------------------------------------------------------------------
// qSlicerCommandOptionsPrivate methods

//-----------------------------------------------------------------------------
qSlicerCommandOptionsPrivate::qSlicerCommandOptionsPrivate()
{
  this->NoSplash = false;
}

//-----------------------------------------------------------------------------
// qSlicerCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCommandOptions::qSlicerCommandOptions(QSettings* _settings):Superclass(_settings)
{
  CTK_INIT_PRIVATE(qSlicerCommandOptions);
}

//-----------------------------------------------------------------------------
qSlicerCommandOptions::~qSlicerCommandOptions()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerCommandOptions, bool, noSplash, NoSplash);

//-----------------------------------------------------------------------------
void qSlicerCommandOptions::initialize()
{
  CTK_D(qSlicerCommandOptions);
  
  this->Superclass::initialize();

  this->addBooleanArgument("--no-splash", 0, &d->NoSplash,
                           "Disables the startup splash screen.");
}
