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
#include <vtkGPUInfo.h>
#include <vtkGPUInfoList.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

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

  int memoryFromString(const QString& memory)const;
  QString memoryToString(int memory)const;

  void addRenderingMethod(const QString& methodName, const QString& methodClassName);

  QRegExp MemoryRegExp;
  vtkSmartPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;
};

// --------------------------------------------------------------------------
// qSlicerVolumeRenderingSettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerVolumeRenderingSettingsPanelPrivate
::qSlicerVolumeRenderingSettingsPanelPrivate(qSlicerVolumeRenderingSettingsPanel& object)
  :q_ptr(&object)
{
  this->MemoryRegExp = QRegExp("^(\\d+(?:\\.\\d*)?)\\s?(Mo|Go)$");
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanelPrivate::init()
{
  Q_Q(qSlicerVolumeRenderingSettingsPanel);

  this->setupUi(q);

  this->GPUMemoryComboBox->setEditable(true);
  this->GPUMemoryComboBox->lineEdit()->setValidator(
    new QRegExpValidator(this->MemoryRegExp, q));
  this->GPUMemoryComboBox->insertItem(0, q->tr("0 Mo"));
  this->GPUMemoryComboBox->insertSeparator(1);

  QObject::connect(this->GPUMemoryComboBox, SIGNAL(editTextChanged(QString)),
                   q, SLOT(onGPUMemoryChanged()));
  QObject::connect(this->GPUMemoryComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SLOT(onGPUMemoryChanged()));

  // Detect the amount of memory in the graphic card
  vtkNew<vtkGPUInfoList> gpuInfoList;
  gpuInfoList->Probe();

  if (gpuInfoList->GetNumberOfGPUs() > 0)
    {
    int gpuMemoryInBytes = gpuInfoList->GetGPUInfo(0)->GetDedicatedVideoMemory();
    int gpuMemoryInKo = gpuMemoryInBytes / 1024;
    int gpuMemoryInMo = gpuMemoryInKo / 1024;
    // Set it as the default amount of memory
    q->setGPUMemory(gpuMemoryInMo);
    }

  q->registerProperty("VolumeRendering/GPUMemorySize", q, "gpuMemory",
                      SIGNAL(gpuMemoryChanged(int)));
}

// --------------------------------------------------------------------------
int qSlicerVolumeRenderingSettingsPanelPrivate::memoryFromString(const QString& memory)const
{
  int pos = this->MemoryRegExp.indexIn(memory);
  if (pos < 0)
    {
    return 0;
    }

  QString memoryValue = this->MemoryRegExp.cap(1);
  QString memoryUnit = this->MemoryRegExp.cap(2);

  double value = memoryValue.toDouble();
  double unit = memoryUnit == "Mo" ? 1. : 1024;

  return static_cast<int>(value * unit);
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingSettingsPanelPrivate::memoryToString(int memory)const
{
  QString value = QString::number(memory) + " Mo";
  if (memory > 1024)
    {
    value = QString::number(static_cast<float>(memory) / 1024) + " Go";
    }
  return value;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanelPrivate
::addRenderingMethod(const QString& methodName, const QString& methodClassName)
{
  this->RenderingMethodComboBox->addItem(
    methodName, methodClassName);
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
qSlicerVolumeRenderingSettingsPanel::~qSlicerVolumeRenderingSettingsPanel()
{
}

// --------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* qSlicerVolumeRenderingSettingsPanel
::volumeRenderingLogic()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  return d->VolumeRenderingLogic;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel
::setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
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
void qSlicerVolumeRenderingSettingsPanel
::onVolumeRenderingLogicModified()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
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
  if (defaultRenderingMethod == 0)
    {
    defaultRenderingMethod = "vtkMRMLCPURayCastVolumeRenderingDisplayNode";
    }
  int defaultRenderingMethodIndex = d->RenderingMethodComboBox->findData(
    QString(defaultRenderingMethod));
  d->RenderingMethodComboBox->setCurrentIndex(defaultRenderingMethodIndex);
}

// --------------------------------------------------------------------------
int qSlicerVolumeRenderingSettingsPanel::gpuMemory()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  QString memory = d->GPUMemoryComboBox->currentText();
  return d->memoryFromString(memory);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::setGPUMemory(int gpuMemory)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  QString value = d->memoryToString(gpuMemory);
  int index = d->GPUMemoryComboBox->findText(value);
  bool currentIndexModified = false;
  if (index == -1)
    {
    int customIndex = 0;
    d->GPUMemoryComboBox->setItemText(customIndex, value);
    index = customIndex;
    if (index == d->GPUMemoryComboBox->currentIndex())
      {
      currentIndexModified = true;
      }
    }
  d->GPUMemoryComboBox->setCurrentIndex(index);
  if (currentIndexModified)
    {
    this->onGPUMemoryChanged();
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel::onGPUMemoryChanged()
{
  vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize =
    this->gpuMemory();

  emit gpuMemoryChanged(this->gpuMemory());
  // Todo:
  // update all the VolumeRendering displayable manager to take the new memory
  // into account.
  //vtkMRMLThreeDViewDisplayableManagerFactory* factory
  //  = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingSettingsPanel
::defaultRenderingMethod()const
{
  Q_D(const qSlicerVolumeRenderingSettingsPanel);
  QString renderingClassName =
    d->RenderingMethodComboBox->itemData(
      d->RenderingMethodComboBox->currentIndex()).toString();
  return renderingClassName;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel
::setDefaultRenderingMethod(const QString& method)
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  int methodIndex = d->RenderingMethodComboBox->findData(method);
  d->RenderingMethodComboBox->setCurrentIndex(methodIndex);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel
::onDefaultRenderingMethodChanged(int index)
{
  Q_UNUSED(index);
  this->updateVolumeRenderingLogicDefaultRenderingMethod();
  emit defaultRenderingMethodChanged(this->defaultRenderingMethod());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingSettingsPanel
::updateVolumeRenderingLogicDefaultRenderingMethod()
{
  Q_D(qSlicerVolumeRenderingSettingsPanel);
  if (d->VolumeRenderingLogic == 0)
    {
    return;
    }
  d->VolumeRenderingLogic->SetDefaultRenderingMethod(
    this->defaultRenderingMethod().toLatin1());
}
