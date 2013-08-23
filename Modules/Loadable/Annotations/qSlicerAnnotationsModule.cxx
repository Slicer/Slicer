
// Qt includes
#include <QtPlugin>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerNodeWriter.h>

// AnnotationModule includes
#include "AnnotationsInstantiator.h"
#include "qSlicerAnnotationsModule.h"
#include "GUI/qSlicerAnnotationModuleWidget.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "qSlicerAnnotationsIO.h"

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
  /// Register Displayable Managers:
  
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

  /// Register IO
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(
    new qSlicerAnnotationsIO(vtkSlicerAnnotationModuleLogic::SafeDownCast(this->logic()), this));

  ioManager->registerIO(new qSlicerNodeWriter(
    "Annotations", QString("AnnotationFile"),
    QStringList() << "vtkMRMLAnnotationNode", this));
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
  QString help = QString("The Annotations module, create and edit supplementary information associated with a scene.<br>"
                         "Currently supported annotations are fiducial points, rulers, and regions of interest (ROIs).<br>"
                         "<a href=\"%1/Documentation/%2.%3/Modules/Annotations\">"
                         "%1/Documentation/%2.%3/Modules/Annotations</a><br>");

  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
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
QStringList qSlicerAnnotationsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  moduleContributors << QString("Daniel Haehn (UPenn)");
  moduleContributors << QString("Kilian Pohl (UPenn)");
  moduleContributors << QString("Yong Zhang (IBM)");
  moduleContributors << QString("Wendy Plesniak (SPL, BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerAnnotationsModule::icon() const
{
  return QIcon(":/Icons/Annotation.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerAnnotationsModule::categories() const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsModule::showScreenshotDialog()
{
  Q_ASSERT(this->widgetRepresentation());
  dynamic_cast<qSlicerAnnotationModuleWidget*>(this->widgetRepresentation())
      ->grabSnapShot();
}
