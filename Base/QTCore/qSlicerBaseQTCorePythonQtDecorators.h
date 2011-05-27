/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerBaseQTCorePythonQtDecorators_h
#define __qSlicerBaseQTCorePythonQtDecorators_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkAbstractPythonManager.h>

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as NULL.  The self argument is the first argument
// for non-static methods.
//

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerBaseQTBasePythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qSlicerBaseQTBasePythonQtDecorators(ctkAbstractPythonManager* pythonManager)
    {
    Q_ASSERT(pythonManager);
    pythonManager->registerClassForPythonQt(&qSlicerCoreApplication::staticMetaObject);
    pythonManager->registerClassForPythonQt(&qSlicerAbstractCoreModule::staticMetaObject);
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  // static methods
  QList<QWidget*> static_qSlicerCoreApplication_allWidgets()
    { 
    return qSlicerCoreApplication::allWidgets();
    }

  //----------------------------------------------------------------------------
  // qSlicerCoreApplication

  void sendEvent(qSlicerCoreApplication* app, QObject* _receiver, QEvent* _event)
    {
    app->sendEvent(_receiver, _event);
    }

  void processEvents(qSlicerCoreApplication* app)
    {
    app->processEvents();
    }
};

#endif
