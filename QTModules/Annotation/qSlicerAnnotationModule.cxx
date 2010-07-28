
// Qt includes
#include <QtPlugin>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLAbstractDisplayableManager.h>

// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// AnnotationModule includes
#include "vtkMRMLAnnotationTextDisplayableManager.h"
#include "qSlicerAnnotationModule.h"
#include "qSlicerAnnotationModuleWidget.h"
#include "vtkSlicerAnnotationModuleLogic.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerAnnotationModule, qSlicerAnnotationModule);

//-----------------------------------------------------------------------------
class qSlicerAnnotationModulePrivate: public ctkPrivate<qSlicerAnnotationModule>
{
  public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAnnotationModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAnnotationModule::setup()
{

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationTextDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationFiducialDisplayableManager");

  //vtkMRMLThreeDViewDisplayableManagerFactory * factory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  //factory->RegisterDisplayableManager("vtkMRMLAnnotationTextDisplayableManager");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAnnotationModule::createWidgetRepresentation()
{
  return new qSlicerAnnotationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAnnotationModule::createLogic()
{
  return vtkSlicerAnnotationModuleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText() const
{
  return "The Annotation module.";
}
