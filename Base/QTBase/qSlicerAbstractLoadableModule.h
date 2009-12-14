/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractLoadableModule_h
#define __qSlicerAbstractLoadableModule_h

#include "qSlicerAbstractModule.h"

#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseWin32Header.h"

class qSlicerAbstractLoadableModulePrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractLoadableModule(QObject *parent=0);

  virtual QString name() const;
  virtual void setName(const QString&);

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();

protected:
  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractLoadableModule);
};

Q_DECLARE_INTERFACE(qSlicerAbstractLoadableModule,
                     "org.slicer.QTModules.qSlicerAbstractLoadableModule/1.0");

#endif
