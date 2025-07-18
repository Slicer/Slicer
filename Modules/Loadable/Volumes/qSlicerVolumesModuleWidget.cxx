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
#include <QDebug>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// CTK includes
// #include <ctkModelTester.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLVolumeDisplayNode.h>

// Colors includes
#include <vtkSlicerColorLogic.h>
#include <vtkMRMLColorLegendDisplayNode.h>

// Volumes includes
#include "qSlicerVolumesModuleWidget.h"
#include "ui_qSlicerVolumesModuleWidget.h"

#include "vtkSlicerVolumesLogic.h"

//-----------------------------------------------------------------------------
class qSlicerVolumesModuleWidgetPrivate : public Ui_qSlicerVolumesModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::qSlicerVolumesModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::~qSlicerVolumesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setup()
{
  Q_D(qSlicerVolumesModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), d->MRMLVolumeInfoWidget, SLOT(setVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), d->VolumeDisplayWidget, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(nodeSelectionChanged(vtkMRMLNode*)));

  QObject::connect(d->ColorLegendCollapsibleButton, SIGNAL(contentsCollapsed(bool)), this, SLOT(colorLegendCollapsibleButtonCollapsed(bool)));

  // Set up labelmap conversion
  d->ConvertVolumeFrame->setVisible(false);
  QObject::connect(d->ConvertVolumeButton, SIGNAL(clicked()), this, SLOT(convertVolume()));
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::nodeSelectionChanged(vtkMRMLNode* node)
{
  Q_UNUSED(node);
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumesModuleWidget);

  vtkMRMLVolumeNode* currentVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode());

  // Color legend section
  vtkMRMLColorLegendDisplayNode* colorLegendNode = nullptr;
  if (currentVolumeNode)
  {
    colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(currentVolumeNode);
  }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
  if (!colorLegendNode)
  {
    d->ColorLegendCollapsibleButton->setCollapsed(true);
  }

  d->InfoCollapsibleButton->setEnabled(currentVolumeNode);
  d->DisplayCollapsibleButton->setEnabled(currentVolumeNode);
  d->ColorLegendCollapsibleButton->setEnabled(currentVolumeNode);

  // Convert to volume section
  bool convertVolumeSectionVisible = false;
  if (currentVolumeNode)
  {

    // Show convert to labelmap frame only if the exact type is scalar volume
    // (not labelmap, vector, tensor, DTI, etc.)
    QStringList convertTargetNodeTypes;
    if (!strcmp(currentVolumeNode->GetClassName(), "vtkMRMLScalarVolumeNode"))
    {
      convertVolumeSectionVisible = true;
      d->ConvertVolumeLabel->setText(tr("Convert to label map:"));
      convertTargetNodeTypes << "vtkMRMLLabelMapVolumeNode";
    }
    else if (!strcmp(currentVolumeNode->GetClassName(), "vtkMRMLLabelMapVolumeNode"))
    {
      convertVolumeSectionVisible = true;
      d->ConvertVolumeLabel->setText(tr("Convert to scalar volume:"));
      convertTargetNodeTypes << "vtkMRMLScalarVolumeNode";
    }
    if (convertVolumeSectionVisible)
    {
      // Set base name of target labelmap node
      d->ConvertVolumeTargetSelector->setBaseName(QString("%1_Label").arg(currentVolumeNode->GetName()));
      d->ConvertVolumeTargetSelector->setNodeTypes(convertTargetNodeTypes);
    }
  }
  d->ConvertVolumeFrame->setVisible(convertVolumeSectionVisible);
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::convertVolume()
{
  Q_D(qSlicerVolumesModuleWidget);

  vtkMRMLVolumeNode* currentVolume = vtkMRMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode());
  if (!vtkMRMLScalarVolumeNode::SafeDownCast(currentVolume))
  {
    qWarning() << Q_FUNC_INFO << " failed: Cannot convert this volume type";
    return;
  }
  vtkSlicerVolumesLogic* logic = vtkSlicerVolumesLogic::SafeDownCast(this->logic());
  if (!logic)
  {
    qWarning() << Q_FUNC_INFO << " failed: Invalid volumes logic";
    return;
  }
  vtkMRMLLabelMapVolumeNode* currentLabelMapVolumeNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(currentVolume);

  // If there is no target labelmap node selected, then perform in-place conversion
  vtkMRMLVolumeNode* targetVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ConvertVolumeTargetSelector->currentNode());
  bool inPlaceConversion = (targetVolumeNode == nullptr);
  if (inPlaceConversion)
  {
    if (currentLabelMapVolumeNode)
    {
      targetVolumeNode = vtkMRMLScalarVolumeNode::New();
    }
    else
    {
      targetVolumeNode = vtkMRMLLabelMapVolumeNode::New();
    }
    targetVolumeNode->SetName(currentVolume->GetName());
    targetVolumeNode->SetHideFromEditors(currentVolume->GetHideFromEditors());
    targetVolumeNode->SetSaveWithScene(currentVolume->GetSaveWithScene());
    targetVolumeNode->SetSelectable(currentVolume->GetSelectable());
    targetVolumeNode->SetSingletonTag(currentVolume->GetSingletonTag());
    targetVolumeNode->SetDescription(currentVolume->GetDescription());
    std::vector<std::string> attributeNames = targetVolumeNode->GetAttributeNames();
    for (std::vector<std::string>::iterator attributeNameIt = attributeNames.begin(); attributeNameIt != attributeNames.end(); ++attributeNameIt)
    {
      targetVolumeNode->SetAttribute(attributeNameIt->c_str(), currentVolume->GetAttribute(attributeNameIt->c_str()));
    }
    targetVolumeNode->SetDescription(currentVolume->GetDescription());
    this->mrmlScene()->AddNode(targetVolumeNode);
    targetVolumeNode->Delete(); // node is now solely owned by the scene
  }
  if (currentLabelMapVolumeNode)
  {
    logic->CreateScalarVolumeFromVolume(this->mrmlScene(), vtkMRMLScalarVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
  }
  else
  {
    logic->CreateLabelVolumeFromVolume(this->mrmlScene(), vtkMRMLLabelMapVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
  }

  // In case of in-place conversion select the new labelmap node and delete the scalar volume node
  if (inPlaceConversion)
  {
    d->ActiveVolumeNodeSelector->setCurrentNode(targetVolumeNode);
    this->mrmlScene()->RemoveNode(currentVolume);
  }
}

//-----------------------------------------------------------
bool qSlicerVolumesModuleWidget::setEditedNode(vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString()*/)
{
  Q_D(qSlicerVolumesModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLVolumeNode::SafeDownCast(node))
  {
    d->ActiveVolumeNodeSelector->setCurrentNode(node);
    return true;
  }

  if (vtkMRMLVolumeDisplayNode::SafeDownCast(node))
  {
    vtkMRMLVolumeDisplayNode* displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(node);
    vtkMRMLVolumeNode* displayableNode = vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
    {
      return false;
    }
    d->ActiveVolumeNodeSelector->setCurrentNode(displayableNode);
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::colorLegendCollapsibleButtonCollapsed(bool collapsed)
{
  Q_D(qSlicerVolumesModuleWidget);
  if (collapsed)
  {
    return;
  }

  vtkMRMLVolumeNode* currentVolume = vtkMRMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode());
  vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(currentVolume);
  if (!colorLegendNode && currentVolume)
  {
    // color legend node does not exist, we need to create it now

    // Pause render to prevent the new Color legend displayed for a moment before it is hidden.
    vtkMRMLApplicationLogic* mrmlAppLogic = this->logic()->GetMRMLApplicationLogic();
    if (mrmlAppLogic)
    {
      mrmlAppLogic->PauseRender();
    }
    colorLegendNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(currentVolume);
    if (colorLegendNode)
    {
      colorLegendNode->SetVisibility(false); // just because the groupbox is opened, don't show color legend yet
    }
    if (mrmlAppLogic)
    {
      mrmlAppLogic->ResumeRender();
    }
  }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
}

//-----------------------------------------------------------
void qSlicerVolumesModuleWidget::enter()
{
  Q_D(qSlicerVolumesModuleWidget);

  this->Superclass::enter();

  // If no node is selected then select the first displayed node to save the user a click
  if (!d->ActiveVolumeNodeSelector->currentNode())
  {
    vtkMRMLNode* node = d->ActiveVolumeNodeSelector->findFirstNodeByClass("vtkMRMLVolumeNode");
    if (node)
    {
      d->ActiveVolumeNodeSelector->setCurrentNode(node);
    }
  }
}
