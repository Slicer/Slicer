// QT includes
#include <QVariant>

// QSlicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// SceneViewWidget includes
#include "qSlicerSceneViewsModuleDialog.h"

// SceneViewLogic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkStdString.h>

// STD includes
#include <vector>

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::qSlicerSceneViewsModuleDialog()
{
  this->m_Logic = 0;
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());
  this->setShowScaleFactorSpinBox(false);
  this->setWindowTitle("3D Slicer SceneView");

  // default name
  QString name("SceneView");
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::~qSlicerSceneViewsModuleDialog()
{

  if(this->m_Logic)
    {
    this->m_Logic = 0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::setLogic(vtkSlicerSceneViewsModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the SceneViews module logic here!");
    return;
    }
  this->m_Logic = logic;
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::loadNode(const QString& nodeId)
{
  if (!this->m_Logic)
    {
    qErrnoWarning("initialize: We need the SceneViews module logic here!");
    return;
    }
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());
  this->setData(QVariant(nodeId));

  // get the name..
  vtkStdString name = this->m_Logic->GetSceneViewName(nodeId.toLatin1());

  // ..and set it in the GUI
  this->setNameEdit(QString::fromStdString(name));

  // get the description..
  vtkStdString description = this->m_Logic->GetSceneViewDescription(nodeId.toLatin1());

  // ..and set it in the GUI
  this->setDescription(QString::fromStdString(description));

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSceneViewScreenshotType(nodeId.toLatin1());

  // ..and set it in the GUI
  this->setWidgetType((qMRMLScreenShotDialog::WidgetType)screenshotType);

  vtkImageData* imageData = this->m_Logic->GetSceneViewScreenshot(nodeId.toLatin1());
  this->setImageData(imageData);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::reset()
{
  QString name = this->nameEdit();
  if (name.length() == 0)
    {
    name = QString("SceneView");
    }
  // check to see if it's an already used name for a node (redrawing the
  // dialog causes it to reset and calling GetUniqueNameByString increments
  // the number each time).
  QByteArray nameBytes = name.toLatin1();
  vtkCollection* col = this->m_Logic->GetMRMLScene()->GetNodesByName(nameBytes.data());
  if (col->GetNumberOfItems() > 0)
    {
    // get a new unique name
    name = this->m_Logic->GetMRMLScene()->GetUniqueNameByString(name.toLatin1());
    }
  col->Delete();
  this->resetDialog();
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::accept()
{
  // name
  QString name = this->nameEdit();
  QByteArray nameBytes = name.toLatin1();

  // description
  QString description = this->description();
  QByteArray descriptionBytes = description.toLatin1();

  // we need to know of which type the screenshot is
  int screenshotType = static_cast<int>(this->widgetType());

  if (this->data().toString().isEmpty())
    {
    // this is a new SceneView
    this->m_Logic->CreateSceneView(nameBytes.data(),descriptionBytes.data(),
                                   screenshotType,this->imageData());
    //QMessageBox::information(this, "3D Slicer SceneView created",
    //             "A new SceneView was created and the current scene was attached.");
    }
  else
    {
    // this SceneView already exists
    this->m_Logic->ModifySceneView(vtkStdString(this->data().toString().toLatin1()),nameBytes.data(),descriptionBytes.data()
                                   ,screenshotType,this->imageData());
    //QMessageBox::information(this, "3D Slicer SceneView updated",
    //             The SceneView was updated without changing the attached scene.");
    }
  this->Superclass::accept();
}

