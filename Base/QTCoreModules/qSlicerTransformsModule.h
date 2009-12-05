#ifndef __qSlicerTransformsModule_h
#define __qSlicerTransformsModule_h

// SlicerQT includes
#include "qSlicerAbstractCoreModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesWin32Header.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerTransformsModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT
public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerTransformsModule(QObject *parent=0);

  qSlicerGetTitleMacro("Transforms");

  // Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:

  // Description:
  // Create and return a widget representation of the object
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();
  
private:
  QCTK_DECLARE_PRIVATE(qSlicerTransformsModule);
};

#endif
