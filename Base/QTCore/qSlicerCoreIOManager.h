#ifndef __qSlicerCoreIOManager_h
#define __qSlicerCoreIOManager_h

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreWin32Header.h"

class vtkMRMLScene; 
class qSlicerCoreIOManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreIOManager
{

public:
  qSlicerCoreIOManager();
  virtual ~qSlicerCoreIOManager();

  // Description:
  // Set MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);

  // Description:
  // Load/Import scene
  void loadScene(const QString& filename);
  void importScene(const QString& filename);

  // Description:
  // Close scene
  void closeScene();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreIOManager);
};

#endif

