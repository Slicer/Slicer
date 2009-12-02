#ifndef __qSlicerModuleLogic_h
#define __qSlicerModuleLogic_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qSlicerBaseQTBaseWin32Header.h"

class vtkMRMLScene;
class vtkSlicerApplicationLogic;
class qSlicerModuleLogicPrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerModuleLogic : public QObject
{
  Q_OBJECT
public:

  typedef QObject Superclass;
  qSlicerModuleLogic(QObject *parent);

  virtual void printAdditionalInfo();

  void initialize(vtkSlicerApplicationLogic* appLogic);
  virtual void setup() = 0;

protected:

  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  vtkMRMLScene* mrmlScene() const;

  // Description:
  // Get application logic
  vtkSlicerApplicationLogic* appLogic() const;

private:
  QCTK_DECLARE_PRIVATE(qSlicerModuleLogic);
};

#endif
