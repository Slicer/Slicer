/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractModuleWidget_h
#define __qSlicerAbstractModuleWidget_h

#include "qSlicerWidget.h"

#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseExport.h"

// class vtkSlicerApplicationLogic;
class vtkSlicerLogic;
class qSlicerAbstractModule; 
class QAction; 
class qSlicerAbstractModuleWidgetPrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractModuleWidget : public qSlicerWidget
{
  Q_OBJECT
public:

  typedef qSlicerWidget Superclass;
  qSlicerAbstractModuleWidget(QWidget *parent=0);

  // Description:
  // Return the action allowing to show the module
  virtual QAction* showModuleAction()  { return 0; }

  // Description:
  // Set/Get module name
  void setName(const QString& moduleName);
  QString name()const;

protected:
  // Description:
  // All inialization code should be done in the setup
  virtual void setup() = 0;

  // Description:
  // Set/Get associated Logic
  void setLogic(vtkSlicerLogic* logic);
  vtkSlicerLogic* logic() const; 

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractModuleWidget);

  // Give access to qSlicerAbstractModule to the method qSlicerAbstractModuleWidget::setLogic
  friend class qSlicerAbstractModule;
};

#endif
