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

// MRMLWidgets includes
#include "qMRMLCheckableNodeComboBox.h"

// Sequences MRML includes
#include <vtkMRMLSequenceBrowserNode.h>

// VTK includes
#include <vtkImageData.h>

// CTK includes
#include <ctkCollapsibleGroupBox.h>

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

  ctkCollapsibleGroupBox* AdvancedNodeSelectionGroupBox{ nullptr };
  qMRMLCheckableNodeComboBox* NodeSelectorComboBox{ nullptr };

  void setupUi(QDialog* dialog);
  void updateCategorySelection();
  void updateNodeSelection();
  void updateDisplayNodesSelection(Qt::CheckState checkState);
  void updateViewNodesSelection(Qt::CheckState checkState);
  void updateExistingNodesSelection(Qt::CheckState checkState);
  void updateNodeSelection(std::vector<vtkMRMLNode*> nodes, Qt::CheckState checkState);
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
  QObject::connect(this->UpdateExistingNodesCheckBox, SIGNAL(clicked()), dialog, SLOT(onUpdateExistingNodesClicked()));
  gridLayout->addWidget(this->UpdateExistingNodesCheckBox, newRowIndex++, 0, 1, 2);

  this->CaptureDisplayNodesCheckBox = new QCheckBox(dialog);
  this->CaptureDisplayNodesCheckBox->setObjectName(QString::fromUtf8("CaptureDisplayNodesCheckBox"));
  this->CaptureDisplayNodesCheckBox->setText(qSlicerSceneViewsModuleDialog::tr("Capture display nodes"));
  this->CaptureDisplayNodesCheckBox->setToolTip(
    qSlicerSceneViewsModuleDialog::tr("If checked, all display nodes in the scene will be added or updated in the current scene view."));
  QObject::connect(this->CaptureDisplayNodesCheckBox, SIGNAL(clicked()), dialog, SLOT(onCaptureDisplayNodesClicked()));
  gridLayout->addWidget(this->CaptureDisplayNodesCheckBox, newRowIndex++, 0, 1, 2);

  this->CaptureViewNodesCheckBox = new QCheckBox(dialog);
  this->CaptureViewNodesCheckBox->setObjectName(QString::fromUtf8("CaptureViewNodesCheckBox"));
  this->CaptureViewNodesCheckBox->setText(qSlicerSceneViewsModuleDialog::tr("Capture view nodes"));
  this->CaptureViewNodesCheckBox->setToolTip(qSlicerSceneViewsModuleDialog::tr("If checked, all view nodes in the scene will be added or updated in the current scene view."));
  QObject::connect(this->CaptureViewNodesCheckBox, SIGNAL(clicked()), dialog, SLOT(onCaptureViewNodesClicked()));
  gridLayout->addWidget(this->CaptureViewNodesCheckBox, newRowIndex++, 0, 1, 2);

  this->AdvancedNodeSelectionGroupBox = new ctkCollapsibleGroupBox(dialog);
  this->AdvancedNodeSelectionGroupBox->setObjectName(QString::fromUtf8("AdvancedNodeSelectionGroupBox"));
  this->AdvancedNodeSelectionGroupBox->setTitle(qSlicerSceneViewsModuleDialog::tr("Advanced"));
  this->AdvancedNodeSelectionGroupBox->setToolTip(qSlicerSceneViewsModuleDialog::tr("Select the nodes to be captured in the scene view."));
  this->AdvancedNodeSelectionGroupBox->setCollapsed(true);
  this->AdvancedNodeSelectionGroupBox->setLayout(new QGridLayout(this->AdvancedNodeSelectionGroupBox));
  gridLayout->addWidget(this->AdvancedNodeSelectionGroupBox, newRowIndex++, 0, 1, 2);

  QGridLayout* advancedNodeSelectionLayout = qobject_cast<QGridLayout*>(this->AdvancedNodeSelectionGroupBox->layout());

  this->NodeSelectorComboBox = new qMRMLCheckableNodeComboBox(dialog);
  this->NodeSelectorComboBox->setObjectName(QString::fromUtf8("NodeSelectorComboBox"));
  this->NodeSelectorComboBox->setToolTip(qSlicerSceneViewsModuleDialog::tr("Select the nodes to be captured in the scene view."));
  this->NodeSelectorComboBox->setShowHidden(true);
  this->NodeSelectorComboBox->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(this->NodeSelectorComboBox, SIGNAL(checkedNodesChanged()), dialog, SLOT(onNodeSelectionChanged()));
  advancedNodeSelectionLayout->addWidget(new QLabel("Nodes to capture:"), 0, 0, 1, 1);
  advancedNodeSelectionLayout->addWidget(this->NodeSelectorComboBox, 0, 1, 1, 1);

  gridLayout->addWidget(buttonBox, newRowIndex++, 0, 1, 2);
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
  Q_D(qSlicerSceneViewsModuleDialog);
  if (!logic)
  {
    qErrnoWarning("setLogic: We need the SceneViews module logic here!");
    return;
  }
  this->m_Logic = logic;
  d->updateNodeSelection();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateCategorySelection()
{
  Q_Q(qSlicerSceneViewsModuleDialog);

  std::vector<std::string> displayNodeClasses;
  q->m_Logic->GetDisplayNodeClasses(displayNodeClasses);

  std::vector<std::string> viewNodeClasses;
  q->m_Logic->GetViewNodeClasses(viewNodeClasses);

  bool areAllDisplayNodesChecked = true;
  bool areAllDisplayNodesUnchecked = true;

  bool areAllViewNodesChecked = true;
  bool areAllViewNodesUnchecked = true;

  bool areAllUpdateExistingNodesChecked = true;
  bool areAllUpdateExistingNodesUnchecked = true;

  std::vector<vtkMRMLNode*> existingNodes;
  int index = q->data().toInt();
  if (index >= 0)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowser = q->m_Logic->GetNthSceneViewSequenceBrowserNode(index);
    if (sequenceBrowser)
    {
      sequenceBrowser->GetAllProxyNodes(existingNodes);
    }
  }

  for (auto node : this->NodeSelectorComboBox->nodes())
  {

    // Determine display node checkbox state
    bool isDisplayNode = false;
    for (const auto& displayNodeType : displayNodeClasses)
    {
      if (node->IsA(displayNodeType.c_str()))
      {
        isDisplayNode = true;
        break;
      }
    }
    if (isDisplayNode)
    {
      if (this->NodeSelectorComboBox->checkState(node) == Qt::Checked)
      {
        areAllDisplayNodesUnchecked = false;
      }
      else
      {
        areAllDisplayNodesChecked = false;
      }
    }

    // Determine view node checkbox state
    bool isViewNode = false;
    for (const auto& viewNodeType : viewNodeClasses)
    {
      if (node->IsA(viewNodeType.c_str()))
      {
        isViewNode = true;
        break;
      }
    }
    if (isViewNode)
    {
      if (this->NodeSelectorComboBox->checkState(node) == Qt::Checked)
      {
        areAllViewNodesUnchecked = false;
      }
      else
      {
        areAllViewNodesChecked = false;
      }
    }

    // Determine update existing nodes checkbox state
    if (std::find(existingNodes.begin(), existingNodes.end(), node) != existingNodes.end())
    {
      if (this->NodeSelectorComboBox->checkState(node) == Qt::Checked)
      {
        areAllUpdateExistingNodesUnchecked = false;
      }
      else
      {
        areAllUpdateExistingNodesChecked = false;
      }
    }
  }

  Qt::CheckState displayCheckState = Qt::PartiallyChecked;
  if (areAllDisplayNodesChecked)
  {
    displayCheckState = Qt::Checked;
  }
  else if (areAllDisplayNodesUnchecked)
  {
    displayCheckState = Qt::Unchecked;
  }
  this->CaptureDisplayNodesCheckBox->setTristate(areAllDisplayNodesChecked || areAllDisplayNodesUnchecked);
  this->CaptureDisplayNodesCheckBox->setCheckState(displayCheckState);

  Qt::CheckState viewCheckState = Qt::PartiallyChecked;
  if (areAllViewNodesChecked)
  {
    viewCheckState = Qt::Checked;
  }
  else if (areAllViewNodesUnchecked)
  {
    viewCheckState = Qt::Unchecked;
  }
  this->CaptureViewNodesCheckBox->setTristate(areAllViewNodesChecked || areAllViewNodesUnchecked);
  this->CaptureViewNodesCheckBox->setCheckState(viewCheckState);

  Qt::CheckState updateExistingNodesCheckState = Qt::PartiallyChecked;
  if (areAllUpdateExistingNodesChecked)
  {
    updateExistingNodesCheckState = Qt::Checked;
  }
  else if (areAllUpdateExistingNodesUnchecked)
  {
    updateExistingNodesCheckState = Qt::Unchecked;
  }
  this->UpdateExistingNodesCheckBox->setTristate(areAllUpdateExistingNodesChecked || areAllUpdateExistingNodesUnchecked);
  this->UpdateExistingNodesCheckBox->setCheckState(updateExistingNodesCheckState);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onUpdateExistingNodesClicked()
{
  Q_D(qSlicerSceneViewsModuleDialog);
  d->UpdateExistingNodesCheckBox->setTristate(false);
  d->updateExistingNodesSelection(d->UpdateExistingNodesCheckBox->checkState());
  d->updateCategorySelection();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onCaptureDisplayNodesClicked()
{
  Q_D(qSlicerSceneViewsModuleDialog);
  d->CaptureDisplayNodesCheckBox->setTristate(false);
  d->updateDisplayNodesSelection(d->CaptureDisplayNodesCheckBox->checkState());
  d->updateCategorySelection();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onCaptureViewNodesClicked()
{
  Q_D(qSlicerSceneViewsModuleDialog);
  d->CaptureViewNodesCheckBox->setTristate(false);
  d->updateViewNodesSelection(d->CaptureViewNodesCheckBox->checkState());
  d->updateCategorySelection();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onNodeSelectionChanged()
{
  Q_D(qSlicerSceneViewsModuleDialog);
  d->updateCategorySelection();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateDisplayNodesSelection(Qt::CheckState checkState)
{
  Q_Q(qSlicerSceneViewsModuleDialog);
  if (!q->m_Logic)
  {
    return;
  }
  std::vector<vtkMRMLNode*> displayNodes;
  q->m_Logic->GetDisplayNodes(displayNodes);
  this->updateNodeSelection(displayNodes, checkState);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateViewNodesSelection(Qt::CheckState checkState)
{
  Q_Q(qSlicerSceneViewsModuleDialog);
  if (!q->m_Logic)
  {
    return;
  }
  std::vector<vtkMRMLNode*> viewNodes;
  q->m_Logic->GetViewNodes(viewNodes);
  this->updateNodeSelection(viewNodes, checkState);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateExistingNodesSelection(Qt::CheckState checkState)
{
  Q_Q(qSlicerSceneViewsModuleDialog);
  if (!q->m_Logic)
  {
    return;
  }
  int index = q->data().toInt();
  std::vector<vtkMRMLNode*> existingNodes;
  if (index >= 0)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowser = q->m_Logic->GetNthSceneViewSequenceBrowserNode(index);
    if (sequenceBrowser)
    {
      sequenceBrowser->GetAllProxyNodes(existingNodes);
    }
  }
  this->updateNodeSelection(existingNodes, checkState);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateNodeSelection(std::vector<vtkMRMLNode*> nodes, Qt::CheckState checkState)
{
  Q_Q(qSlicerSceneViewsModuleDialog);
  if (!q->m_Logic)
  {
    return;
  }

  if (checkState == Qt::PartiallyChecked)
  {
    // If the check state is partially checked, we do not change the check state of the nodes.
    return;
  }

  bool wasBlocking = this->NodeSelectorComboBox->blockSignals(true);
  for (auto node : nodes)
  {
    this->NodeSelectorComboBox->setCheckState(node, checkState);
  }
  this->NodeSelectorComboBox->blockSignals(wasBlocking);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialogPrivate::updateNodeSelection()
{
  Q_Q(qSlicerSceneViewsModuleDialog);
  if (!q->m_Logic)
  {
    return;
  }

  bool wasBlocking = this->NodeSelectorComboBox->blockSignals(true);
  this->updateViewNodesSelection(this->CaptureViewNodesCheckBox->checkState());
  this->updateDisplayNodesSelection(this->CaptureDisplayNodesCheckBox->checkState());
  this->updateExistingNodesSelection(this->UpdateExistingNodesCheckBox->checkState());
  this->NodeSelectorComboBox->blockSignals(wasBlocking);
  this->updateCategorySelection();
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

  std::string name = this->m_Logic->GetNthSceneViewName(index);
  this->setNameEdit(QString::fromStdString(name));

  std::string description = this->m_Logic->GetNthSceneViewDescription(index);
  this->setDescription(QString::fromStdString(description));

  int screenshotType = this->m_Logic->GetNthSceneViewScreenshotType(index);
  this->setWidgetType((qMRMLScreenShotDialog::WidgetType)screenshotType);

  vtkImageData* imageData = this->m_Logic->GetNthSceneViewScreenshot(index);
  this->setImageData(imageData);

  d->UpdateExistingNodesCheckBox->setVisible(true);
  d->UpdateExistingNodesCheckBox->setChecked(false);
  d->CaptureDisplayNodesCheckBox->setChecked(false);
  d->CaptureViewNodesCheckBox->setChecked(false);
  d->updateNodeSelection();
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

  d->updateNodeSelection();
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

  QList<vtkMRMLNode*> selectedNodes = d->NodeSelectorComboBox->checkedNodes();
  std::vector<vtkMRMLNode*> selectedNodesVector = std::vector<vtkMRMLNode*>(selectedNodes.begin(), selectedNodes.end());

  if (index < 0)
  {
    // this is a new SceneView
    this->m_Logic->CreateSceneView(selectedNodesVector, nameBytes.data(), descriptionBytes.data(), screenshotType, this->imageData());
  }
  else
  {
    if (d->UpdateExistingNodesCheckBox->isChecked()    //
        || d->CaptureDisplayNodesCheckBox->isChecked() //
        || d->CaptureViewNodesCheckBox->isChecked())
    {
      // update the nodes saved in the scene view
      this->m_Logic->UpdateNthSceneView(index, selectedNodesVector, d->UpdateExistingNodesCheckBox->isChecked());
    }

    // update the name and description
    this->m_Logic->ModifyNthSceneView(index, nameBytes.data(), descriptionBytes.data(), screenshotType, this->imageData());
  }
  this->Superclass::accept();
}
