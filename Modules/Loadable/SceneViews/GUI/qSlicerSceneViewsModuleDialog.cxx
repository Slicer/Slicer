// Qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
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
class qSlicerSceneViewsModuleDialogPrivate
{
  Q_DECLARE_PUBLIC(qSlicerSceneViewsModuleDialog);

protected:
  qSlicerSceneViewsModuleDialog* const q_ptr;

public:
  qSlicerSceneViewsModuleDialogPrivate(qSlicerSceneViewsModuleDialog& object);

  QCheckBox* UpdateExistingNodesCheckBox{ nullptr };
  QCheckBox* CaptureDisplayNodesCheckBox{ nullptr };
  QCheckBox* CaptureViewNodesCheckBox{ nullptr };

  void setupUi(QDialog* dialog);
};

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialogPrivate::qSlicerSceneViewsModuleDialogPrivate(qSlicerSceneViewsModuleDialog& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::setupUi(QDialog* dialog)
{
  Q_Q(qSlicerSceneViewsModuleDialog);

  QGridLayout* gridLayout = qobject_cast<QGridLayout*>(dialog->layout());

  QWidget* buttonBox = dialog->findChild<QWidget*>("buttonBox");
  int index = gridLayout->indexOf(buttonBox);
  int newRowIndex = -1;
  int column = -1;
  int rowSpan = -1;
  int columnSpan = -1;
  gridLayout->getItemPosition(index, &newRowIndex, &column, &rowSpan, &columnSpan);
  gridLayout->removeWidget(buttonBox);

  this->UpdateExistingNodesCheckBox = new QCheckBox(dialog);
  this->UpdateExistingNodesCheckBox->setObjectName(QString::fromUtf8("UpdateExistingNodesCheckBox"));
  this->UpdateExistingNodesCheckBox->setText(qSlicerSceneViewsModuleDialog::tr("Update existing nodes"));
  this->UpdateExistingNodesCheckBox->setToolTip(
    qSlicerSceneViewsModuleDialog::tr("If checked, the nodes already contained in the scene view will be updated to match the current state of the scene."));

  gridLayout->addWidget(this->UpdateExistingNodesCheckBox, newRowIndex++, 0, 1, 2);

  QWidget* captureNodeTypesWidget = new QWidget(dialog);
  captureNodeTypesWidget->setObjectName(QString::fromUtf8("updateNodeTypesWidget"));
  QHBoxLayout* captureNodeTypesLayout = new QHBoxLayout(captureNodeTypesWidget);
  captureNodeTypesLayout->setObjectName(QString::fromUtf8("captureNodeTypesLayout"));
  captureNodeTypesLayout->setContentsMargins(0, 0, 0, 0);
  captureNodeTypesLayout->setSpacing(6);
  captureNodeTypesWidget->setLayout(captureNodeTypesLayout);

  this->CaptureDisplayNodesCheckBox = new QCheckBox(captureNodeTypesWidget);
  this->CaptureDisplayNodesCheckBox->setObjectName(QString::fromUtf8("CaptureDisplayNodesCheckBox"));
  this->CaptureDisplayNodesCheckBox->setText(qSlicerSceneViewsModuleDialog::tr("Capture display nodes"));
  this->CaptureDisplayNodesCheckBox->setToolTip(
    qSlicerSceneViewsModuleDialog::tr("If checked, all display nodes in the scene will be added or updated in the current scene view."));
  captureNodeTypesLayout->addWidget(this->CaptureDisplayNodesCheckBox);

  this->CaptureViewNodesCheckBox = new QCheckBox(captureNodeTypesWidget);
  this->CaptureViewNodesCheckBox->setObjectName(QString::fromUtf8("CaptureViewNodesCheckBox"));
  this->CaptureViewNodesCheckBox->setText(qSlicerSceneViewsModuleDialog::tr("Capture view nodes"));
  this->CaptureViewNodesCheckBox->setToolTip(qSlicerSceneViewsModuleDialog::tr("If checked, all view nodes in the scene will be added or updated in the current scene view."));
  captureNodeTypesLayout->addWidget(this->CaptureViewNodesCheckBox);

  gridLayout->addWidget(captureNodeTypesWidget, newRowIndex++, 0, 1, 2);

  gridLayout->addWidget(buttonBox, newRowIndex, 0, 1, 2);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::qSlicerSceneViewsModuleDialog(QWidget* parent /*=nullptr*/)
  : qMRMLScreenShotDialog(parent)
  , d_ptr(new qSlicerSceneViewsModuleDialogPrivate(*this))
{
  this->m_Logic = nullptr;
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());
  this->setShowScaleFactorSpinBox(false);
  this->setWindowTitle(tr("3D Slicer SceneView"));

  // default name
  QString name("SceneView");
  this->setNameEdit(name);

  Q_D(qSlicerSceneViewsModuleDialog);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::~qSlicerSceneViewsModuleDialog()
{

  if (this->m_Logic)
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
  Q_D(qSlicerSceneViewsModuleDialog);

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

  d->UpdateExistingNodesCheckBox->setVisible(true);
  d->UpdateExistingNodesCheckBox->setChecked(false);
  d->CaptureDisplayNodesCheckBox->setChecked(false);
  d->CaptureViewNodesCheckBox->setChecked(false);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::reset()
{
  Q_D(qSlicerSceneViewsModuleDialog);

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

  d->UpdateExistingNodesCheckBox->setVisible(false);
  d->UpdateExistingNodesCheckBox->setChecked(false);
  d->CaptureDisplayNodesCheckBox->setChecked(true);
  d->CaptureViewNodesCheckBox->setChecked(true);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::accept()
{
  Q_D(qSlicerSceneViewsModuleDialog);

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
    this->m_Logic->CreateSceneView(
      nameBytes.data(), descriptionBytes.data(), screenshotType, this->imageData(), d->CaptureDisplayNodesCheckBox->isChecked(), d->CaptureViewNodesCheckBox->isChecked());
  }
  else
  {
    if (d->UpdateExistingNodesCheckBox->isChecked()    //
        || d->CaptureDisplayNodesCheckBox->isChecked() //
        || d->CaptureViewNodesCheckBox->isChecked())
    {
      // update the nodes saved in the scene view
      this->m_Logic->UpdateNthSceneView(index, d->UpdateExistingNodesCheckBox->isChecked(), d->CaptureDisplayNodesCheckBox->isChecked(), d->CaptureViewNodesCheckBox->isChecked());
    }

    // update the name and description
    this->m_Logic->ModifyNthSceneView(index, nameBytes.data(), descriptionBytes.data(), screenshotType, this->imageData());
  }
  this->Superclass::accept();
}
