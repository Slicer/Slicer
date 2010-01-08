/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#define qSlicerGetTitleMacro(_TITLE)               \
  static QString staticTitle() { return _TITLE; }  \
  virtual QString title()const { return _TITLE; }

#include "qSlicerBaseQTBaseExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerModuleLogic; 
class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class QAction; 
class qSlicerAbstractModulePrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractModule : public QObject
{
  Q_OBJECT

public:

  typedef QObject Superclass;
  qSlicerAbstractModule(QObject *parent=0);

  virtual void printAdditionalInfo();

  // Description:
  // All initialization code should be done in the initialize function
  void initialize(vtkSlicerApplicationLogic* appLogic);
  inline bool initialized() { return this->Initialized; }

  // Description:
  virtual QString name()const;
  virtual void setName(const QString& name); 
  virtual QString title()const = 0;
  virtual QString category()const { return QString(); }
  virtual QString contributor()const { return QString(); }

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText()const {return "";}
  virtual QString acknowledgementText()const { return "";}

  // Description:
  // This method allows to get a pointer to the WidgetRepresentation.
  // If no WidgetRepresentation already exists, one will be created calling
  // 'createWidgetRepresentation' method.
  qSlicerAbstractModuleWidget* widgetRepresentation();

  // Description:
  // Set the application logic
  //void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  vtkSlicerApplicationLogic* appLogic() const;

  // Description:
  // This method allows to get a pointer to the ModuleLogic.
  // If no moduleLogic already exists, one will be created calling
  // 'createLogic' method.
  qSlicerModuleLogic* logic();

  // Description:
  // Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;
  
public slots:

  // Description:
  // Set/Get module enabled
  bool moduleEnabled()const;
  virtual void setModuleEnabled(bool value);

  // Description:
  // Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  // Description:
  // All inialization code should be done in the setup
  virtual void setup() = 0;

  // Description:
  // Create and return a widget representation for the module.
  virtual qSlicerAbstractModuleWidget* createWidgetRepresentation() = 0;

  // Description:
  // create and return the module logic
  // Note: Only one instance of the logic will exist per module
  virtual qSlicerModuleLogic* createLogic() = 0;
  

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractModule);

  // Description:
  // Indicate if the module has already been initialized
  bool Initialized;
};

#endif
