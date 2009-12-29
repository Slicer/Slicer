#include "qSlicerCamerasModuleLogic.h"

//-----------------------------------------------------------------------------
struct qSlicerCamerasModuleLogicPrivate: public qCTKPrivate<qSlicerCamerasModuleLogic>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_NO_ARG_CXX(qSlicerCamerasModuleLogic);

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::setup()
{
  //this->Superclass::setup();
  
  //QCTK_D(qSlicerCamerasModuleLogic);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::synchronizeCameraWithView( void * currentView )
{
  if( currentView == NULL )
    {
    return;
    }
}
