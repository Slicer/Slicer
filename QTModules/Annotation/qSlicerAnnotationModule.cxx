
// Qt includes
#include <QtPlugin>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
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
      "vtkMRMLAnnotationTextThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationAngleThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationFiducialThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationStickyThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationSplineThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationRulerThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager");

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationROIThreeDViewDisplayableManager");

  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
      "vtkMRMLAnnotationSliceViewDisplayableManager");

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
  return "The Annotation module. Still under heavy development. For feedback please contact the Slicer mailing list (slicer-users@bwh.harvard.edu).";
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationModule::acknowledgementText() const
{
  return "This module was developed by Daniel Haehn, Kilian Pohl and Yong Zhang. Thank you to Nicole Aucoin, Wendy Plesniak, Steve Pieper and Ron Kikinis. The research was funded by an ARRA supplement to NIH NCRR (P41 RR13218).";
}

//-----------------------------------------------------------------------------
QIcon qSlicerAnnotationModule::icon() const
{
  return QIcon(":/Icons/AnnotationReport.png");
}
