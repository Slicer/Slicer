/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QStringBuilder>

// C++ includes
#include <cmath>

// SlicerQt includes
#include "qSlicerTransformsModuleWidget.h"
#include "ui_qSlicerTransformsModuleWidget.h"

// vtkSlicerLogic includes
#include "vtkSlicerTransformLogic.h"

// MRMLWidgets includes
#include <qMRMLUtils.h>

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformDisplayNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetPrivate: public Ui_qSlicerTransformsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTransformsModuleWidget);
protected:
  qSlicerTransformsModuleWidget* const q_ptr;
public:
  qSlicerTransformsModuleWidgetPrivate(qSlicerTransformsModuleWidget& object);
  static QList<vtkSmartPointer<vtkMRMLTransformableNode> > getSelectedNodes(qMRMLTreeView* tree);
  vtkSlicerTransformLogic*      logic()const;
  vtkMRMLTransformNode*         MRMLTransformNode;
  QAction*                      CopyAction;
  QAction*                      PasteAction;
};

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidgetPrivate::qSlicerTransformsModuleWidgetPrivate(qSlicerTransformsModuleWidget& object)
  : q_ptr(&object)
{
  this->MRMLTransformNode = 0;
  this->CopyAction = 0;
  this->PasteAction = 0;
}
//-----------------------------------------------------------------------------
vtkSlicerTransformLogic* qSlicerTransformsModuleWidgetPrivate::logic()const
{
  Q_Q(const qSlicerTransformsModuleWidget);
  return vtkSlicerTransformLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
QList<vtkSmartPointer<vtkMRMLTransformableNode> > qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(qMRMLTreeView* tree)
{
  QModelIndexList selectedIndexes =
    tree->selectionModel()->selectedRows();
  selectedIndexes = qMRMLTreeView::removeChildren(selectedIndexes);

  // Return the list of nodes
  QList<vtkSmartPointer<vtkMRMLTransformableNode> > selectedNodes;
  foreach(QModelIndex selectedIndex, selectedIndexes)
    {
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(
      tree->sortFilterProxyModel()->
      mrmlNodeFromIndex( selectedIndex ));
    Q_ASSERT(node);
    selectedNodes << node;
    }
  return selectedNodes;
}

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidget::qSlicerTransformsModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerTransformsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidget::~qSlicerTransformsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::setup()
{
  Q_D(qSlicerTransformsModuleWidget);
  d->setupUi(this);

  // Add coordinate reference button to a button group
  d->CopyAction = new QAction(this);
  d->CopyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  d->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->CopyAction->setShortcuts(QKeySequence::Copy);
  d->CopyAction->setToolTip(tr("Copy"));
  this->addAction(d->CopyAction);
  d->PasteAction = new QAction(this);
  d->PasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  d->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->PasteAction->setShortcuts(QKeySequence::Paste);
  d->PasteAction->setToolTip(tr("Paste"));
  this->addAction(d->PasteAction);

  // Connect button group
  this->connect(d->TranslateFirstToolButton,
                SIGNAL(toggled(bool)),
                SLOT(onTranslateFirstButtonPressed(bool)));

  // Connect identity button
  this->connect(d->IdentityPushButton,
                SIGNAL(clicked()),
                SLOT(identity()));

  // Connect invert button
  this->connect(d->InvertPushButton,
                SIGNAL(clicked()),
                SLOT(invert()));

  // Connect split button
  this->connect(d->SplitPushButton,
                SIGNAL(clicked()),
                SLOT(split()));

  // Connect node selector with module itself
  this->connect(d->TransformNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onNodeSelected(vtkMRMLNode*)));

  // Set a static min/max range to let users freely enter values
  d->MatrixWidget->setRange(-1e10, 1e10);

  // Transform nodes connection
  this->connect(d->TransformToolButton, SIGNAL(clicked()),
                SLOT(transformSelectedNodes()));
  this->connect(d->UntransformToolButton, SIGNAL(clicked()),
                SLOT(untransformSelectedNodes()));
  this->connect(d->HardenToolButton, SIGNAL(clicked()),
                SLOT(hardenSelectedNodes()));

  // Observe display section, if opened, then add display node
  this->connect(d->DisplayCollapsibleButton,
                SIGNAL(clicked(bool)),
                SLOT(onDisplaySectionClicked(bool)));

  // Observe Apply transform section to maintain a nice layout
  // even when the section is closed.
  this->connect(d->TransformedCollapsibleButton,
                SIGNAL(clicked(bool)),
                SLOT(onTransformableSectionClicked(bool)));

  // Connect copy and paste actions
  d->CopyTransformToolButton->setDefaultAction(d->CopyAction);
  this->connect(d->CopyAction,
                SIGNAL(triggered()),
                SLOT(copyTransform()));

  d->PasteTransformToolButton->setDefaultAction(d->PasteAction);
  this->connect(d->PasteAction,
                SIGNAL(triggered()),
                SLOT(pasteTransform()));

  // Icons
  QIcon rightIcon =
    QApplication::style()->standardIcon(QStyle::SP_ArrowRight);
  d->TransformToolButton->setIcon(rightIcon);

  QIcon leftIcon =
    QApplication::style()->standardIcon(QStyle::SP_ArrowLeft);
  d->UntransformToolButton->setIcon(leftIcon);

  // Connect convert button
  this->connect(d->ConvertPushButton,
    SIGNAL(clicked()),
    SLOT(convert()));

  // Connect node convert input/output node selectors
  this->connect(d->ConvertReferenceVolumeNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    SLOT(updateConvertButtonState()));
  this->connect(d->ConvertOutputDisplacementFieldNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    SLOT(updateConvertButtonState()));

  this->onNodeSelected(0);
  this->updateConvertButtonState();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onTranslateFirstButtonPressed(bool checked)
{
  Q_D(qSlicerTransformsModuleWidget);

  qMRMLTransformSliders::CoordinateReferenceType ref =
    checked ? qMRMLTransformSliders::LOCAL : qMRMLTransformSliders::GLOBAL;
  d->TranslationSliders->setCoordinateReference(ref);
  d->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(node);

  bool isLinearTransform = (transformNode!=NULL && transformNode->IsLinear());
  bool isCompositeTransform = (transformNode!=NULL && transformNode->IsComposite());

  // Enable/Disable CoordinateReference, identity, split buttons, MatrixViewGroupBox, and
  // Min/Max translation inputs

  d->InvertPushButton->setEnabled(transformNode != 0);

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  d->TranslateFirstToolButton->setVisible(isLinearTransform);
  d->MatrixViewGroupBox->setVisible(isLinearTransform);
  d->TranslationSliders->setVisible(isLinearTransform);
  d->RotationSliders->setVisible(isLinearTransform);

  d->CopyTransformToolButton->setVisible(isLinearTransform);
  d->PasteTransformToolButton->setVisible(isLinearTransform);

  d->SplitPushButton->setVisible(isCompositeTransform);

  QStringList nodeTypes;
  // If no transform node, it would show the entire scene, lets shown none
  // instead.
  if (transformNode == 0)
    {
    nodeTypes << QString("vtkMRMLNotANode");
    }
  d->TransformedTreeView->setNodeTypes(nodeTypes);

  // Filter the current node in the transformed tree view
  d->TransformedTreeView->setRootNode(transformNode);

  // Hide the current node in the transformable tree view
  QStringList hiddenNodeIDs;
  if (transformNode)
    {
    hiddenNodeIDs << QString(transformNode->GetID());
    }
  d->TransformableTreeView->sortFilterProxyModel()
    ->setHiddenNodeIDs(hiddenNodeIDs);

  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
                      vtkMRMLTransformableNode::TransformModifiedEvent,
                      this, SLOT(onMRMLTransformNodeModified(vtkObject*)));

  d->MRMLTransformNode = transformNode;

  // If there is no display node then collapse the display section.
  // This allows creation of transform display nodes on request:
  // the display node is created if the user expands the display section.
  vtkMRMLTransformDisplayNode* dispNode = NULL;
  if (transformNode)
    {
    dispNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
    }
  if (dispNode==NULL)
    {
    d->DisplayCollapsibleButton->setCollapsed(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onMRMLTransformNodeModified(vtkObject* caller)
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(caller);
  if (!transformNode)
    {
    return;
    }
  Q_ASSERT(d->MRMLTransformNode == transformNode);

  bool isLinearTransform = transformNode->IsLinear();
  bool isCompositeTransform = transformNode->IsComposite();

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  // This method may be called very frequently (when transform is changing
  // in real time). Due to some reason setVisible calls take time,
  // even if the visibility state does not change.
  // To save time, only call the set function if the visibility has to be changed.
  if (isLinearTransform!=d->TranslateFirstToolButton->isVisible())
    {
    d->TranslateFirstToolButton->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->MatrixViewGroupBox->isVisible())
    {
    d->MatrixViewGroupBox->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->TranslationSliders->isVisible())
    {
    d->TranslationSliders->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->RotationSliders->isVisible())
    {
    d->RotationSliders->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->CopyTransformToolButton->isVisible())
    {
    d->CopyTransformToolButton->setVisible(isLinearTransform);
    }
  if (isLinearTransform!=d->PasteTransformToolButton->isVisible())
    {
    d->PasteTransformToolButton->setVisible(isLinearTransform);
    }
  if (isCompositeTransform!=d->SplitPushButton->isVisible())
    {
    d->SplitPushButton->setVisible(isCompositeTransform);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::identity()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (d->MRMLTransformNode==NULL || !d->MRMLTransformNode->IsLinear())
    {
    return;
    }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
  d->MRMLTransformNode->SetMatrixTransformToParent(matrix.GetPointer());
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::invert()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (!d->MRMLTransformNode) { return; }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  d->MRMLTransformNode->Inverse();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::split()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (d->MRMLTransformNode==NULL)
    {
    return;
    }

  d->MRMLTransformNode->Split();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::copyTransform()
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkLinearTransform* linearTransform =
      vtkLinearTransform::SafeDownCast(d->MRMLTransformNode->GetTransformToParent());
  if (!linearTransform)
    {
    // Silent fail, no worries!
    qWarning() << "Unable to cast parent transform as a vtkLinearTransform";
    return;
    }

  vtkMatrix4x4* internalMatrix = linearTransform->GetMatrix();
  std::string delimiter = " ";
  std::string rowDelimiter = "\n";
  std::string output = vtkAddonMathUtilities::ToString(internalMatrix, delimiter, rowDelimiter);
  QApplication::clipboard()->setText(QString::fromStdString(output));
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::pasteTransform()
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkNew<vtkMatrix4x4> tempMatrix;

  std::string text = QApplication::clipboard()->text().toStdString();
  bool success = vtkAddonMathUtilities::FromString(tempMatrix.GetPointer(), text);
  if (!success)
    {
    qWarning() << "Cannot convert pasted string to matrix.";
    return;
    }
  d->MRMLTransformNode->SetMatrixTransformToParent(tempMatrix.GetPointer());
}

//-----------------------------------------------------------------------------
int qSlicerTransformsModuleWidget::coordinateReference()const
{
  Q_D(const qSlicerTransformsModuleWidget);
  return (d->TranslateFirstToolButton->isChecked() ? qMRMLTransformSliders::LOCAL : qMRMLTransformSliders::GLOBAL);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTransformsModuleWidget);
  this->Superclass::setMRMLScene(scene);
  // If the root index is set before the scene, it will show the scene as
  // top-level item. Setting the root index to be the scene makes the nodes
  // top-level, and this can only be done once the scene is set.
  d->TransformableTreeView->setRootIndex(
    d->TransformableTreeView->sortFilterProxyModel()->mrmlSceneIndex());
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::transformSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode> > nodesToTransform =
    qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformableTreeView);
  foreach(vtkSmartPointer<vtkMRMLTransformableNode> node, nodesToTransform)
    {
    node->SetAndObserveTransformNodeID(d->MRMLTransformNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::untransformSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode> > nodesToTransform =
    qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  foreach(vtkSmartPointer<vtkMRMLTransformableNode> node, nodesToTransform)
    {
    node->SetAndObserveTransformNodeID(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::hardenSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode> > nodesToTransform =
    qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  foreach(vtkSmartPointer<vtkMRMLTransformableNode> node, nodesToTransform)
    {
    d->logic()->hardenTransform(vtkMRMLTransformableNode::SafeDownCast(node));
    }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onDisplaySectionClicked(bool clicked)
{
  Q_D(qSlicerTransformsModuleWidget);
  // If the display section is opened and there is no display node then create one
  if (!clicked)
    {
    return;
    }
  if (d->MRMLTransformNode==NULL)
    {
    return;
    }
  if (vtkMRMLTransformDisplayNode::SafeDownCast(d->MRMLTransformNode->GetDisplayNode())==NULL)
    {
    d->MRMLTransformNode->CreateDefaultDisplayNodes();
    // Refresh the display node section
    d->TransformDisplayNodeWidget->setMRMLTransformNode(d->MRMLTransformNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onTransformableSectionClicked(bool clicked)
{
  Q_D(qSlicerTransformsModuleWidget);
  if (clicked)
    {
    // the transformable section is open, so no need for spacer
    d->BottomSpacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
  else
    {
    // the transformable section is open, add spacer to prevent stretching of
    // the remaining sections
    d->BottomSpacer->changeSize(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::convert()
{
  Q_D(qSlicerTransformsModuleWidget);
  if (d->MRMLTransformNode == NULL)
    {
    qWarning("qSlicerTransformsModuleWidget::convert failed: MRMLTransformNode is invalid");
    return;
    }
  if (d->ConvertReferenceVolumeNodeComboBox->currentNode() == NULL)
    {
    qWarning("qSlicerTransformsModuleWidget::convert failed: reference volume node is invalid");
    return;
    }
  if (d->ConvertOutputDisplacementFieldNodeComboBox->currentNode() == NULL)
    {
    qWarning("qSlicerTransformsModuleWidget::convert failed: reference volume node is invalid");
    return;
    }
  vtkMRMLVolumeNode* outputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode());
  vtkMRMLTransformNode* outputTransformNode = vtkMRMLTransformNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode());
  vtkMRMLVolumeNode* referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ConvertReferenceVolumeNodeComboBox->currentNode());
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  if (outputVolumeNode)
    {
    bool magnitudeOnly = (d->ConvertMagnitudeOnlyCheckBox->checkState() == Qt::Checked);
    d->logic()->CreateDisplacementVolumeFromTransform(d->MRMLTransformNode, referenceVolumeNode, magnitudeOnly, outputVolumeNode);
    }
  else if (outputTransformNode)
    {
    d->logic()->ConvertToGridTransform(d->MRMLTransformNode, referenceVolumeNode, outputTransformNode);
    }
  else
    {
    qWarning("qSlicerTransformsModuleWidget::convert failed: invalid output node type");
    }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::updateConvertButtonState()
{
  Q_D(qSlicerTransformsModuleWidget);
  bool enableConvert = (d->MRMLTransformNode != NULL
    && d->ConvertReferenceVolumeNodeComboBox->currentNode() != NULL
    && d->ConvertOutputDisplacementFieldNodeComboBox->currentNode() != NULL);
  d->ConvertPushButton->setEnabled(enableConvert);

  bool isVolumeOutput = (vtkMRMLVolumeNode::SafeDownCast(d->ConvertOutputDisplacementFieldNodeComboBox->currentNode()) != NULL);
  d->ConvertMagnitudeOnlyCheckBox->setEnabled(isVolumeOutput);
  if (!isVolumeOutput)
    {
    d->ConvertMagnitudeOnlyCheckBox->setChecked(false);
    }
}

//-----------------------------------------------------------
bool qSlicerTransformsModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_D(qSlicerTransformsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLTransformNode::SafeDownCast(node))
    {
    d->TransformNodeSelector->setCurrentNode(node);
    return true;
    }

  if (vtkMRMLTransformDisplayNode::SafeDownCast(node))
    {
    vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node);
    vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->TransformNodeSelector->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}
