#ifndef __qSlicerCoreIOManager_h
#define __qSlicerCoreIOManager_h

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreWin32Header.h"


class qSlicerCoreIOManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreIOManager
{

public:
  qSlicerCoreIOManager();
  virtual ~qSlicerCoreIOManager();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreIOManager);
};

#endif
