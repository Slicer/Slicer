
// Qt includes
#include <QDebug>
#include <QComboBox>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeComboBox.h>

// EMSegment includes
#include "qSlicerEMSegmentEditRegistrationParametersPanel.h" 
#include "ui_qSlicerEMSegmentEditRegistrationParametersPanel.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLEMSAtlasNode.h>

// MRML includes
#include <vtkMRMLScene.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.panels.qSlicerEMSegmentEditRegistrationParametersPanel");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditRegistrationParametersPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentEditRegistrationParametersPanel>,
    public Ui_qSlicerEMSegmentEditRegistrationParametersPanel
{
public:

  qSlicerEMSegmentEditRegistrationParametersPanelPrivate()
    {
    }

  void setupUi(qSlicerEMSegmentEditRegistrationParametersPanel *widget);

  void updateAtlasScansToInputChannelsLayoutFromMRML();

  void updateMRMLFromAtlasScansToInputChannelsLayout();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersPanelPrivate methods

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanelPrivate::setupUi(
    qSlicerEMSegmentEditRegistrationParametersPanel* widget)
{
  this->Ui_qSlicerEMSegmentEditRegistrationParametersPanel::setupUi(widget);

  // Affine registration comboBox
  this->AffineRegistrationComboBox->addItem(
      widget->tr("None"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Align Image Centers"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationCenters);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Fast"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMIFast);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Rigid, MI"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMI);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Slow"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMISlow);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Rigid, NCC Fast"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCFast);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Rigid, NCC"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCC);
  this->AffineRegistrationComboBox->addItem(
      widget->tr("Rigid, NCC Slow"), vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCSlow);

  // Deformable registration comboBox
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("None"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("Fast"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMIFast);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("B-Spline, MI"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMI);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("Slow"), vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMISlow);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("B-Spline, NCC Fast"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCFast);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("B-Spline, NCC"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCC);
  this->DeformableRegistrationComboBox->addItem(
      widget->tr("B-Spline, NCC Slow"),
      vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCSlow);

  // Interpolation comboBox
  this->InterpolationComboBox->addItem(
      widget->tr("Nearest Neighbor"), vtkEMSegmentMRMLManager::InterpolationNearestNeighbor);
  this->InterpolationComboBox->addItem(
      widget->tr("Linear"), vtkEMSegmentMRMLManager::InterpolationLinear);
  this->InterpolationComboBox->addItem(
      widget->tr("Cubic"), vtkEMSegmentMRMLManager::InterpolationCubic);

  // Connect comboboxes
  QObject::connect(this->AffineRegistrationComboBox, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(onCurrentAffineRegistrationIndexChanged(int)));
  QObject::connect(this->DeformableRegistrationComboBox, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(onCurrentDeformableRegistrationIndexChanged(int)));
  QObject::connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(onCurrentInterpolationIndexChanged(int)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanelPrivate::updateAtlasScansToInputChannelsLayoutFromMRML()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersPanel);
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
void qSlicerEMSegmentEditRegistrationParametersPanelPrivate::
    updateMRMLFromAtlasScansToInputChannelsLayout()
{
  CTK_P(qSlicerEMSegmentEditRegistrationParametersPanel);
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
// qSlicerEMSegmentEditRegistrationParametersPanel methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersPanel::qSlicerEMSegmentEditRegistrationParametersPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditRegistrationParametersPanel);
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::updateWidgetFromMRML()
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);

  if (!this->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }

  QList<QWidget*> widgets;
  widgets << d->AffineRegistrationComboBox
      << d->DeformableRegistrationComboBox
      << d->InterpolationComboBox;

  foreach(QWidget* w, widgets)
    {
    w->setEnabled(this->mrmlManager()->HasGlobalParametersNode());
    }

  d->AffineRegistrationComboBox->setCurrentIndex(
      d->AffineRegistrationComboBox->findData(
          QVariant(this->mrmlManager()->GetRegistrationAffineType())));

  d->DeformableRegistrationComboBox->setCurrentIndex(
      d->DeformableRegistrationComboBox->findData(
          QVariant(this->mrmlManager()->GetRegistrationDeformableType())));

  d->InterpolationComboBox->setCurrentIndex(
      d->InterpolationComboBox->findData(
          QVariant(this->mrmlManager()->GetRegistrationInterpolationType())));

  d->updateAtlasScansToInputChannelsLayoutFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::updateMRMLFromWidget()
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);

  if (!this->mrmlManager())
    {
    logger.warn("updateMRMLFromWidget - MRMLManager is NULL");
    return;
    }

  d->updateMRMLFromAtlasScansToInputChannelsLayout();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::
    onCurrentAffineRegistrationIndexChanged(int index)
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);
  Q_ASSERT(this->mrmlManager());

  int type = d->AffineRegistrationComboBox->itemData(index).toInt();
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
  this->mrmlManager()->SetRegistrationAffineType(type);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::
    onCurrentDeformableRegistrationIndexChanged(int index)
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);
  Q_ASSERT(this->mrmlManager());

  int type = d->DeformableRegistrationComboBox->itemData(index).toInt();
  Q_ASSERT(type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMI ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCC ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMIFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCFast ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineMMISlow ||
           type == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationBSplineNCCSlow);
  logger.debug(QString("onCurrentDeformableRegistrationIndexChanged - type: %1").
               arg(type));
  this->mrmlManager()->SetRegistrationDeformableType(type);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersPanel::onCurrentInterpolationIndexChanged(int index)
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersPanel);
  Q_ASSERT(this->mrmlManager());

  int type = d->InterpolationComboBox->itemData(index).toInt();
  Q_ASSERT(type == vtkEMSegmentMRMLManager::InterpolationLinear ||
           type == vtkEMSegmentMRMLManager::InterpolationNearestNeighbor ||
           type == vtkEMSegmentMRMLManager::InterpolationCubic);
  logger.debug(QString("onCurrentInterpolationIndexChanged - type: %1").
               arg(type));
  this->mrmlManager()->SetRegistrationInterpolationType(type);
}

