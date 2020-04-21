/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerBaseQTAppPythonQtDecorators_h
#define __qSlicerBaseQTAppPythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerApplicationHelper.h"

#include "qSlicerBaseQTAppExport.h"


// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qSlicerBaseQTAppPythonQtDecorators : public QObject
{
  Q_OBJECT

public:

  qSlicerBaseQTAppPythonQtDecorators() = default;

public slots:

  //----------------------------------------------------------------------------
  // qSlicerApplicationHelper

  //----------------------------------------------------------------------------
  void static_qSlicerApplicationHelper_setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager)
    {
    qSlicerApplicationHelper::setupModuleFactoryManager(moduleFactoryManager);
    }
};

//-----------------------------------------------------------------------------
void initqSlicerBaseQTAppPythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qSlicerBaseQTAppPythonQtDecorators);
}

#endif
