/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by
    Danielle Pace and Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QComboBox>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeComboBox.h>

// EMSegment includes
#include "qSlicerEMSegmentEditRegistrationParametersStep.h"
#include "ui_qSlicerEMSegmentEditRegistrationParametersPanel.h"
#include "qSlicerEMSegmentEditRegistrationParametersStep_p.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLEMSAtlasNode.h>

// MRML includes
#include <vtkMRMLScene.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentEditRegistrationParametersStep");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersStepPrivate::qSlicerEMSegmentEditRegistrationParametersStepPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersStepPrivate methods

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::setupUi(
    qSlicerEMSegmentWorkflowWidgetStep* step)
{

  this->Ui_qSlicerEMSegmentEditRegistrationParametersPanel::setupUi(step);

  // Affine registration comboBox
  this->AffineRegistrationComboBox->addItem(
      step->tr("None"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Align Image Centers"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationCenters);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Fast"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMIFast);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Rigid, MI"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMI);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Slow"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMISlow);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Rigid, NCC Fast"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCFast);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Rigid, NCC"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCC);
  this->AffineRegistrationComboBox->addItem(
      step->tr("Rigid, NCC Slow"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCSlow);

  // Deformable registration comboBox
  this->DeformableRegistrationComboBox->addItem(
      step->tr("None"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("Fast"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMIFast);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("B-Spline, MI"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMI);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("Slow"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMISlow);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("B-Spline, NCC Fast"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCFast);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("B-Spline, NCC"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCC);
  this->DeformableRegistrationComboBox->addItem(
      step->tr("B-Spline, NCC Slow"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCSlow);

  // Interpolation comboBox
  this->InterpolationComboBox->addItem(
      step->tr("Nearest Neighbor"), vtkEMSegmentMRMLManager::InterpolationNearestNeighbor);
  this->InterpolationComboBox->addItem(
      step->tr("Linear"), vtkEMSegmentMRMLManager::InterpolationLinear);
  this->InterpolationComboBox->addItem(
      step->tr("Cubic"), vtkEMSegmentMRMLManager::InterpolationCubic);

  // Connect comboboxes
  QObject::connect(this->AffineRegistrationComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onCurrentAffineRegistrationIndexChanged(int)));
  QObject::connect(this->DeformableRegistrationComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onCurrentDeformableRegistrationIndexChanged(int)));
  QObject::connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onCurrentInterpolationIndexChanged(int)));

}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::updateAtlasScansToInputChannelsLayoutFromMRML()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);
  Q_ASSERT(p->mrmlManager());

  vtkMRMLEMSTargetNode *inputNodes = p->mrmlManager()->GetTargetInputNode();
  Q_ASSERT(inputNodes);

  // Loop over inputChannels
  for(int inputVolumeId = 0; inputVolumeId < inputNodes->GetNumberOfVolumes(); inputVolumeId++)
    {
    QString inputChannelName = QLatin1String(inputNodes->GetNthInputChannelName(inputVolumeId));
    logger.debug(QString("updateAtlasScansToInputChannelsLayoutFromMRML - inputChannelName: %1").
                 arg(inputChannelName));
    vtkMRMLVolumeNode * volumeNode = inputNodes->GetNthVolumeNode(inputVolumeId);
    Q_ASSERT(volumeNode);

    vtkIdType volumeId = inputNodes->GetIndexByVolumeNodeID(volumeNode->GetID());
    logger.debug(QString("updateAtlasScansToInputChannelsLayoutFromMRML - volumeId: %1").
                 arg(volumeId));

    vtkIdType atlasNodeIDToSelect = p->mrmlManager()->GetRegistrationAtlasVolumeID(volumeId);
    logger.debug(QString("updateAtlasScansToInputChannelsLayoutFromMRML - atlasNodeIDToSelect: %1").
                 arg(atlasNodeIDToSelect));

    if (inputVolumeId >= this->AtlasScansToInputChannelsLayout->rowCount())
      { // Insert
//      qMRMLNodeComboBox * atlasNodeComboBox = new qMRMLNodeComboBox(p);
//      QStringList nodeTypes;
//      nodeTypes << "vtkMRMLVolumeNode";
//      atlasNodeComboBox->setToolTip(
//          p->tr("Select atlas volume representing the %1Nth channel.").arg(inputVolumeId));
//      atlasNodeComboBox->setNodeTypes(nodeTypes);
//      atlasNodeComboBox->setNoneEnabled(true);
//      atlasNodeComboBox->setAddEnabled(false);
//      atlasNodeComboBox->setRemoveEnabled(false);
//      atlasNodeComboBox->setMRMLScene(p->mrmlScene());
//      atlasNodeComboBox->setCurrentNode(atlasMRMLNodeIDToSelect);

      // Since couldn't convert atlasNodeIDToSelect <-> MRMLId, let's use a
      // QComboBox instead of qMRMLNodeComboBox
      QComboBox * atlasNodeComboBox = new QComboBox(p);
      atlasNodeComboBox->setToolTip(
          p->tr("Select atlas volume representing the %1Nth channel.").arg(inputVolumeId));
      atlasNodeComboBox->addItem("None"); // index:0 -> ERROR_NODE_VTKID
      for(int i = 0; i < p->mrmlManager()->GetVolumeNumberOfChoices(); i++)
        {
        vtkIdType volumeId = p->mrmlManager()->GetVolumeNthID(i);
        atlasNodeComboBox->addItem(p->mrmlManager()->GetVolumeName(volumeId),
                                   QVariant(volumeId));
        }
      int index = atlasNodeComboBox->findData(QVariant(atlasNodeIDToSelect));
      atlasNodeComboBox->setCurrentIndex(index == -1 ? 0 : index);

      this->AtlasScansToInputChannelsLayout->addRow(inputChannelName, atlasNodeComboBox);
      }
    else
      { // Update

      // Get a reference to the QLabel
      QLayoutItem * labelItem =
          this->AtlasScansToInputChannelsLayout->itemAt(inputVolumeId, QFormLayout::LabelRole);
      Q_ASSERT(labelItem);
      QLabel * label = qobject_cast<QLabel*>(labelItem->widget());
      Q_ASSERT(label);

      // Update label
      label->setText(inputChannelName);

      // Get a reference to the QComboBox
      QLayoutItem * fieldItem =
          this->AtlasScansToInputChannelsLayout->itemAt(inputVolumeId, QFormLayout::FieldRole);
      Q_ASSERT(fieldItem);
      QComboBox * atlasNodeComboBox = qobject_cast<QComboBox*>(fieldItem->widget());
      Q_ASSERT(atlasNodeComboBox);

      // Update comboBox
      int index = atlasNodeComboBox->findData(QVariant(atlasNodeIDToSelect));
      atlasNodeComboBox->setCurrentIndex(index == -1 ? 0 : index);
      }
    }

  // Remove extra rows
  if (this->AtlasScansToInputChannelsLayout->rowCount() > inputNodes->GetNumberOfVolumes())
    {
    int rowId =
        this->AtlasScansToInputChannelsLayout->rowCount() + 1 - inputNodes->GetNumberOfVolumes();

    QLayoutItem *child;
    while ((child = this->AtlasScansToInputChannelsLayout->takeAt(rowId)) != 0)
      {
      delete child;
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::
    updateMRMLFromAtlasScansToInputChannelsLayout()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);
  Q_ASSERT(p->mrmlManager());

  for(int inputVolumeId = 0; inputVolumeId < this->AtlasScansToInputChannelsLayout->rowCount(); inputVolumeId++)
    {
    // Get a reference to the QComboBox
    QLayoutItem * fieldItem =
        this->AtlasScansToInputChannelsLayout->itemAt(inputVolumeId, QFormLayout::FieldRole);
    Q_ASSERT(fieldItem);
    QComboBox * atlasNodeComboBox = qobject_cast<QComboBox*>(fieldItem->widget());
    Q_ASSERT(atlasNodeComboBox);

    int atlasVolumeId = atlasNodeComboBox->itemData(atlasNodeComboBox->currentIndex()).toInt();
    logger.debug(QString(
            "updateMRMLFromAtlasScansToInputChannelsLayout - inputVolumeId:%1, atlasVolumeId:%2").
                 arg(inputVolumeId).arg(atlasVolumeId));
    p->mrmlManager()->SetRegistrationAtlasVolumeID(inputVolumeId, atlasVolumeId);
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::updateWidgetFromMRML()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);

  if (!p->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }

  QList<QWidget*> widgets;
  widgets << this->AffineRegistrationComboBox
      << this->DeformableRegistrationComboBox
      << this->InterpolationComboBox;

  foreach(QWidget* w, widgets)
    {
    w->setEnabled(p->mrmlManager()->HasGlobalParametersNode());
    }

  this->AffineRegistrationComboBox->setCurrentIndex(
      this->AffineRegistrationComboBox->findData(
          QVariant(p->mrmlManager()->GetRegistrationAffineType())));

  this->DeformableRegistrationComboBox->setCurrentIndex(
      this->DeformableRegistrationComboBox->findData(
          QVariant(p->mrmlManager()->GetRegistrationDeformableType())));

  this->InterpolationComboBox->setCurrentIndex(
      this->InterpolationComboBox->findData(
          QVariant(p->mrmlManager()->GetRegistrationInterpolationType())));

  this->updateAtlasScansToInputChannelsLayoutFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::updateMRMLFromWidget()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);

  if (!p->mrmlManager())
    {
    logger.warn("updateMRMLFromWidget - MRMLManager is NULL");
    return;
    }

  this->updateMRMLFromAtlasScansToInputChannelsLayout();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::
    onCurrentAffineRegistrationIndexChanged(int index)
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);
  Q_ASSERT(p->mrmlManager());

  int type = this->AffineRegistrationComboBox->itemData(index).toInt();
  Q_ASSERT(type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationCenters ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMI ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCC ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMIFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMISlow ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCSlow);
  logger.debug(QString("onCurrentAffineRegistrationIndexChanged - type: %1").
               arg(type));
  p->mrmlManager()->SetRegistrationAffineType(type);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::
    onCurrentDeformableRegistrationIndexChanged(int index)
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);
  Q_ASSERT(p->mrmlManager());

  int type = this->DeformableRegistrationComboBox->itemData(index).toInt();
  Q_ASSERT(type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMI ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCC ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMIFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMISlow ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCSlow);
  logger.debug(QString("onCurrentDeformableRegistrationIndexChanged - type: %1").
               arg(type));
  p->mrmlManager()->SetRegistrationDeformableType(type);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStepPrivate::onCurrentInterpolationIndexChanged(int index)
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersStep);
  Q_ASSERT(p->mrmlManager());

  int type = this->InterpolationComboBox->itemData(index).toInt();
  Q_ASSERT(type == vtkEMSegmentMRMLManager::InterpolationLinear ||
           type == vtkEMSegmentMRMLManager::InterpolationNearestNeighbor ||
           type == vtkEMSegmentMRMLManager::InterpolationCubic);
  logger.debug(QString("onCurrentInterpolationIndexChanged - type: %1").
               arg(type));
  p->mrmlManager()->SetRegistrationInterpolationType(type);
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentEditRegistrationParametersStep::StepId = "EditRegistrationParameters";

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersStep::qSlicerEMSegmentEditRegistrationParametersStep(
    ctkWorkflow* newWorkflow, QWidget* newWidget) : Superclass(newWorkflow, Self::StepId, newWidget)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditRegistrationParametersStep);
  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);
  d->setupUi(this);

  this->setName("5/9. Edit Registration Parameters");
  this->setDescription("Specify atlas-to-input scans registration parameters.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::createUserInterface()
{
  emit createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);
  d->updateWidgetFromMRML();

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);

  Q_ASSERT(this->mrmlManager());

  if (transitionType != ctkWorkflowInterstepTransition::TransitionToNextStep)
    {
    // Signals that we are finished
    emit onExitComplete();
    return;
    }

  d->updateMRMLFromWidget();
 
  // Signals that we are finished
  emit onExitComplete();
}
