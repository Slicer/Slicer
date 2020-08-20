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
#include <QDebug>
#include <QLineEdit>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerVolumeRenderingSettingsPanel.h"
#include "ui_qSlicerVolumeRenderingSettingsPanel.h"

// MRMLDisplayableManager includes
#include <vtkMRMLVolumeRenderingDisplayableManager.h>

// VolumeRendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// --------------------------------------------------------------------------
// qSlicerVolumeRenderingSettingsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerVolumeRenderingSettingsPanelPrivate: public Ui_qSlicerVolumeRenderingSettingsPanel
{
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingSettingsPanel);
protected:
  qSlicerVolumeRenderingSettingsPanel* const q_ptr;

public:
  qSlicerVolumeRenderingSettingsPanelPrivate(qSlicerVolumeRenderingSettingsPanel& object);
  void init();

  void addRenderingMethod(const QString& methodName, const QString& methodClassName);

  vtkMRMLScene* mrmlScene();
  vtkMRMLViewNode* defaultMrmlViewNode();

  vtkSmartPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;
};

// --------------------------------------------------------------------------
// qSlicerVolumeRenderingSettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerVolumeRenderingSettingsPanelPrivate
::qSlicerVolumeRenderingSettingsPanelPrivate(qSlicerVolumeRenderingSettingsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanelPrivate::init()
{
  Q_Q(qSlicerVolumeRenderingSettingsPanel);

  this->setupUi(q);

  //
  // Quality
  //
  for (int qualityIndex=0; qualityIndex<vtkMRMLViewNode::VolumeRenderingQuality_Last; qualityIndex++)
    {
    this->QualityControlComboBox->addItem(vtkMRMLViewNode::GetVolumeRenderingQualityAsString(qualityIndex));
    }
  QObject::connect(this->QualityControlComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onDefaultQualityChanged(int)));
  q->registerProperty("VolumeRendering/DefaultQuality", q,
                      "defaultQuality", SIGNAL(defaultQualityChanged(QString)));

  //
  // Interactive speed
  //
  QObject::connect(this->InteractiveSpeedSlider, SIGNAL(valueChanged(double)),
                   q, SLOT(onDefaultInteractiveSpeedChanged(double)));
  q->registerProperty("VolumeRendering/DefaultInteractiveSpeed", q,
                      "defaultInteractiveSpeed", SIGNAL(defaultInteractiveSpeedChanged(int)));

  //
  // Surface smoothing
  //
  QObject::connect(this->SurfaceSmoothingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onDefaultSurfaceSmoothingChanged(bool)));
  q->registerProperty("VolumeRendering/DefaultSurfaceSmoothing", q,
                      "defaultSurfaceSmoothing", SIGNAL(defaultSurfaceSmoothingChanged(bool)));

  //
  // GPU memory
  //

  // Currently, VTK ignores GPU memory size request - hide it on the GUI to not confuse users
  this->GPUMemoryLabel->hide();
  this->GPUMemoryComboBox->hide();

  QObject::connect(this->GPUMemoryComboBox, SIGNAL(editTextChanged(QString)),
                   q, SLOT(onGPUMemoryChanged()));
  QObject::connect(this->GPUMemoryComboBox, SIGNAL(currentTextChanged(QString)),
                   q, SLOT(onGPUMemoryChanged()));

  q->registerProperty("VolumeRendering/GPUMemorySize", q,
                      "gpuMemory", SIGNAL(gpuMemoryChanged(QString)));

  // Update default view node from settings when startup completed.
  // MRML scene is not accessible yet from the logic when it is set, so cannot access default view node
  // either. Need to setup default node and set defaults to 3D views when the scene is available.
  QObject::connect(qSlicerApplication::application(), SIGNAL(startupCompleted()),
                   q, SLOT(updateDefaultViewNodeFromWidget()));
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanelPrivate::addRenderingMethod(
  const QString& methodName, const QString& methodClassName )
{
  this->RenderingMethodComboBox->addItem(methodName, methodClassName);
}

// --------------------------------------------------------------------------
vtkMRMLScene* qSlicerVolumeRenderingSettingsPanelPrivate::mrmlScene()
{
  Q_Q(qSlicerVolumeRenderingSettingsPanel);

  vtkSlicerVolumeRenderingLogic* logic = q->volumeRenderingLogic();
  if (!logic)
    {
    return nullptr;
    }
  return logic->GetMRMLScene();
}

// --------------------------------------------------------------------------
vtkMRMLViewNode* qSlicerVolumeRenderingSettingsPanelPrivate::defaultMrmlViewNode()
{
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return nullptr;
    }

  // Setup a default 3D view node so that the default settings are propagated to all new 3D views
  vtkSmartPointer<vtkMRMLNode> defaultNode = scene->GetDefaultNodeByClass("vtkMRMLViewNode");
  if (!defaultNode)
    {
    defaultNode.TakeReference(scene->CreateNodeByClass("vtkMRMLViewNode"));
    scene->AddDefaultNode(defaultNode);
    }
  return vtkMRMLViewNode::SafeDownCast(defaultNode.GetPointer());
}

// --------------------------------------------------------------------------
// qSlicerVolumeRenderingSettingsPanel methods

// --------------------------------------------------------------------------
qSlicerVolumeRenderingSettingsPanel::qSlicerVolumeRenderingSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeRenderingSettingsPanelPrivate(*this))
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerVolumeRenderingSettingsPanel::~qSlicerVolumeRenderingSettingsPanel() = default;

// --------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* qSlicerVolumeRenderingSettingsPanel::volumeRenderingLogic()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  return d->VolumeRenderingLogic;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);

  qvtkReconnect(d->VolumeRenderingLogic, logic, vtkCommand::ModifiedEvent,
                this, SLOT(onVolumeRenderingLogicModified()));
  d->VolumeRenderingLogic = logic;

  this->onVolumeRenderingLogicModified();

  this->registerProperty("VolumeRendering/RenderingMethod", this,
                         "defaultRenderingMethod", SIGNAL(defaultRenderingMethodChanged(QString)));
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onVolumeRenderingLogicModified()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);

  // Update default rendering method
  const std::map<std::string, std::string>& renderingMethods =
    d->VolumeRenderingLogic->GetRenderingMethods();
  /// \todo not the best test to make sure the list is different
  if (static_cast<int>(renderingMethods.size()) != d->RenderingMethodComboBox->count())
    {
    std::map<std::string, std::string>::const_iterator it;
    for (it = renderingMethods.begin(); it != renderingMethods.end(); ++it)
      {
      d->addRenderingMethod(it->first.c_str(), it->second.c_str());
      }
    }
  QObject::connect(d->RenderingMethodComboBox, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onDefaultRenderingMethodChanged(int)),Qt::UniqueConnection);

  const char* defaultRenderingMethod = d->VolumeRenderingLogic->GetDefaultRenderingMethod();
  if (defaultRenderingMethod == nullptr)
    {
    defaultRenderingMethod = "vtkMRMLCPURayCastVolumeRenderingDisplayNode";
    }
  int defaultRenderingMethodIndex = d->RenderingMethodComboBox->findData(
    QString(defaultRenderingMethod));
  d->RenderingMethodComboBox->setCurrentIndex(defaultRenderingMethodIndex);
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingSettingsPanel::gpuMemory()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  return d->GPUMemoryComboBox->currentGPUMemoryAsString();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setGPUMemory(const QString& gpuMemoryString)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  d->GPUMemoryComboBox->setCurrentGPUMemoryFromString(gpuMemoryString);
  this->onGPUMemoryChanged();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onGPUMemoryChanged()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (!d->mrmlScene())
    {
    return;
    }

  int memory = d->GPUMemoryComboBox->currentGPUMemoryInMB();

  // Set to default view node
  vtkMRMLViewNode* defaultViewNode = d->defaultMrmlViewNode();
  if (defaultViewNode)
    {
    defaultViewNode->SetGPUMemorySize(memory);
    }

  // Set to all existing view nodes
  std::vector<vtkMRMLNode*> viewNodes;
  d->mrmlScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    viewNode->SetGPUMemorySize(memory);
    }

  emit gpuMemoryChanged(this->gpuMemory());
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingSettingsPanel::defaultRenderingMethod()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  QString renderingClassName =
    d->RenderingMethodComboBox->itemData(d->RenderingMethodComboBox->currentIndex()).toString();
  return renderingClassName;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setDefaultRenderingMethod(const QString& method)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  int methodIndex = d->RenderingMethodComboBox->findData(method);
  d->RenderingMethodComboBox->setCurrentIndex(methodIndex);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onDefaultRenderingMethodChanged(int index)
{
  Q_UNUSED(index);
  this->updateVolumeRenderingLogicDefaultRenderingMethod();
  emit defaultRenderingMethodChanged(this->defaultRenderingMethod());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::updateVolumeRenderingLogicDefaultRenderingMethod()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (d->VolumeRenderingLogic == nullptr)
    {
    return;
    }
  d->VolumeRenderingLogic->SetDefaultRenderingMethod(this->defaultRenderingMethod().toUtf8());
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingSettingsPanel::defaultQuality()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  int qualityIndex = d->QualityControlComboBox->currentIndex();
  QString quality(vtkMRMLViewNode::GetVolumeRenderingQualityAsString(qualityIndex));
  return quality;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setDefaultQuality(const QString& quality)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  int qualityIndex = d->QualityControlComboBox->findText(quality);
  d->QualityControlComboBox->setCurrentIndex(qualityIndex);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onDefaultQualityChanged(int qualityIndex)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (!d->mrmlScene())
    {
    return;
    }

  // Set to default view node
  vtkMRMLViewNode* defaultViewNode = d->defaultMrmlViewNode();
  if (defaultViewNode)
    {
    defaultViewNode->SetVolumeRenderingQuality(qualityIndex);
    }

  // Set to all existing view nodes
  std::vector<vtkMRMLNode*> viewNodes;
  d->mrmlScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    viewNode->SetVolumeRenderingQuality(qualityIndex);
    }

  QString quality(vtkMRMLViewNode::GetVolumeRenderingQualityAsString(qualityIndex));
  emit defaultQualityChanged(quality);
}

// --------------------------------------------------------------------------
int qSlicerVolumeRenderingSettingsPanel::defaultInteractiveSpeed()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  int interactiveSpeed = d->InteractiveSpeedSlider->value();
  return interactiveSpeed;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setDefaultInteractiveSpeed(int interactiveSpeed)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  d->InteractiveSpeedSlider->setValue(interactiveSpeed);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onDefaultInteractiveSpeedChanged(double interactiveSpeed)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (!d->mrmlScene())
    {
    return;
    }

  // Set to default view node
  vtkMRMLViewNode* defaultViewNode = d->defaultMrmlViewNode();
  if (defaultViewNode)
    {
    defaultViewNode->SetExpectedFPS((int)interactiveSpeed);
    }

  // Set to all existing view nodes
  std::vector<vtkMRMLNode*> viewNodes;
  d->mrmlScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    viewNode->SetExpectedFPS((int)interactiveSpeed);
    }

  emit defaultInteractiveSpeedChanged((int)interactiveSpeed);
}

// --------------------------------------------------------------------------
bool qSlicerVolumeRenderingSettingsPanel::defaultSurfaceSmoothing()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  bool smoothing = d->SurfaceSmoothingCheckBox->isChecked();
  return smoothing;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setDefaultSurfaceSmoothing(bool surfaceSmoothing)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  d->SurfaceSmoothingCheckBox->setChecked(surfaceSmoothing);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onDefaultSurfaceSmoothingChanged(bool smoothing)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (!d->mrmlScene())
    {
    return;
    }

  // Set to default view node
  vtkMRMLViewNode* defaultViewNode = d->defaultMrmlViewNode();
  if (defaultViewNode)
    {
    defaultViewNode->SetVolumeRenderingSurfaceSmoothing(smoothing);
    }

  // Set to all existing view nodes
  std::vector<vtkMRMLNode*> viewNodes;
  d->mrmlScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    viewNode->SetVolumeRenderingSurfaceSmoothing(smoothing);
    }

  emit defaultSurfaceSmoothingChanged(smoothing);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::updateDefaultViewNodeFromWidget()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);

  this->onDefaultQualityChanged(d->QualityControlComboBox->currentIndex());
  this->onDefaultInteractiveSpeedChanged(d->InteractiveSpeedSlider->value());
  this->onDefaultSurfaceSmoothingChanged(d->SurfaceSmoothingCheckBox->isChecked());
  this->onGPUMemoryChanged();
}
