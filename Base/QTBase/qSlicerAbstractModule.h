#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h

#include "qSlicerWidget.h"

#define qSlicerGetTitleMacro(_TITLE)               \
  static QString staticTitle() { return _TITLE; }  \
  virtual QString title()const { return _TITLE; }

#include "qSlicerBaseQTBaseWin32Header.h"

class vtkSlicerApplicationLogic;
class vtkMRMLScene;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractModule : public qSlicerWidget
{
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerAbstractModule(QWidget *parent=0);
  virtual ~qSlicerAbstractModule();

  virtual void printAdditionalInfo();

  // Description:
  // All initialization code should be done in the initialize function
  void initialize(vtkSlicerApplicationLogic* appLogic);
  inline bool initialized() { return this->Initialized; }

  // Description:
  virtual QString name()const;
  virtual QString title()const = 0;
  virtual QString category()const { return QString(); }
  virtual QString contributor()const { return QString(); }

  // Description:
  virtual void populateToolbar(){}
  virtual void unPopulateToolbar(){}

  // Description:
  virtual void populateApplicationSettings(){}
  virtual void unPopulateApplicationSettings(){}

  // Description:
  // Return help/acknowledgement text
  virtual QString helpText()const {return "";}
  virtual QString acknowledgementText()const { return "";}

public slots:

  // Description:
  // Set/Get module enabled
  bool moduleEnabled();
  virtual void setModuleEnabled(bool value);

protected:
  // Description:
  // All inialization code should be done in the setup
  virtual void setup() = 0;

private:
  struct qInternal;
  qInternal* Internal;

  // Description:
  // Indicate if the module has already been initialized
  bool Initialized;
};

#endif
