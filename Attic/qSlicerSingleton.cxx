#include "qSlicerModuleManager.h"

//-----------------------------------------------------------------------------
struct qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
  QCTK_DECLARE_PUBLIC(qSlicerModuleManager);
  
  qSlicerModuleManagerPrivate()
    {
    }
};

//----------------------------------------------------------------------------
void qSlicerModuleManager::classInitialize()
{
  Self::Instance = new qSlicerModuleManager;
}

//----------------------------------------------------------------------------
void qSlicerModuleManager::classFinalize()
{
  delete Self::Instance;
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager()
{
  QCTK_INIT_PRIVATE(qSlicerModuleManager);
  //QCTK_D(qSlicerModuleManager);
}

//----------------------------------------------------------------------------
// Implementation of qSlicerModuleManagerInitialize class.
//----------------------------------------------------------------------------
qSlicerModuleManagerInitialize::qSlicerModuleManagerInitialize()
{
  if(++Self::Count == 1)
    { qSlicerModuleManager::classInitialize(); }
}

//----------------------------------------------------------------------------
qSlicerModuleManagerInitialize::~qSlicerModuleManagerInitialize()
{
  if(--Self::Count == 0)
    { qSlicerModuleManager::classFinalize(); }
}

//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int qSlicerModuleManagerInitialize::Count;
qSlicerModuleManager* qSlicerModuleManager::Instance;
