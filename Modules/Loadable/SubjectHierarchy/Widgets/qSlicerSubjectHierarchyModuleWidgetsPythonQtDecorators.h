/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators_h
#define __qSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerSubjectHierarchyPluginHandler.h"

#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators()
    {
    //PythonQt::self()->registerClass(&qSlicerSubjectHierarchyPluginHandler::staticMetaObject);
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  //----------------------------------------------------------------------------
  // qSlicerSubjectHierarchyPluginHandler

  //----------------------------------------------------------------------------
  // static methods

  //----------------------------------------------------------------------------
  qSlicerSubjectHierarchyPluginHandler* static_qSlicerSubjectHierarchyPluginHandler_instance()
    {
    return qSlicerSubjectHierarchyPluginHandler::instance();
    }

  //----------------------------------------------------------------------------
  // instance methods

  //----------------------------------------------------------------------------
  bool registerPlugin(qSlicerSubjectHierarchyPluginHandler* handler,
                      PythonQtPassOwnershipToCPP<qSlicerSubjectHierarchyAbstractPlugin*> plugin)
    {
    return handler->registerPlugin(plugin);
    }
};

//-----------------------------------------------------------------------------
void initqSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qSlicerSubjectHierarchyModuleWidgetsPythonQtDecorators);
}

#endif
