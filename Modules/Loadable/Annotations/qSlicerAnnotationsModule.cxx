
// Qt includes
#include <QtPlugin>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// QTGUI includes
#include <qSlicerApplication.h>

// AnnotationModule includes
#include "qSlicerAnnotationsModule.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerAnnotationsModule, qSlicerAnnotationsModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationsModulePrivate
{
  public:
};

//-----------------------------------------------------------------------------
qSlicerAnnotationsModule::qSlicerAnnotationsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAnnotationsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAnnotationsModule::~qSlicerAnnotationsModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsModule::setup()
{
  // 3D
  QStringList threeDdisplayableManagers;
  threeDdisplayableManagers
      //<< "TextThreeDView"
      //<< "AngleThreeDView"
      //<< "Text"
      << "Fiducial"
      //<< "Bidimensional"
      << "Ruler"
      << "ROI"
      //<< "StickyThreeDView"
      //<< "SplineThreeDView"
      //<< "RulerThreeDView"
      //<< "BidimensionalThreeDView"
      ;

  foreach(const QString& name, threeDdisplayableManagers)
    {
    vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
        QString("vtkMRMLAnnotation%1DisplayableManager").arg(name).toLatin1());
    }

  // 2D
  QStringList slicerViewDisplayableManagers;
  slicerViewDisplayableManagers
      //<< "Text"
      << "Fiducial"
      //<< "Bidimensional"
      << "Ruler"
      << "ROI"
      ;
  foreach(const QString& name, slicerViewDisplayableManagers)
    {
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
        QString("vtkMRMLAnnotation%1DisplayableManager").arg(name).toLatin1());
    }

}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAnnotationsModule::createWidgetRepresentation()
{
  return new qSlicerAnnotationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerAnnotationsModule::createLogic()
{
  return vtkSlicerAnnotationModuleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationsModule::helpText() const
{
  return "The Annotation module. Still under heavy development. "
      "For feedback please contact the Slicer mailing list (slicer-users@bwh.harvard.edu).";
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationsModule::acknowledgementText() const
{
  return "<img src=':/Icons/UPenn_logo.png'><br><br>This module was "
      "developed by Daniel Haehn, Kilian Pohl and Yong Zhang. "
      "Thank you to Nicole Aucoin, Wendy Plesniak, Steve Pieper, Ron Kikinis and Kitware. "
      "The research was funded by an ARRA supplement to NIH NCRR (P41 RR13218).";
}

//-----------------------------------------------------------------------------
QIcon qSlicerAnnotationsModule::icon() const
{
  return QIcon(":/Icons/Annotation.png");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsModule::showScreenshotDialog()
{
  Q_ASSERT(this->widgetRepresentation());
  dynamic_cast<qSlicerAnnotationModuleWidget*>(this->widgetRepresentation())
      ->grabSnapShot();
}
