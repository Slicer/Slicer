/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerMarkupsModuleWidgetsPythonQtDecorators_h
#define __qSlicerMarkupsModuleWidgetsPythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qMRMLMarkupsOptionsWidgetsFactory.h"

#include "qSlicerMarkupsModuleWidgetsExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qSlicerMarkupsModuleWidgetsPythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qSlicerMarkupsModuleWidgetsPythonQtDecorators()
    {
    //PythonQt::self()->registerClass(&qMRMLMarkupsOptionsWidgetsFactory::staticMetaObject);
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  //----------------------------------------------------------------------------
  // qMRMLMarkupsOptionsWidgetsFactory

  //----------------------------------------------------------------------------
  // static methods

  //----------------------------------------------------------------------------
  qMRMLMarkupsOptionsWidgetsFactory* static_qMRMLMarkupsOptionsWidgetsFactory_instance()
    {
    return qMRMLMarkupsOptionsWidgetsFactory::instance();
    }

  //----------------------------------------------------------------------------
  // instance methods

  //----------------------------------------------------------------------------
  bool registerOptionsWidget(qMRMLMarkupsOptionsWidgetsFactory* factory,
                                       PythonQtPassOwnershipToCPP<qMRMLMarkupsAbstractOptionsWidget*> plugin)
    {
    return factory->registerOptionsWidget(plugin);
    }
};

//-----------------------------------------------------------------------------
void initqSlicerMarkupsModuleWidgetsPythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qSlicerMarkupsModuleWidgetsPythonQtDecorators);
}

#endif
