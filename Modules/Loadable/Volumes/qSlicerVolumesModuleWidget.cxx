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

// CTK includes
//#include <ctkModelTester.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLVolumeDisplayNode.h>

// Volumes includes
#include "qSlicerVolumesModuleWidget.h"
#include "ui_qSlicerVolumesModuleWidget.h"

#include "vtkSlicerVolumesLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerVolumesModuleWidgetPrivate: public Ui_qSlicerVolumesModuleWidget
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

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   d->MRMLVolumeInfoWidget, SLOT(setVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   d->VolumeDisplayWidget, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(nodeSelectionChanged(vtkMRMLNode*)));

  // Set up labelmap conversion
  d->ConvertVolumeFrame->setVisible(false);
  QObject::connect(d->ConvertVolumeButton, SIGNAL(clicked()),
                   this, SLOT(convertVolume()));
  /*QObject::connect(d->ConvertVolumeTargetSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(nodeSelectionChanged(vtkMRMLNode*)));*/

  //ctkModelTester* tester = new ctkModelTester(this);
  //tester->setModel(d->ActiveVolumeNodeSelector->model());
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

  vtkMRMLVolumeNode* currentVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    d->ActiveVolumeNodeSelector->currentNode() );
  if (!currentVolumeNode)
    {
    d->ConvertVolumeFrame->setVisible(false);
    return;
    }

  // Show convert to labelmap frame only if the exact type is scalar volume
  // (not labelmap, vector, tensor, DTI, etc.)
  QStringList convertTargetNodeTypes;
  if (!strcmp(currentVolumeNode->GetClassName(), "vtkMRMLScalarVolumeNode"))
    {
    d->ConvertVolumeFrame->setVisible(true);
    d->ConvertVolumeLabel->setText(tr("Convert to label map:"));
    convertTargetNodeTypes << "vtkMRMLLabelMapVolumeNode";
    }
  else if (!strcmp(currentVolumeNode->GetClassName(), "vtkMRMLLabelMapVolumeNode"))
    {
    d->ConvertVolumeFrame->setVisible(true);
    d->ConvertVolumeLabel->setText(tr("Convert to scalar volume:"));
    convertTargetNodeTypes << "vtkMRMLScalarVolumeNode";
    }
  else
    {
    d->ConvertVolumeFrame->setVisible(false);
    }

  // Set base name of target labelmap node
  d->ConvertVolumeTargetSelector->setBaseName(QString("%1_Label").arg(currentVolumeNode->GetName()));
  d->ConvertVolumeTargetSelector->setNodeTypes(convertTargetNodeTypes);
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
  vtkMRMLVolumeNode* targetVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    d->ConvertVolumeTargetSelector->currentNode());
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
    std::vector< std::string > attributeNames = targetVolumeNode->GetAttributeNames();
    for (std::vector< std::string >::iterator attributeNameIt = attributeNames.begin();
      attributeNameIt != attributeNames.end(); ++attributeNameIt)
      {
      targetVolumeNode->SetAttribute(attributeNameIt->c_str(), currentVolume->GetAttribute(attributeNameIt->c_str()));
      }
    targetVolumeNode->SetDescription(currentVolume->GetDescription());
    this->mrmlScene()->AddNode(targetVolumeNode);
    targetVolumeNode->Delete(); // node is now solely owned by the scene
    }
  if (currentLabelMapVolumeNode)
    {
    logic->CreateScalarVolumeFromVolume(this->mrmlScene(),
      vtkMRMLScalarVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
    }
  else
    {
    logic->CreateLabelVolumeFromVolume(this->mrmlScene(),
      vtkMRMLLabelMapVolumeNode::SafeDownCast(targetVolumeNode), currentVolume);
    }

  // In case of in-place conversion select the new labelmap node and delete the scalar volume node
  if (inPlaceConversion)
    {
    d->ActiveVolumeNodeSelector->setCurrentNode(targetVolumeNode);
    this->mrmlScene()->RemoveNode(currentVolume);
    }
}

//-----------------------------------------------------------
bool qSlicerVolumesModuleWidget::setEditedNode(vtkMRMLNode* node,
                                               QString role /* = QString()*/,
                                               QString context /* = QString()*/)
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
