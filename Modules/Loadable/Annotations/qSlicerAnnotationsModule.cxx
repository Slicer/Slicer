// QTGUI includes
#include <QMainWindow>
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <vtkSlicerConfigure.h> // For Slicer_USE_PYTHONQT

// AnnotationModule includes
#include "qSlicerAnnotationsModule.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "GUI/qSlicerAnnotationModuleSnapShotDialog.h"

// PythonQt includes
#ifdef Slicer_USE_PYTHONQT
#include "PythonQt.h"
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationsModulePrivate
{
  public:
    qSlicerAnnotationModuleSnapShotDialog* m_SnapShotDialog{nullptr};
};

//-----------------------------------------------------------------------------
qSlicerAnnotationsModule::qSlicerAnnotationsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAnnotationsModulePrivate)
{
  this->setWidgetRepresentationCreationEnabled(false);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationsModule::~qSlicerAnnotationsModule()
{
  Q_D(qSlicerAnnotationsModule);
  if (d->m_SnapShotDialog)
    {
    // If the application was started without main window then snapshot dialog
    // does not have a parent, so we need to delete it manually.
    if (!d->m_SnapShotDialog->parent())
      {
      delete d->m_SnapShotDialog;
      d->m_SnapShotDialog = nullptr;
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsModule::setup()
{
  Q_D(qSlicerAnnotationsModule);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAnnotationsModule::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerAnnotationsModule::createLogic()
{
  return vtkSlicerAnnotationModuleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationsModule::helpText() const
{
  QString help = QString(
  "Legacy Annotations module, replaced by Markups module."
   "The module class is only preserved to be able to load old scenes and manage screenshots (snapshots).<br>");
  help += this->defaultDocumentationLink();
  return help;
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
  return QStringList() << "Legacy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerAnnotationsModule::dependencies() const
{
  QStringList moduleDependencies;
  moduleDependencies << "SubjectHierarchy";
  return moduleDependencies;
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsModule::showScreenshotDialog()
{
  Q_D(qSlicerAnnotationsModule);
  if (!d->m_SnapShotDialog)
    {
    QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
    d->m_SnapShotDialog = new qSlicerAnnotationModuleSnapShotDialog(mainWindow);
    }
  d->m_SnapShotDialog->setLogic(vtkSlicerAnnotationModuleLogic::SafeDownCast(this->logic()));
  d->m_SnapShotDialog->reset();
  d->m_SnapShotDialog->open();
}

//-----------------------------------------------------------------------------
QStringList qSlicerAnnotationsModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLAnnotationNode"
    << "vtkMRMLAnnotationDisplayNode"
    << "vtkMRMLAnnotationStorageNode"
    << "vtkMRMLAnnotationHierarchyNode";
}
