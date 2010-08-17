
// Qt includes
#include <QtPlugin>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// AnnotationModule includes
#include "qSlicerAnnotationModule.h"
#include "GUI/qSlicerAnnotationModuleWidget.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

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
      "vtkMRMLAnnotationAngleDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationFiducialDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationStickyDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationSplineDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationRulerDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationBidimensionalDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationROIDisplayableManager");

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
QString qSlicerAnnotationModule::helpText() const
{
  return "The Annotation module.";
}
