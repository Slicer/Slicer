#ifndef __qSlicerIOManager_h
#define __qSlicerIOManager_h

// SlicerQT includes
#include "qSlicerCoreIOManager.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreWin32Header.h"


class qSlicerIOManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIOManager : public qSlicerCoreIOManager
{

public:
  typedef qSlicerCoreIOManager Superclass;
  qSlicerIOManager();
  virtual ~qSlicerIOManager();

private:
  QCTK_DECLARE_PRIVATE(qSlicerIOManager);
};

#endif
