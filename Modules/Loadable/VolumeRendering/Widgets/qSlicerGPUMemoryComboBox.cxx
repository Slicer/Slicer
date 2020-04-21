/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// qSlicerVolumeRendering includes
#include "qSlicerGPUMemoryComboBox.h"

// VTK includes
#include <vtkNew.h>
#include <vtkGPUInfo.h>
#include <vtkGPUInfoList.h>

// Qt includes
#include <QDebug>
#include <QLineEdit>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerGPUMemoryComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qSlicerGPUMemoryComboBox);
protected:
  qSlicerGPUMemoryComboBox* const q_ptr;

public:
  qSlicerGPUMemoryComboBoxPrivate(qSlicerGPUMemoryComboBox& object);
  virtual ~qSlicerGPUMemoryComboBoxPrivate();

  void init();

  double memoryFromString(const QString& memory)const;
  QString memoryToString(double memory)const;

  QRegExp MemoryRegExp;
  QString DefaultText;
};

//-----------------------------------------------------------------------------
// qSlicerGPUMemoryComboBoxPrivate methods

//-----------------------------------------------------------------------------
qSlicerGPUMemoryComboBoxPrivate::qSlicerGPUMemoryComboBoxPrivate(
  qSlicerGPUMemoryComboBox& object)
  : q_ptr(&object)
  , DefaultText("0 MB (Default)")
{
  this->MemoryRegExp = QRegExp("^(\\d+(?:\\.\\d*)?)\\s?(MB|GB|\\%)$");
}

//-----------------------------------------------------------------------------
qSlicerGPUMemoryComboBoxPrivate::~qSlicerGPUMemoryComboBoxPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerGPUMemoryComboBoxPrivate::init()
{
  Q_Q(qSlicerGPUMemoryComboBox);

  q->setEditable(true);
  q->lineEdit()->setValidator( new QRegExpValidator(this->MemoryRegExp, q));
  q->addItem(DefaultText);
  //q->addItem(qSlicerGPUMemoryComboBox::tr("25 %")); //TODO: Uncomment when totalGPUMemoryInMB works
  //q->addItem(qSlicerGPUMemoryComboBox::tr("50 %"));
  //q->addItem(qSlicerGPUMemoryComboBox::tr("75 %"));
  //q->addItem(qSlicerGPUMemoryComboBox::tr("90 %"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("128 MB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("256 MB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("512 MB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("1024 MB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("1.5 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("2 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("3 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("4 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("6 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("8 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("12 GB"));
  q->addItem(qSlicerGPUMemoryComboBox::tr("16 GB"));
  q->insertSeparator(1);

  // Detect the amount of memory in the graphic card and set it as default
  int gpuMemoryInMB = q->totalGPUMemoryInMB();
  if (gpuMemoryInMB > 0)
    {
    q->setCurrentGPUMemory(gpuMemoryInMB);
    }
}

// --------------------------------------------------------------------------
double qSlicerGPUMemoryComboBoxPrivate::memoryFromString(const QString& memory)const
{
  if (memory == this->DefaultText)
    {
    return 0.0;
    }

  int pos = this->MemoryRegExp.indexIn(memory);
  if (pos < 0)
    {
    return 0.0;
    }

  QString memoryValue = this->MemoryRegExp.cap(1);
  double value = memoryValue.toDouble();
  QString memoryUnit = this->MemoryRegExp.cap(2);

  if (memoryUnit == "%")
    {
    return value / 100.0;
    }
  else if (memoryUnit == "GB")
    {
    return value * 1024.0;
    }
  return value;
}

// --------------------------------------------------------------------------
QString qSlicerGPUMemoryComboBoxPrivate::memoryToString(double memory)const
{
  if (memory == 0.0)
    {
    return this->DefaultText;
    }
  if (memory < 1.0)
    {
    return QString::number(static_cast<int>(memory * 100)) + " %";
    }
  if (memory > 1024.0)
    {
    return QString::number(static_cast<float>(memory) / 1024) + " GB";
    }
  return QString::number(static_cast<int>(memory)) + " MB";
}


//-----------------------------------------------------------------------------
// qSlicerGPUMemoryComboBox methods

// --------------------------------------------------------------------------
qSlicerGPUMemoryComboBox::qSlicerGPUMemoryComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerGPUMemoryComboBoxPrivate(*this))
{
  Q_D(qSlicerGPUMemoryComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerGPUMemoryComboBox::~qSlicerGPUMemoryComboBox() = default;

//-----------------------------------------------------------------------------
int qSlicerGPUMemoryComboBox::totalGPUMemoryInMB()const
{
  // Detect the amount of memory in the graphic card
  vtkNew<vtkGPUInfoList> gpuInfoList;
  gpuInfoList->Probe();

  if (gpuInfoList->GetNumberOfGPUs() > 0)
    {
    int gpuMemoryInBytes = gpuInfoList->GetGPUInfo(0)->GetDedicatedVideoMemory();
    int gpuMemoryInKB = gpuMemoryInBytes / 1024;
    int gpuMemoryInMB = gpuMemoryInKB / 1024;
    return gpuMemoryInMB;
    }

  return 0;
}

// --------------------------------------------------------------------------
double qSlicerGPUMemoryComboBox::currentGPUMemory()const
{
  Q_D(const qSlicerGPUMemoryComboBox);

  QString memoryString = this->currentText();
  return d->memoryFromString(memoryString);
}

// --------------------------------------------------------------------------
int qSlicerGPUMemoryComboBox::currentGPUMemoryInMB()const
{
  Q_D(const qSlicerGPUMemoryComboBox);

  QString memoryString = this->currentText();
  if (memoryString == d->DefaultText)
    {
    return 0;
    }
  double memory = d->memoryFromString(memoryString);
  if (memory < 1.0)
    {
    int gpuMemoryInMB = this->totalGPUMemoryInMB();
    if (gpuMemoryInMB == 0)
      {
      return 0;
      }
    return static_cast<int>(memory * gpuMemoryInMB);
    }
  return static_cast<int>(memory);
}

// --------------------------------------------------------------------------
QString qSlicerGPUMemoryComboBox::currentGPUMemoryAsString()const
{
  return this->currentText();
}

// --------------------------------------------------------------------------
void qSlicerGPUMemoryComboBox::setCurrentGPUMemory(double memory)
{
  Q_D(qSlicerGPUMemoryComboBox);

  QString memoryString = d->memoryToString(memory);
  this->setCurrentGPUMemoryFromString(memoryString);
}

// --------------------------------------------------------------------------
void qSlicerGPUMemoryComboBox::setCurrentGPUMemoryFromString(const QString& memoryString)
{
  int index = this->findText(memoryString);
  if (index == -1)
    {
    int customIndex = 0;
    this->setItemText(customIndex, memoryString);
    index = customIndex;
    }
  this->setCurrentIndex(index);
}
