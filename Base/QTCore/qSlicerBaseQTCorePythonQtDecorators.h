/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerBaseQTCorePythonQtDecorators_h
#define __qSlicerBaseQTCorePythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerUtils.h"

#include "qSlicerBaseQTCoreExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qSlicerBaseQTBasePythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qSlicerBaseQTBasePythonQtDecorators()
    {
    PythonQt::self()->registerClass(&qSlicerCoreApplication::staticMetaObject);
    PythonQt::self()->registerClass(&qSlicerAbstractCoreModule::staticMetaObject);
    PythonQt::self()->registerCPPClass("qSlicerUtils", nullptr, "qSlicerBaseQTCore");
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  //----------------------------------------------------------------------------
  // qSlicerCoreApplication

  //----------------------------------------------------------------------------
  // static methods

  //----------------------------------------------------------------------------
  bool static_qSlicerCoreApplication_testingEnabled()
    {
    return qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
    }

  //----------------------------------------------------------------------------
  // instance methods

  //----------------------------------------------------------------------------
  void sendEvent(qSlicerCoreApplication* app, QObject* _receiver, QEvent* _event)
    {
    app->sendEvent(_receiver, _event);
    }

  //----------------------------------------------------------------------------
  void processEvents(qSlicerCoreApplication* app)
    {
    app->processEvents();
    }

  //----------------------------------------------------------------------------
  // qSlicerUtils

  //----------------------------------------------------------------------------
  // static methods

  QString static_qSlicerUtils_replaceWikiUrlVersion(const QString& text,
                                                    const QString& version)
  {
    return qSlicerUtils::replaceWikiUrlVersion(text, version);
  }
};

//-----------------------------------------------------------------------------
void initqSlicerBaseQTCorePythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qSlicerBaseQTBasePythonQtDecorators);
}

#endif
