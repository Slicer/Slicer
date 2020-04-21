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
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
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
  // Store what data range was used to automatically slider range of display range,
  // to prevent resetting slider range when user moves the slider.
  double DataRangeUsedForAutoDisplayRange[2];
};

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidgetPrivate::qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
  this->DataRangeUsedForAutoDisplayRange[0] = 0.0;
  this->DataRangeUsedForAutoDisplayRange[1] = 0.0;
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

  q->connect(this->LightingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setLighting(bool)));
  q->connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setInterpolation(int)));

  // Without this infinite loop of widget update/VTK data set update occurs when updating a VTK data set
  // that is generated by an algorithm that  temporarily removes all arrays from its output temporarily
  // (for example the vtkGlyph3D filter behaves like this).
  // The root cause of the problem is that if none option is not enabled then the combobox
  // automatically selects the first array, which triggers a data set change, which removes all arrays,
  // which triggers a widget update, etc. - until stack overflow occurs.
  //this->ActiveScalarComboBox->setNoneEnabled(true);

  // Scalar
  QObject::connect(this->ScalarsVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setScalarsVisibility(bool)));
  QObject::connect(this->ActiveScalarComboBox, SIGNAL(currentArrayChanged(QString)),
    q, SLOT(setActiveScalarName(QString)));
  QObject::connect(this->ScalarsColorNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setScalarsColorNode(vtkMRMLNode*)));
  // scalar range
  QObject::connect(this->DisplayedScalarRangeModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setScalarRangeMode(int)));
  QObject::connect(this->DisplayedScalarRangeWidget, SIGNAL(valuesChanged(double,double)),
    q, SLOT(setScalarsDisplayRange(double,double)));

  // Thresholding
  this->ThresholdCheckBox->setChecked(false);
  this->ThresholdRangeWidget->setEnabled(false);
  QObject::connect(this->ThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setTresholdEnabled(bool)));
  QObject::connect(this->ThresholdRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(setThresholdRange(double,double)));

  // update range mode
  q->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Data); // former auto

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

  qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
  if (folderPlugin->canOwnSubjectHierarchyItem(currentItemID) > 0.0)
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

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsVisibility(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetScalarVisibility(visible);
    }
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::scalarsVisibility()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ScalarsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setActiveScalarName(const QString& arrayName)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkMRMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    int wasModified = modelDisplayNode->StartModify();
    modelDisplayNode->SetActiveScalar(arrayName.toUtf8(), d->ActiveScalarComboBox->currentArrayLocation());

    // if there's no color node set for a non empty array name, use a default
    if (!arrayName.isEmpty() && modelDisplayNode->GetColorNodeID() == nullptr)
      {
      const char* colorNodeID = "vtkMRMLColorTableNodeFileViridis.txt";
      modelDisplayNode->SetAndObserveColorNodeID(colorNodeID);
      }
    modelDisplayNode->EndModify(wasModified);
    }
}

//------------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidget::activeScalarName()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  // TODO: use currentArrayName()
  vtkAbstractArray* array = d->ActiveScalarComboBox->currentArray();
  return array ? array->GetName() : "";
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLNode* colorNode)
{
  this->setScalarsColorNode(vtkMRMLColorNode::SafeDownCast(colorNode));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLColorNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetAndObserveColorNodeID(colorNode ? colorNode->GetID() : nullptr);
    }
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLModelDisplayNodeWidget::scalarsColorNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return vtkMRMLColorNode::SafeDownCast(d->ScalarsColorNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarRangeMode(ControlMode controlMode)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  bool modified = false;
  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkMRMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    int newScalarRangeMode = vtkMRMLDisplayNode::UseManualScalarRange;
    switch (controlMode)
      {
      case Data: newScalarRangeMode = vtkMRMLDisplayNode::UseDataScalarRange; break;
      case LUT: newScalarRangeMode = vtkMRMLDisplayNode::UseColorNodeScalarRange; break;
      case DataType: newScalarRangeMode = vtkMRMLDisplayNode::UseDataTypeScalarRange; break;
      case Manual: newScalarRangeMode = vtkMRMLDisplayNode::UseManualScalarRange; break;
      case DirectMapping: newScalarRangeMode = vtkMRMLDisplayNode::UseDirectMapping; break;
      }

    int currentScalarRangeMode = modelDisplayNode->GetScalarRangeFlag();
    if (currentScalarRangeMode != newScalarRangeMode)
      {
      modelDisplayNode->SetScalarRangeFlag(newScalarRangeMode);
      modified = true;
      }
    }
  if (modified)
    {
    emit this->scalarRangeModeValueChanged(controlMode);
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarRangeMode(int scalarRangeMode)
{
  switch(scalarRangeMode)
    {
    case 0:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Data);
      break;
    case 1:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::LUT);
      break;
    case 2:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::DataType);
      break;
    case 3:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Manual);
      break;
    case 4:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::DirectMapping);
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::ControlMode qMRMLModelDisplayNodeWidget::scalarRangeMode() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  switch( d->DisplayedScalarRangeModeComboBox->currentIndex() )
    {
    case 0:
      return qMRMLModelDisplayNodeWidget::Data;
      break;
    case 1:
      return qMRMLModelDisplayNodeWidget::LUT;
      break;
    case 2:
      return qMRMLModelDisplayNodeWidget::DataType;
      break;
    case 3:
      return qMRMLModelDisplayNodeWidget::Manual;
      break;
    case 4:
      return qMRMLModelDisplayNodeWidget::DirectMapping;
      break;
    default:
      break;
    }
  return qMRMLModelDisplayNodeWidget::Data;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsDisplayRange(double min, double max)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkMRMLDisplayNode* displayNode, displayNodesInSelection)
    {
    double *range = displayNode->GetScalarRange();
    if (range[0] == min && range[1] == max)
      {
      return;
      }
    displayNode->SetScalarRange(min, max);
    }
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setTresholdEnabled(bool b)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkMRMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    modelDisplayNode->SetThresholdEnabled(b);
    }
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setThresholdRange(double min, double max)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkMRMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    double oldMin = modelDisplayNode->GetThresholdMin();
    double oldMax = modelDisplayNode->GetThresholdMax();
    if (oldMin == min && oldMax == max)
      {
      return;
      }
    modelDisplayNode->SetThresholdRange(min, max);
    }
}

// --------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::minimumValue() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  return d->DisplayedScalarRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::maximumValue() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  return d->DisplayedScalarRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMinimumValue(double min)
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  if (displayNodesInSelection.size() > 1)
    {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
    }

  d->DisplayedScalarRangeWidget->setMinimumValue(min);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMaximumValue(double max)
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  QList<vtkMRMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  if (displayNodesInSelection.size() > 1)
    {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
    }

  d->DisplayedScalarRangeWidget->setMaximumValue(max);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateWidgetFromMRML()
{
  bool wasBlocking;

  Q_D(qMRMLModelDisplayNodeWidget);
  this->setEnabled(d->CurrentDisplayNode.GetPointer() != nullptr);
  if (!d->CurrentDisplayNode.GetPointer())
    {
    emit displayNodeChanged();
    return;
    }

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

  wasBlocking = d->DistanceToColorNodeComboBox->blockSignals(true);
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
  bool showLineWidth = (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_WIREFRAME)
    || (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_SURFACE && d->CurrentDisplayNode->GetEdgeVisibility());
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

  d->LightingCheckBox->setChecked(d->CurrentDisplayNode->GetLighting());
  d->InterpolationComboBox->setCurrentIndex(d->CurrentDisplayNode->GetInterpolation());

  // Material
  d->Property->SetAmbient(d->CurrentDisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->CurrentDisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->CurrentDisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->CurrentDisplayNode->GetPower());

  // Scalars

  if (d->ScalarsVisibilityCheckBox->isChecked() != (bool)d->CurrentDisplayNode->GetScalarVisibility())
    {
    wasBlocking = d->ScalarsVisibilityCheckBox->blockSignals(true);
    d->ScalarsVisibilityCheckBox->setChecked(d->CurrentDisplayNode->GetScalarVisibility());
    d->ScalarsVisibilityCheckBox->blockSignals(wasBlocking);
    }

  double dataRange[2] = { 0.0, 0.0 };
  if (d->CurrentModelDisplayNode)
    {
    vtkDataArray* dataArray = d->CurrentModelDisplayNode->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(dataRange);
      }
    }

  // Update scalar values, range, decimals and single step
  double precision = 1.0;
  double dataMin = 0.0;
  double dataMax = 0.0;
  int decimals = 0;
  bool resetSliderRange = false;
  if (dataRange[0] < dataRange[1])
    {
    if (d->DataRangeUsedForAutoDisplayRange[0] != dataRange[0]
      || d->DataRangeUsedForAutoDisplayRange[1] != dataRange[1])
      {
      d->DataRangeUsedForAutoDisplayRange[0] = dataRange[0];
      d->DataRangeUsedForAutoDisplayRange[1] = dataRange[1];
      resetSliderRange = true;
      }
    // Begin with a precision of 1% of the range
    precision = dataRange[1]/100.0 - dataRange[0]/100.0;
    // Extend min/max by 20% to give some room to work with
    dataMin = (floor(dataRange[0]/precision) - 20 ) * precision;
    dataMax = (ceil(dataRange[1]/precision) + 20 ) * precision;
    // Use closest power of ten value as a step value
    precision = ctk::closestPowerOfTen(precision);
    // Find significant decimals to show
    double stepDecimals = ctk::significantDecimals(precision);
    double minDecimals = ctk::significantDecimals(dataRange[0]);
    double maxDecimals = ctk::significantDecimals(dataRange[1]);
    decimals = std::max(stepDecimals, std::max(minDecimals, maxDecimals));
    }

  double* displayRange = d->CurrentDisplayNode->GetScalarRange();

  wasBlocking = d->DisplayedScalarRangeWidget->blockSignals(true);
  if (resetSliderRange)
    {
    d->DisplayedScalarRangeWidget->setRange(std::min(dataMin, displayRange[0]), std::max(dataMax, displayRange[1]));
    }
  else
    {
    double currentRange[2] = { 0.0 };
    d->DisplayedScalarRangeWidget->range(currentRange);
    d->DisplayedScalarRangeWidget->setRange(std::min(currentRange[0], displayRange[0]),
      std::max(currentRange[1], displayRange[1]));
    }
  d->DisplayedScalarRangeWidget->setValues(displayRange[0], displayRange[1]);
  d->DisplayedScalarRangeWidget->setDecimals(decimals);
  d->DisplayedScalarRangeWidget->setSingleStep(precision);
  d->DisplayedScalarRangeWidget->setEnabled(d->CurrentDisplayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseManualScalarRange);
  d->DisplayedScalarRangeWidget->blockSignals(wasBlocking);

  double thresholdRange[2] = { 0.0, 0.0 };
  if (d->CurrentModelDisplayNode)
    {
    d->CurrentModelDisplayNode->GetThresholdRange(thresholdRange);
    }

  wasBlocking = d->ThresholdRangeWidget->blockSignals(true);
  d->ThresholdRangeWidget->setEnabled(d->CurrentModelDisplayNode && d->CurrentModelDisplayNode->GetThresholdEnabled());
  d->ThresholdRangeWidget->setRange(dataRange[0] - precision, dataRange[1] + precision);
  if (thresholdRange[0] <= thresholdRange[1])
    {
    // there is a valid threshold range
    // If current threshold values do not fit in the current data range
    // then we move the slider handles to make them fit,
    // but values in the display node will not be changed until the user moves the handles.
    d->ThresholdRangeWidget->setValues(std::max(dataRange[0] - precision, thresholdRange[0]),
      std::min(dataRange[1] + precision, thresholdRange[1]));
    }
  else
    {
    // no valid threshold range is set move handles to the center
    d->ThresholdRangeWidget->setValues(dataRange[0], dataRange[1]);
    }
  d->ThresholdRangeWidget->setDecimals(decimals);
  d->ThresholdRangeWidget->setSingleStep(precision);
  d->ThresholdRangeWidget->blockSignals(wasBlocking);

  wasBlocking = d->ThresholdCheckBox->blockSignals(true);
  d->ThresholdCheckBox->setEnabled(d->CurrentModelDisplayNode);
  d->ThresholdCheckBox->setChecked(d->CurrentModelDisplayNode && d->CurrentModelDisplayNode->GetThresholdEnabled());
  d->ThresholdCheckBox->blockSignals(wasBlocking);

  ControlMode controlMode = qMRMLModelDisplayNodeWidget::Manual;
  switch (d->CurrentDisplayNode->GetScalarRangeFlag())
    {
    case vtkMRMLDisplayNode::UseDataScalarRange: controlMode = qMRMLModelDisplayNodeWidget::Data; break;
    case vtkMRMLDisplayNode::UseColorNodeScalarRange: controlMode = qMRMLModelDisplayNodeWidget::LUT; break;
    case vtkMRMLDisplayNode::UseDataTypeScalarRange: controlMode = qMRMLModelDisplayNodeWidget::DataType; break;
    case vtkMRMLDisplayNode::UseManualScalarRange: controlMode = qMRMLModelDisplayNodeWidget::Manual; break;
    case vtkMRMLDisplayNode::UseDirectMapping: controlMode = qMRMLModelDisplayNodeWidget::DirectMapping; break;
    }

  wasBlocking = d->DisplayedScalarRangeModeComboBox->blockSignals(true);
  d->DisplayedScalarRangeModeComboBox->setCurrentIndex(controlMode);
  d->DisplayedScalarRangeModeComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
  d->ActiveScalarComboBox->setEnabled(d->CurrentModelDisplayNode);
  if (d->CurrentModelDisplayNode)
    {
    d->ActiveScalarComboBox->setDataSet(d->CurrentModelDisplayNode->GetInputMesh());
    if (d->ActiveScalarComboBox->currentArrayName() != d->CurrentModelDisplayNode->GetActiveScalarName())
      {
      d->ActiveScalarComboBox->setCurrentArray(d->CurrentModelDisplayNode->GetActiveScalarName());
      // Array location would need to be set in d->ActiveScalarComboBox if
      // same scalar name is used in multiple locations.
      }
    }
  d->ActiveScalarComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ScalarsColorNodeComboBox->blockSignals(true);
  if (d->ScalarsColorNodeComboBox->mrmlScene() != this->mrmlScene())
    {
    d->ScalarsColorNodeComboBox->setMRMLScene(this->mrmlScene());
    }
  if ( d->CurrentModelDisplayNode
    && d->ScalarsColorNodeComboBox->currentNodeID() != d->CurrentModelDisplayNode->GetColorNodeID() )
    {
    d->ScalarsColorNodeComboBox->setCurrentNodeID(d->CurrentModelDisplayNode->GetColorNodeID());
    }
  d->ScalarsColorNodeComboBox->setEnabled( d->CurrentModelDisplayNode &&
    d->CurrentModelDisplayNode->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseDirectMapping );
  d->ScalarsColorNodeComboBox->blockSignals(wasBlocking);
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
      modelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID("vtkMRMLFreeSurferProceduralColorNodeRedGreen");
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
    displayNode->SetColor(newColor.redF(), newColor.greenF(), newColor.blueF());
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
  d->Property->SetInterpolation(newInterpolation);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setDistanceToColorNode(vtkMRMLNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->CurrentModelDisplayNode.GetPointer())
    {
    return;
    }
  d->CurrentModelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID(
    colorNode ? colorNode->GetID() : nullptr);
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
