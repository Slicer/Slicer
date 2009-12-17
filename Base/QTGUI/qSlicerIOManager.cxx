#include "qSlicerIOManager.h"

//-----------------------------------------------------------------------------
class qSlicerIOManagerPrivate: public qCTKPrivate<qSlicerIOManager>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerIOManager);
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
