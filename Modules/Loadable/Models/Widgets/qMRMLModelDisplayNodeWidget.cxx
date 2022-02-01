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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QColor>

// CTK includes
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLModelDisplayNodeWidget.h"
#include "ui_qMRMLModelDisplayNodeWidget.h"

// Subject hierarchy includes
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>

// MRML include
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

static const int REPRESENTATION_POINTS = 0;
static const int REPRESENTATION_WIREFRAME = 1;
static const int REPRESENTATION_SURFACE = 2;

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qMRMLModelDisplayNodeWidgetPrivate: public QWidget, public Ui_qMRMLModelDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLModelDisplayNodeWidget);

protected:
  qMRMLModelDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object);
  void init();

  bool IsUpdatingWidgetFromMRML{ false };

  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesFromSelection()const;
  QList<vtkMRMLDisplayNode*> displayNodesFromSelection()const;

  // Current display nodes, used to display the current display properties in the widget.
  // They are the first display node that belong to the first current subject hierarchy item.
  // - The model-specific display node pointer is a quick accessor to the model specific
  //   features if the first subject hierarchy item belongs to a model node
  // - The generic display node is an accessor to the generic display properties. it is
  //   needed because selection of both folders and models are supported
  vtkWeakPointer<vtkMRMLModelDisplayNode> CurrentModelDisplayNode;
  vtkWeakPointer<vtkMRMLDisplayNode> CurrentDisplayNode;

  vtkSmartPointer<vtkProperty> Property;
  QList<vtkIdType> CurrentSubjectHierarchyItemIDs;
};

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidgetPrivate::qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLModelDisplayNodeWidget);
  this->setupUi(q);

  this->MaterialPropertyWidget->setProperty(this->Property);
  q->qvtkConnect(this->Property, vtkCommand::ModifiedEvent,
    q, SLOT(updateDisplayNodesFromProperty()));

  q->connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setVisibility(bool)));
  q->connect(this->ClippingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setClipping(bool)));
  q->connect(this->ConfigureClippingPushButton, SIGNAL(clicked()),
    q, SIGNAL(clippingConfigurationButtonClicked()));
  this->ConfigureClippingPushButton->setVisible(false);

  q->connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setSliceIntersectionVisible(bool)));
  q->connect(this->SliceDisplayModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setSliceDisplayMode(int)));
  q->connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setSliceIntersectionThickness(int)));
  q->connect(this->SliceIntersectionOpacitySlider, SIGNAL(valueChanged(double)),
    q, SLOT(setSliceIntersectionOpacity(double)));
  q->connect(this->DistanceToColorNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(setDistanceToColorNode(vtkMRMLNode*)));

  q->connect(this->RepresentationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setRepresentation(int)));
  q->connect(this->PointSizeSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setPointSize(double)));
  q->connect(this->LineWidthSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setLineWidth(double)));
  q->connect(this->ShowFacesComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setShowFaces(int)));
  q->connect(this->ColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setColor(QColor)));
  q->connect(this->OpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setOpacity(double)));
  q->connect(this->EdgeColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setEdgeColor(QColor)));

  q->connect(this->BackfaceHueOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceHueOffset(double)));
  q->connect(this->BackfaceSaturationOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceSaturationOffset(double)));
  q->connect(this->BackfaceBrightnessOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceBrightnessOffset(double)));

  q->connect(this->LightingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setLighting(bool)));
  q->connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setInterpolation(int)));

  q->connect(this->ScalarsDisplayWidget, SIGNAL(scalarRangeModeValueChanged(vtkMRMLDisplayNode::ScalarRangeFlagType)),
    q, SIGNAL(scalarRangeModeValueChanged(vtkMRMLDisplayNode::ScalarRangeFlagType)));
  q->connect(this->ScalarsDisplayWidget, SIGNAL(displayNodeChanged()),
    q, SIGNAL(displayNodeChanged()));

  if (this->CurrentModelDisplayNode.GetPointer())
    {
    q->setEnabled(true);
    q->setMRMLModelDisplayNode(this->CurrentModelDisplayNode);
    }
}

//------------------------------------------------------------------------------
QList<vtkMRMLModelDisplayNode*> qMRMLModelDisplayNodeWidgetPrivate::modelDisplayNodesFromSelection()const
{
  Q_Q(const qMRMLModelDisplayNodeWidget);
  QList<vtkMRMLModelDisplayNode*> modelDisplayNodes;
  if (q->mrmlScene() == nullptr)
    {
    return modelDisplayNodes;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->mrmlScene());
  if (!shNode)
    {
    return modelDisplayNodes;
    }

  foreach (vtkIdType itemID, this->CurrentSubjectHierarchyItemIDs)
    {
    // Can be set from model or folder
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (modelNode)
      {
      // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
      vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
        modelNode->GetDisplayNode());
      if (modelDisplayNode)
        {
        modelDisplayNodes << modelDisplayNode;
        }
      }
    }
  return modelDisplayNodes;
}

//------------------------------------------------------------------------------
QList<vtkMRMLDisplayNode*> qMRMLModelDisplayNodeWidgetPrivate::displayNodesFromSelection()const
{
  Q_Q(const qMRMLModelDisplayNodeWidget);
  QList<vtkMRMLDisplayNode*> displayNodes;
  if (q->mrmlScene() == nullptr)
    {
    return displayNodes;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->mrmlScene());
  if (!shNode)
    {
    return displayNodes;
    }

  foreach (vtkIdType itemID, this->CurrentSubjectHierarchyItemIDs)
    {
    // Can be set from model or folder
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (displayNode)
      {
      displayNodes << displayNode;
      }
    else if (displayableNode)
      {
      // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
      displayNode = displayableNode->GetDisplayNode();
      if (displayNode)
        {
        displayNodes << displayNode;
        }
      }
    }
  return displayNodes;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::qMRMLModelDisplayNodeWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLModelDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLModelDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::~qMRMLModelDisplayNodeWidget() = default;

//---------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

  this->Superclass::setMRMLScene(newScene);
  this->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
    this, SLOT(updateWidgetFromMRML()));

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}

//------------------------------------------------------------------------------
vtkMRMLModelDisplayNode* qMRMLModelDisplayNodeWidget::mrmlModelDisplayNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->CurrentModelDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLModelDisplayNodeWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->CurrentDisplayNode;
}

//------------------------------------------------------------------------------
vtkIdType qMRMLModelDisplayNodeWidget::currentSubjectHierarchyItemID()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  if (d->CurrentSubjectHierarchyItemIDs.empty())
    {
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return d->CurrentSubjectHierarchyItemIDs[0];
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setCurrentSubjectHierarchyItemID(vtkIdType currentItemID)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  if ( d->CurrentSubjectHierarchyItemIDs.size() == 1
    && d->CurrentSubjectHierarchyItemIDs[0] == currentItemID )
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  d->CurrentSubjectHierarchyItemIDs.clear();
  d->CurrentSubjectHierarchyItemIDs << currentItemID;

  if (!currentItemID)
    {
    return;
    }

  vtkMRMLDisplayNode* displayNode = nullptr;

  // Can be set from model or folder
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (modelNode)
    {
    // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
    displayNode = modelNode->GetDisplayNode();
    }

  // get folder plugin (can fail if subject hierarchy logic is not instantiated)
  qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
  if (folderPlugin && folderPlugin->canOwnSubjectHierarchyItem(currentItemID) > 0.0)
    {
    displayNode = folderPlugin->createDisplayNodeForItem(currentItemID);
    }

  this->setMRMLDisplayNode(displayNode);
}

//------------------------------------------------------------------------------
QList<vtkIdType> qMRMLModelDisplayNodeWidget::currentSubjectHierarchyItemIDs()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->CurrentSubjectHierarchyItemIDs;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setCurrentSubjectHierarchyItemIDs(QList<vtkIdType> currentItemIDs)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  // Set first item as current item (that the widget displays)
  if (currentItemIDs.size() > 0)
    {
    this->setCurrentSubjectHierarchyItemID(currentItemIDs[0]);
    }

  d->CurrentSubjectHierarchyItemIDs = currentItemIDs;

  // Set display nodes to scalars display widget
  d->ScalarsDisplayWidget->setMRMLDisplayNodes(d->displayNodesFromSelection());
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLModelDisplayNode(vtkMRMLModelDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode)
{
  this->setMRMLDisplayNode(modelDisplayNode);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (d->CurrentDisplayNode == displayNode)
    {
    return;
    }

  qvtkReconnect(d->CurrentDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->CurrentDisplayNode = displayNode;
  d->CurrentModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);

  // Set display node to scalars display widget
  d->ScalarsDisplayWidget->setMRMLDisplayNode(displayNode);

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLModelDisplayNodeWidget);
  this->setEnabled(d->CurrentDisplayNode.GetPointer() != nullptr);
  if (!d->CurrentDisplayNode.GetPointer())
    {
    emit displayNodeChanged();
    return;
    }

  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  d->IsUpdatingWidgetFromMRML = true;

  d->VisibilityCheckBox->setChecked(d->CurrentDisplayNode->GetVisibility());
  d->DisplayNodeViewComboBox->setMRMLDisplayNode(d->CurrentDisplayNode);
  d->ClippingCheckBox->setChecked(d->CurrentDisplayNode->GetClipping());
  d->SliceIntersectionVisibilityCheckBox->setChecked(d->CurrentDisplayNode->GetVisibility2D());
  d->SliceIntersectionThicknessSpinBox->setValue(d->CurrentDisplayNode->GetSliceIntersectionThickness());
  bool showSliceIntersectionThickness =
    (d->CurrentModelDisplayNode ? d->CurrentModelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayIntersection : true);
  d->SliceIntersectionThicknessSpinBox->setEnabled(showSliceIntersectionThickness);
  d->SliceIntersectionOpacitySlider->setValue(d->CurrentDisplayNode->GetSliceIntersectionOpacity());

  d->SliceDisplayModeComboBox->setEnabled(d->CurrentModelDisplayNode);
  if (d->CurrentModelDisplayNode)
    {
    d->SliceDisplayModeComboBox->setCurrentIndex(d->CurrentModelDisplayNode->GetSliceDisplayMode());
    }

  bool wasBlocking = d->DistanceToColorNodeComboBox->blockSignals(true);
  if (d->DistanceToColorNodeComboBox->mrmlScene() != this->mrmlScene())
    {
    d->DistanceToColorNodeComboBox->setMRMLScene(this->mrmlScene());
    }
  if ( d->CurrentModelDisplayNode
    && d->DistanceToColorNodeComboBox->currentNodeID() != d->CurrentModelDisplayNode->GetDistanceEncodedProjectionColorNodeID() )
    {
    d->DistanceToColorNodeComboBox->setCurrentNodeID(d->CurrentModelDisplayNode->GetDistanceEncodedProjectionColorNodeID());
    }
  d->DistanceToColorNodeComboBox->setEnabled( d->CurrentModelDisplayNode &&
    d->CurrentModelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection );
  d->DistanceToColorNodeComboBox->blockSignals(wasBlocking);

  // Representation
  switch (d->CurrentDisplayNode->GetRepresentation())
    {
    case REPRESENTATION_POINTS:
      d->RepresentationComboBox->setCurrentIndex(0);
      break;
    case REPRESENTATION_WIREFRAME:
      d->RepresentationComboBox->setCurrentIndex(1);
      break;
    case REPRESENTATION_SURFACE:
      if (d->CurrentDisplayNode->GetEdgeVisibility())
        {
        d->RepresentationComboBox->setCurrentIndex(3);
        }
      else
        {
        d->RepresentationComboBox->setCurrentIndex(2);
        }
      break;
    }

  d->PointSizeSliderWidget->setValue(d->CurrentDisplayNode->GetPointSize());
  bool showPointSize = d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_POINTS;
  d->PointSizeSliderWidget->setEnabled(showPointSize);

  d->LineWidthSliderWidget->setValue(d->CurrentDisplayNode->GetLineWidth());
  // Enable line width editing in REPRESENTATION_SURFACE mode regardless of edge visibility,
  // because if the model consists only of lines then line width will make a difference
  // even if edge visibility is disabled.
  bool showLineWidth = (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_WIREFRAME
    || d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_SURFACE);
  d->LineWidthSliderWidget->setEnabled(showLineWidth);

  if (!d->CurrentDisplayNode->GetFrontfaceCulling() && d->CurrentDisplayNode->GetBackfaceCulling())
    {
    // show only front faces
    d->ShowFacesComboBox->setCurrentIndex(1);
    }
  else if (d->CurrentDisplayNode->GetFrontfaceCulling() && !d->CurrentDisplayNode->GetBackfaceCulling())
    {
    // show only back faces
    d->ShowFacesComboBox->setCurrentIndex(2);
    }
  else
    {
    // show all faces
    d->ShowFacesComboBox->setCurrentIndex(0);
    }

  double* c = d->CurrentDisplayNode->GetColor();
  bool wasBlocked = d->ColorPickerButton->blockSignals(true);
  d->ColorPickerButton->setColor(QColor::fromRgbF(qMin(c[0], 1.), qMin(c[1], 1.), qMin(c[2], 1.)));
  d->ColorPickerButton->blockSignals(wasBlocked);

  d->OpacitySliderWidget->setValue(d->CurrentDisplayNode->GetOpacity());
  double* ec = d->CurrentDisplayNode->GetEdgeColor();
  d->EdgeColorPickerButton->setColor(
    QColor::fromRgbF(qMin(ec[0], 1.), qMin(ec[1], 1.), qMin(ec[2], 1.)));
  bool showEdgeColor =
    (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_SURFACE && d->CurrentDisplayNode->GetEdgeVisibility());
  d->EdgeColorPickerButton->setEnabled(showEdgeColor);

  if (d->CurrentModelDisplayNode)
    {
    double hsvOffset[3];
    d->CurrentModelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    QSignalBlocker blocker1(d->BackfaceHueOffsetSpinBox);
    QSignalBlocker blocker2(d->BackfaceSaturationOffsetSpinBox);
    QSignalBlocker blocker3(d->BackfaceSaturationOffsetSpinBox);
    d->BackfaceHueOffsetSpinBox->setValue(hsvOffset[0]);
    d->BackfaceSaturationOffsetSpinBox->setValue(hsvOffset[1]);
    d->BackfaceBrightnessOffsetSpinBox->setValue(hsvOffset[2]);
    }
  d->BackfaceHueOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);
  d->BackfaceSaturationOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);
  d->BackfaceBrightnessOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);

  d->LightingCheckBox->setChecked(d->CurrentDisplayNode->GetLighting());
  d->InterpolationComboBox->setCurrentIndex(d->CurrentDisplayNode->GetInterpolation());

  // Material
  d->Property->SetAmbient(d->CurrentDisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->CurrentDisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->CurrentDisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->CurrentDisplayNode->GetPower());
  d->Property->SetMetallic(d->CurrentDisplayNode->GetMetallic());
  d->Property->SetRoughness(d->CurrentDisplayNode->GetRoughness());

  // Scalars
  d->ScalarsDisplayWidget->updateWidgetFromMRML();

  d->IsUpdatingWidgetFromMRML = false;

  emit displayNodeChanged();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetVisibility(visible);
    }
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::visibility()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setClipping(bool clip)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetClipping(clip);
    }
  emit clippingToggled(clip);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::clipping()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ClippingCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetVisibility2D(visible);
    }
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionThickness(int thickness)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetSliceIntersectionThickness(thickness);
    }
}

//------------------------------------------------------------------------------
int qMRMLModelDisplayNodeWidget::sliceIntersectionThickness()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionThicknessSpinBox->value();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionOpacity(double opacity)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetSliceIntersectionOpacity(opacity);
    }
}

//------------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::sliceIntersectionOpacity()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionOpacitySlider->value();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateDisplayNodesFromProperty()
{
  Q_D(qMRMLModelDisplayNodeWidget);

  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    int wasModifying = displayNode->StartModify();
    // Lighting
    displayNode->SetLighting(d->Property->GetLighting());
    displayNode->SetInterpolation(d->Property->GetInterpolation());
    displayNode->SetShading(d->Property->GetShading());
    // Material
    displayNode->SetAmbient(d->Property->GetAmbient());
    displayNode->SetDiffuse(d->Property->GetDiffuse());
    displayNode->SetSpecular(d->Property->GetSpecular());
    displayNode->SetPower(d->Property->GetSpecularPower());
    displayNode->SetMetallic(d->Property->GetMetallic());
    displayNode->SetRoughness(d->Property->GetRoughness());
    displayNode->EndModify(wasModifying);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setRepresentation(int newRepresentation)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    switch (newRepresentation)
      {
      case 0: // points
        displayNode->SetRepresentation(REPRESENTATION_POINTS);
        break;
      case 1: // wireframe
        displayNode->SetRepresentation(REPRESENTATION_WIREFRAME);
        break;
      case 2: // surface
      case 3: // surface with edges
        {
        int wasModified = displayNode->StartModify();
        displayNode->SetRepresentation(REPRESENTATION_SURFACE);
        displayNode->SetEdgeVisibility(newRepresentation == 3);
        displayNode->EndModify(wasModified);
        break;
        }
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceDisplayMode(int newMode)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkMRMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    int wasModified = modelDisplayNode->StartModify();
    // Select a color node if none is selected yet
    if (modelDisplayNode->GetSliceDisplayMode()
      != vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
      && newMode == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
      && modelDisplayNode->GetDistanceEncodedProjectionColorNodeID() == nullptr)
      {
      modelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID("vtkMRMLProceduralColorNodeRedGreenBlue");
      }
    modelDisplayNode->SetSliceDisplayMode(newMode);
    modelDisplayNode->EndModify(wasModified);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setPointSize(double newPointSize)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetPointSize(newPointSize);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setLineWidth(double newLineWidth)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetLineWidth(newLineWidth);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setBackfaceHueOffset(double newOffset)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(newOffset, hsvOffset[1], hsvOffset[2]);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setBackfaceSaturationOffset(double newOffset)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(hsvOffset[0], newOffset, hsvOffset[2]);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setBackfaceBrightnessOffset(double newOffset)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(hsvOffset[0], hsvOffset[1], newOffset);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setShowFaces(int newShowFaces)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    int wasModified = displayNode->StartModify();
    switch (newShowFaces)
      {
      case 0: // show all
        displayNode->SetFrontfaceCulling(false);
        displayNode->SetBackfaceCulling(false);
        break;
      case 1: // show front only
        displayNode->SetFrontfaceCulling(false);
        displayNode->SetBackfaceCulling(true);
        break;
      case 2: // show back only
        displayNode->SetFrontfaceCulling(true);
        displayNode->SetBackfaceCulling(false);
        break;
      }
    displayNode->EndModify(wasModified);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setColor(const QColor& newColor)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    double* oldColorArray = displayNode->GetColor();
    QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
    if (oldColor != newColor)
      {
      displayNode->SetColor(newColor.redF(), newColor.greenF(), newColor.blueF());
      // Solid color is set, therefore disable scalar visibility
      // (otherwise color would come from the scalar value and colormap).
      displayNode->SetScalarVisibility(false);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setOpacity(double newOpacity)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetOpacity(newOpacity);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setEdgeVisibility(bool newEdgeVisibility)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetEdgeVisibility(newEdgeVisibility);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setEdgeColor(const QColor& newColor)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetEdgeColor(newColor.redF(), newColor.greenF(), newColor.blueF());
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setLighting(bool newLighting)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetLighting(newLighting);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setInterpolation(int newInterpolation)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
    {
    return;
    }
  switch (newInterpolation)
    {
    case vtkMRMLDisplayNode::FlatInterpolation: d->Property->SetInterpolationToFlat(); break;
    case vtkMRMLDisplayNode::GouraudInterpolation: d->Property->SetInterpolationToGouraud(); break;
    case vtkMRMLDisplayNode::PhongInterpolation: d->Property->SetInterpolationToPhong(); break;
    case vtkMRMLDisplayNode::PBRInterpolation: d->Property->SetInterpolationToPBR(); break;
    default:
      qWarning() << Q_FUNC_INFO << " failed: invalid interpolation mode " << newInterpolation;
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setDistanceToColorNode(vtkMRMLNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->CurrentModelDisplayNode.GetPointer())
    {
    return;
    }
  d->CurrentModelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID(colorNode ? colorNode->GetID() : nullptr);
}

// --------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::clippingConfigurationButtonVisible()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ConfigureClippingPushButton->isVisible();
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setClippingConfigurationButtonVisible(bool show)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  d->ConfigureClippingPushButton->setVisible(show);
}
