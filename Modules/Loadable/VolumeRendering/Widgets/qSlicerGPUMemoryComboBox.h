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

#ifndef __qSlicerGPUMemoryComboBox_h
#define __qSlicerGPUMemoryComboBox_h

// Qt includes
#include <QComboBox>

// Slicer includes
#include "qSlicerVolumeRenderingModuleWidgetsExport.h"

class qSlicerGPUMemoryComboBoxPrivate;

class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerGPUMemoryComboBox
  : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(double currentGPUMemory READ currentGPUMemory WRITE setCurrentGPUMemory)
  Q_PROPERTY(QString currentGPUMemoryString READ currentGPUMemoryAsString WRITE setCurrentGPUMemoryFromString)

public:
  /// Constructors
  typedef QComboBox Superclass;
  explicit qSlicerGPUMemoryComboBox(QWidget* parent=nullptr);
  ~qSlicerGPUMemoryComboBox() override;

  /// Return total memory available in the GPU
  Q_INVOKABLE int totalGPUMemoryInMB()const;
  /// Return currently selected GPU memory in MB or percentage (% value between 0-1)
  Q_INVOKABLE double currentGPUMemory()const;
  /// Return currently selected GPU memory in MB
  Q_INVOKABLE int currentGPUMemoryInMB()const;
  /// Return currently selected GPU memory as string
  Q_INVOKABLE QString currentGPUMemoryAsString()const;

public slots:
  /// Set currently selected GPU memory in MB or percentage (% value between 0-1)
  void setCurrentGPUMemory(double memory);
  /// Set currently selected GPU memory from string
  /// (\sa qSlicerGPUMemoryComboBoxPrivate::memoryFromString)
  void setCurrentGPUMemoryFromString(const QString& memoryString);

protected:
  QScopedPointer<qSlicerGPUMemoryComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerGPUMemoryComboBox);
  Q_DISABLE_COPY(qSlicerGPUMemoryComboBox);
};

#endif
