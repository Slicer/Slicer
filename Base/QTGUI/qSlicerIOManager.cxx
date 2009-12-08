#include "qSlicerIOManager.h"

//-----------------------------------------------------------------------------
struct qSlicerIOManagerPrivate: public qCTKPrivate<qSlicerIOManager>
{

};

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerIOManager);
}

//-----------------------------------------------------------------------------
qSlicerIOManager::~qSlicerIOManager()
{
}
