// Qt includes
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

// STD includes
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::qSlicerSceneViewsModuleDialog(QWidget* parent/*=nullptr*/)
  : qMRMLScreenShotDialog(parent)
{
  this->m_Logic = nullptr;
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());
  this->setShowScaleFactorSpinBox(false);
  this->setWindowTitle(tr("3D Slicer SceneView"));

  // default name
  QString name("SceneView");
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::~qSlicerSceneViewsModuleDialog()
{

  if(this->m_Logic)
  {
    this->m_Logic = nullptr;
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
void qSlicerSceneViewsModuleDialog::loadSceneViewInfo(int index)
{
  if (!this->m_Logic)
  {
    qErrnoWarning("initialize: We need the SceneViews module logic here!");
    return;
  }
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());

  this->setData(QVariant(index));

  // get the name..
  std::string name = this->m_Logic->GetNthSceneViewName(index);

  // ..and set it in the GUI
  this->setNameEdit(QString::fromStdString(name));

  // get the description..
  std::string description = this->m_Logic->GetNthSceneViewDescription(index);

  // ..and set it in the GUI
  this->setDescription(QString::fromStdString(description));

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetNthSceneViewScreenshotType(index);

  // ..and set it in the GUI
  this->setWidgetType((qMRMLScreenShotDialog::WidgetType)screenshotType);

  vtkImageData* imageData = this->m_Logic->GetNthSceneViewScreenshot(index);
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
  QByteArray nameBytes = name.toUtf8();
  vtkCollection* col = this->m_Logic->GetMRMLScene()->GetNodesByName(nameBytes.data());
  if (col->GetNumberOfItems() > 0)
  {
    // get a new unique name
    name = this->m_Logic->GetMRMLScene()->GetUniqueNameByString(name.toUtf8());
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
  QByteArray nameBytes = name.toUtf8();

  // description
  QString description = this->description();
  QByteArray descriptionBytes = description.toUtf8();

  // we need to know of which type the screenshot is
  int screenshotType = static_cast<int>(this->widgetType());

  int index = -1;
  if (!this->data().toString().isEmpty())
  {
    index = this->data().toInt();
  }

  if (index < 0)
  {
    // this is a new SceneView
    this->m_Logic->CreateSceneView(nameBytes.data(),descriptionBytes.data(),
                                   screenshotType,this->imageData(), true, true);
  }
  else
  {
    // this SceneView already exists

    this->m_Logic->ModifyNthSceneView(index, nameBytes.data(),descriptionBytes.data(),
      screenshotType,this->imageData());
  }
  this->Superclass::accept();
}
