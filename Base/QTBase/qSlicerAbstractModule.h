#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#define qSlicerGetTitleMacro(_TITLE)               \
  static QString staticTitle() { return _TITLE; }  \
  virtual QString title()const { return _TITLE; }

#include "qSlicerBaseQTBaseWin32Header.h"

class qSlicerAbstractModuleWidget; 
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
  virtual QString name()const = 0;
  virtual void setName(const QString& name) = 0; 
  virtual QString title()const = 0;
  virtual QString category()const { return QString(); }
  virtual QString contributor()const { return QString(); }

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText()const {return "";}
  virtual QString acknowledgementText()const { return "";}

  // Description:
  // This method allows to get a pointer to the WidgetRepresentation created by
  // method 'createWidgetRepresentation'
  // Nota: At time of destruction, if the widget created doesn't have any parent,
  // it will be automatically deleted.
  qSlicerAbstractModuleWidget* widgetRepresentation();

  // Description:
  // Set the application logic
  //void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  vtkSlicerApplicationLogic* appLogic() const;

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
  // Create and return a widget representation of the module.
  virtual qSlicerAbstractModuleWidget* createWidgetRepresentation() = 0;

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractModule);

  // Description:
  // Indicate if the module has already been initialized
  bool Initialized;
};

#endif
