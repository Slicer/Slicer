
// CTK includes
#include <ctkUtils.h>

// MRML includes
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"

// Tractography Logic includes
#include "vtkSlicerTractographyInteractiveSeedingLogic.h"
#include "vtkMRMLTractographyInteractiveSeedingNode.h"

// Tractography QTModule includes
#include "qSlicerTractographyInteractiveSeedingModuleWidget.h"
#include "ui_qSlicerTractographyInteractiveSeedingModuleWidget.h"

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyInteractiveSeeding
class qSlicerTractographyInteractiveSeedingModuleWidgetPrivate:
  public Ui_qSlicerTractographyInteractiveSeedingModuleWidget
{
};

//-----------------------------------------------------------------------------
qSlicerTractographyInteractiveSeedingModuleWidget::qSlicerTractographyInteractiveSeedingModuleWidget(QWidget *_parent)
  : Superclass(_parent),
  settingFiberBundleNode(false),
  settingMRMLScene(false),
  d_ptr(new qSlicerTractographyInteractiveSeedingModuleWidgetPrivate)
{
  this->TractographyInteractiveSeedingNode = 0;
}
//-----------------------------------------------------------------------------
qSlicerTractographyInteractiveSeedingModuleWidget::~qSlicerTractographyInteractiveSeedingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::onEnter()
{
  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);

  if (this->mrmlScene() == 0)
    {
    return;
    }

  vtkSlicerTractographyInteractiveSeedingLogic* logic =
           vtkSlicerTractographyInteractiveSeedingLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    return;
    }

  // first check the logic if it has a parameter node
  if (logic->GetTractographyInteractiveSeedingNode())
    {
    this->setTractographyInteractiveSeedingNode(logic->GetTractographyInteractiveSeedingNode());
    }

  // if we have a parameter node select it
  if (this->TractographyInteractiveSeedingNode == 0)
    {
    vtkMRMLNode * node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLTractographyInteractiveSeedingNode");
    if (node)
      {
      this->setTractographyInteractiveSeedingNode(node);
      return;
      }
    else
      {
      vtkMRMLNode * nodeAdded =
          this->mrmlScene()->AddNode(vtkNew<vtkMRMLTractographyInteractiveSeedingNode>().GetPointer());
      this->setTractographyInteractiveSeedingNode(nodeAdded);
      }
    }
  else
    {
    this->updateWidgetFromMRML();
    return;
    }

  // if we have one dti volume node select it
  std::vector<vtkMRMLNode*> nodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLDiffusionTensorVolumeNode", nodes);
  if (nodes.size() == 1 && d->DTINodeSelector->currentNode() == 0)
    {
    this->setDiffusionTensorVolumeNode(nodes[0]);
    }

  // if we have one Fiducial List node select it
  nodes.clear();
  this->mrmlScene()->GetNodesByClass("vtkMRMLAnnotationHierarchyNode", nodes);
  if (nodes.size() > 1 && d->FiducialNodeSelector->currentNode() == 0)
    {
    for (unsigned int i=0; i<nodes.size(); i++)
      {
      vtkMRMLAnnotationHierarchyNode *hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(nodes[i]);
      vtkCollection *cnodes = vtkCollection::New();
      hnode->GetDirectChildren(cnodes);
      if (cnodes->GetNumberOfItems() > 0 && cnodes->GetNumberOfItems() < 5 &&
          vtkMRMLAnnotationFiducialNode::SafeDownCast(cnodes->GetItemAsObject(0)) != NULL)
        {
        this->setSeedingNode(nodes[i]);
        cnodes->RemoveAllItems();
        cnodes->Delete();
        break;
        }
      cnodes->RemoveAllItems();
      cnodes->Delete();
      }
    }

  // if we dont' have FiberBundleNode create it
  nodes.clear();
  this->mrmlScene()->GetNodesByClass("vtkMRMLFiberBundleNode", nodes);
  if (nodes.size() == 0 && d->FiberNodeSelector->currentNode() == 0)
    {
    vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::New();
    fiberNode->SetScene(this->mrmlScene());

    vtkMRMLFiberBundleDisplayNode *dnode = 0;
    dnode = fiberNode->AddTubeDisplayNode();
    dnode->DisableModifiedEventOn();
    dnode->SetScalarVisibility(1);
    dnode->SetOpacity(1);
    dnode->SetVisibility(1);
    dnode->DisableModifiedEventOff();

    dnode = fiberNode->AddLineDisplayNode();
    dnode->DisableModifiedEventOn();
    dnode->SetVisibility(0);
    dnode->SetOpacity(1);
    dnode->SetScalarVisibility(0);
    dnode->DisableModifiedEventOff();

    dnode = fiberNode->AddGlyphDisplayNode();
    dnode->DisableModifiedEventOn();
    dnode->SetVisibility(0);
    dnode->SetScalarVisibility(1);
    dnode->SetOpacity(1);
    dnode->DisableModifiedEventOff();

    fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(this->mrmlScene()->AddNode(fiberNode));
    Q_ASSERT(fiberNode);
    fiberNode->SetName("FiberBundle");

    this->setFiberBundleNode(fiberNode);
    fiberNode->Delete();
    }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->settingMRMLScene = true;
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect(this->logic(), scene, vtkMRMLScene::EndImportEvent,
                this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(this->logic(), scene, vtkMRMLScene::EndRestoreEvent,
                this, SLOT(onSceneRestoredEvent()));

  // find parameters node or create it if there is no one in the scene
  if (scene && this->TractographyInteractiveSeedingNode == 0)
    {
    vtkMRMLTractographyInteractiveSeedingNode *tnode = 0;
    vtkMRMLNode *node = scene->GetNthNodeByClass(0, "vtkMRMLTractographyInteractiveSeedingNode");
    if (node)
      {
      tnode = vtkMRMLTractographyInteractiveSeedingNode::SafeDownCast(node);
      this->setTractographyInteractiveSeedingNode(tnode);
      }
    }
  this->settingMRMLScene = false;
}


//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::onSceneRestoredEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setup()
{
  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);
  d->setupUi(this);

  d->FiducialNodeSelector->addAttribute("vtkMRMLScalarVolumeNode", "LabelMap", "1");

  QObject::connect(d->DTINodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
                                       SLOT(setDiffusionTensorVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->FiducialNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
                                            SLOT(setSeedingNode(vtkMRMLNode*)));

  QObject::connect(d->FiberNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
                                         SLOT(setFiberBundleNode(vtkMRMLNode*)));

  QObject::connect(d->StoppingCurvatureSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setStoppingCurvature(double)));

  QObject::connect(d->StoppingCriteriaComboBox,
                SIGNAL(currentIndexChanged(int)),
                SLOT(setStoppingCriteria(int)));

  QObject::connect(d->StoppingValueSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setStoppingValue(double)));

  QObject::connect(d->IntegrationStepSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setIntegrationStep(double)));

  QObject::connect(d->MinimumPathSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setMinimumPath(double)));

  QObject::connect(d->MaximumPathSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setMaximumPath(double)));

  QObject::connect(d->FiducialRegionSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setFiducialRegion(double)));

  QObject::connect(d->FiducialStepSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(setFiducialRegionStep(double)));

  QObject::connect(d->DisplayTracksComboBox,
                SIGNAL(currentIndexChanged(int)),
                SLOT(setTrackDisplayMode(int)));

  QObject::connect(d->SeedSelectedCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(setSeedSelectedFiducials(int)));

  QObject::connect(d->EnableSeedingCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(setEnableSeeding(int)));

  QObject::connect(d->MaxNumberSeedsNumericInput,
                SIGNAL(valueChanged(int)),
                SLOT(setMaxNumberSeeds(int)));

  QObject::connect(d->LinearMeasureStartSlider,
                SIGNAL(valueChanged(double)),
                SLOT(setLinearMeasureStart(double)));

  QObject::connect(d->ROILabelInput,
                SIGNAL(textChanged(const QString &)),
                SLOT(setROILabels(const QString &)));

  QObject::connect(d->ROILabelInput,
                SIGNAL(returnPressed()),
                SLOT(setROILabels()));

  QObject::connect(d->RandomGridCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(setRandomGrid(int)));

  QObject::connect(d->SeedSpacingSlider,
                SIGNAL(valueChanged(double)),
                SLOT(setSeedSpacing(double)));

  QObject::connect(d->UseIndexSpaceCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(setUseIndexSpace(int)));

  QObject::connect(d->WriteFibersCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(setWriteFibers(int)));

  QObject::connect(d->OutputDirectoryButton,
                SIGNAL(directorySelected(const QString &)),
                SLOT(setDirectory(const QString &)));

  QObject::connect(d->FilePrefixLineEdit,
                SIGNAL(textChanged(const QString &)),
                SLOT(setFilePrefix(const QString &)));

  QObject::connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
                                             SLOT(setTractographyInteractiveSeedingNode(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerTractographyInteractiveSeedingModuleWidget::seedingNode()
{
  vtkMRMLNode *node = 0;
  if (this->TractographyInteractiveSeedingNode)
    {
    node = this->mrmlScene()->GetNodeByID(
                        this->TractographyInteractiveSeedingNode->GetInputFiducialRef());
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode* qSlicerTractographyInteractiveSeedingModuleWidget::diffusionTensorVolumeNode()
{
  vtkMRMLDiffusionTensorVolumeNode *dtiNode = 0;
  if (this->TractographyInteractiveSeedingNode)
    {
    dtiNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(
              this->TractographyInteractiveSeedingNode->GetInputVolumeRef()));
    }
  return dtiNode;
}

//-----------------------------------------------------------------------------
vtkMRMLFiberBundleNode* qSlicerTractographyInteractiveSeedingModuleWidget::fiberBundleNode()
{
  vtkMRMLFiberBundleNode *fiberNode = 0;
  if (this->TractographyInteractiveSeedingNode)
    {
    fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(this->mrmlScene()->GetNodeByID(
                this->TractographyInteractiveSeedingNode->GetOutputFiberRef()));
    }
  return fiberNode;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setTractographyInteractiveSeedingNode(vtkMRMLNode *node)
{
  if (this->settingMRMLScene)
    {
    return;
    }

  vtkMRMLTractographyInteractiveSeedingNode *paramNode = vtkMRMLTractographyInteractiveSeedingNode::SafeDownCast(node);

  // each time the node is modified, the logic creates tracks
  vtkSlicerTractographyInteractiveSeedingLogic *seedingLogic =
        vtkSlicerTractographyInteractiveSeedingLogic::SafeDownCast(this->logic());
  if (seedingLogic && this->mrmlScene())
    {
    seedingLogic->SetAndObserveTractographyInteractiveSeedingNode(paramNode);
    }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(this->TractographyInteractiveSeedingNode, paramNode,
                       vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->TractographyInteractiveSeedingNode = paramNode;
  this->updateWidgetFromMRML();
}


//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setSeedingNode(vtkMRMLNode *node)
{
  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);

  if (this->settingMRMLScene)
    {
    return;
    }

  if (vtkMRMLScalarVolumeNode::SafeDownCast(d->FiducialNodeSelector->currentNode()) != 0)
    {
    d->stackedWidget->setCurrentIndex(1);

    // set defult label value in UI
    vtkMRMLScalarVolumeNode *labelsVolume = vtkMRMLScalarVolumeNode::SafeDownCast(d->FiducialNodeSelector->currentNode());
    if (labelsVolume && labelsVolume->GetImageData())
      {
      double range[2];
      labelsVolume->GetImageData()->GetScalarRange(range);
      int label = (int)range[0];
      if (label < 1)
        {
        label = 1;
        }
      std::stringstream ss;
      ss << label;
      QString str = QString::fromUtf8(ss.str().c_str());
      d->ROILabelInput->setText(str); 
      }
    }
  else
    {
    d->stackedWidget->setCurrentIndex(0);
    }

  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetInputFiducialRef(node ?
                                                               node->GetID() : "" );
    vtkSlicerTractographyInteractiveSeedingLogic *seedingLogic =
          vtkSlicerTractographyInteractiveSeedingLogic::SafeDownCast(this->logic());
    if (seedingLogic && this->mrmlScene())
      {
      seedingLogic->SetAndObserveTractographyInteractiveSeedingNode(this->TractographyInteractiveSeedingNode);
      }
    }
}

//-----------------------------------------------------------------------------
static double round_num(double num)
{
  double result = num;

  if (num < 1.0)
    {
    std::stringstream ss;
    ss << num;
    std::string s = ss.str();
    std::stringstream res;
    for (unsigned int i=0; i<s.size(); i++)
      {
      if (s.at(i) != '0' && s.at(i) != '.')
        {
        res << s.at(i);
        break;
        }
        res << s.at(i);
      }
    res >> result;
    }
  else
    {
    // drop off everything past the decimal point
    result = floor(result);
    }

  return result;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setDiffusionTensorVolumeNode(vtkMRMLNode *node)
{
  if (this->settingMRMLScene)
    {
    return;
    }

  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);

  vtkMRMLDiffusionTensorVolumeNode *diffusionTensorVolumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node);

  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetInputVolumeRef(diffusionTensorVolumeNode ?
                                                             diffusionTensorVolumeNode->GetID() : "" );
    vtkSlicerTractographyInteractiveSeedingLogic *seedingLogic =
          vtkSlicerTractographyInteractiveSeedingLogic::SafeDownCast(this->logic());
    if (seedingLogic && this->mrmlScene())
      {
      seedingLogic->SetAndObserveTractographyInteractiveSeedingNode(this->TractographyInteractiveSeedingNode);
      }
    }

  if (diffusionTensorVolumeNode && diffusionTensorVolumeNode->GetImageData())
    {
    double spacing[3];
    diffusionTensorVolumeNode->GetSpacing(spacing);
    double minSpacing = spacing[0];
    for (int i=1; i<3; i++)
      {
      if (spacing[i] < minSpacing)
        {
        minSpacing = spacing[i];
        }
      }
    // get 0 decimal places
    minSpacing = round_num(0.5*minSpacing);

    int decimal = ctk::orderOfMagnitude(minSpacing);
    decimal = decimal >= 0 ? 0 : -decimal;

    d->FiducialStepSpinBox->setDecimals(decimal+1);
    d->FiducialStepSpinBox->setSingleStep(minSpacing);
    d->FiducialStepSpinBox->setMinimum(minSpacing);
    d->FiducialStepSpinBox->setMaximum(10*minSpacing);

    d->FiducialRegionSpinBox->setDecimals(decimal+1);
    d->FiducialRegionSpinBox->setSingleStep(minSpacing);
    d->FiducialRegionSpinBox->setMinimum(minSpacing);
    d->FiducialRegionSpinBox->setMaximum(100*minSpacing);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setFiberBundleNode(vtkMRMLNode *node)
{
  if (this->settingMRMLScene)
    {
    return;
    }

  if (settingFiberBundleNode)
    {
    return;
    }
  settingFiberBundleNode = true;
  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(node);
  if (this->TractographyInteractiveSeedingNode)
    {
    if (this->TractographyInteractiveSeedingNode->GetInputFiducialRef())
      {
      vtkMRMLNode *seedNode = this->mrmlScene()->GetNodeByID(this->TractographyInteractiveSeedingNode->GetInputFiducialRef());
      if (fiberBundleNode && seedNode && seedNode->GetName() && fiberBundleNode->GetName())
        {
        fiberBundleNode->SetName(std::string(std::string(fiberBundleNode->GetName()) +
                      std::string("_")+std::string(seedNode->GetName())).c_str());
        }
      }
    this->TractographyInteractiveSeedingNode->SetOutputFiberRef(fiberBundleNode ?
                                                             fiberBundleNode->GetID() : "" );
    }
  settingFiberBundleNode = false;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);

  vtkMRMLTractographyInteractiveSeedingNode *paramNode = this->TractographyInteractiveSeedingNode;

  if (paramNode && this->mrmlScene())
    {
    d->IntegrationStepSpinBox->setValue(paramNode->GetIntegrationStep());
    d->MaxNumberSeedsNumericInput->setValue(paramNode->GetMaxNumberOfSeeds());
    d->MinimumPathSpinBox->setValue(paramNode->GetMinimumPathLength());
    d->MaximumPathSpinBox->setValue(paramNode->GetMaximumPathLength());
    d->FiducialRegionSpinBox->setValue(paramNode->GetSeedingRegionSize());
    d->FiducialStepSpinBox->setValue(paramNode->GetSeedingRegionStep());
    d->SeedSelectedCheckBox->setChecked(paramNode->GetSeedSelectedFiducials()==1);
    d->StoppingCurvatureSpinBox->setValue(paramNode->GetStoppingCurvature());
    d->StoppingCriteriaComboBox->setCurrentIndex(paramNode->GetStoppingMode());
    d->StoppingValueSpinBox->setValue(paramNode->GetStoppingValue());
    d->DisplayTracksComboBox->setCurrentIndex(paramNode->GetDisplayMode());
    d->ROILabelInput->setText(paramNode->ROILabelsToString().c_str());
    d->RandomGridCheckBox->setChecked(paramNode->GetRandomGrid());
    d->UseIndexSpaceCheckBox->setChecked(paramNode->GetUseIndexSpace());
    d->LinearMeasureStartSlider->setValue(paramNode->GetLinearMeasureStart());
    d->SeedSpacingSlider->setValue(paramNode->GetSeedSpacing());
    d->WriteFibersCheckBox->setChecked(paramNode->GetWriteToFile());
    d->FilePrefixLineEdit->setText(paramNode->GetFilePrefix());
    d->OutputDirectoryButton->setDirectory(paramNode->GetFileDirectoryName());

    d->ParameterNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetID()));
    d->FiberNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetOutputFiberRef()));
    d->FiducialNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()));
    d->DTINodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()));
    }
}


//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setStoppingCurvature(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetStoppingCurvature(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setStoppingValue(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetStoppingValue(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setIntegrationStep(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetIntegrationStep(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setMinimumPath(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetMinimumPathLength(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setMaximumPath(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetMaximumPathLength(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setFiducialRegion(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetSeedingRegionSize(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setFiducialRegionStep(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetSeedingRegionStep(value);
    }
}


//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setStoppingCriteria(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetStoppingMode(value);
    }
}
//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setTrackDisplayMode(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetDisplayMode(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setSeedSelectedFiducials(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetSeedSelectedFiducials(value!=0?1:0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setMaxNumberSeeds(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetMaxNumberOfSeeds(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setEnableSeeding(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetEnableSeeding(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setUseIndexSpace(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetUseIndexSpace(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setSeedSpacing(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetSeedSpacing(value);
    }
}
//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setROILabels()
{
  Q_D(qSlicerTractographyInteractiveSeedingModuleWidget);
  this->setROILabels(d->ROILabelInput->text());
}
//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setROILabels(const QString &labels)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->StringToROILabels(labels.toStdString());
    }
}
//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setRandomGrid(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetRandomGrid(value);
    }
}
//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setLinearMeasureStart(double value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetLinearMeasureStart(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setWriteFibers(int value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetWriteToFile(value);
    }
}


//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setDirectory(const QString &value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetFileDirectoryName(value.toLatin1());
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyInteractiveSeedingModuleWidget::setFilePrefix(const QString &value)
{
  if (this->TractographyInteractiveSeedingNode)
    {
    this->TractographyInteractiveSeedingNode->SetFilePrefix(value.toLatin1());
    }
}
