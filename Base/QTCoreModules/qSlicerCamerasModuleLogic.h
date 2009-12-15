#ifndef __qSlicerCamerasModuleLogic_h
#define __qSlicerCamerasModuleLogic_h

// SlicerQT includes
#include "qSlicerModuleLogic.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerCamerasModuleLogicPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModuleLogic : public qSlicerModuleLogic
{
public:

  typedef qSlicerModuleLogic Superclass;
  qSlicerCamerasModuleLogic();

  void synchronizeCameraWithView(void * currentView= 0);

protected:

  virtual void setup();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCamerasModuleLogic);
};

#endif
