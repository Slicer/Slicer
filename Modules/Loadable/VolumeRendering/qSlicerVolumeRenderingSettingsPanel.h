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

#ifndef __qSlicerVolumeRenderingSettingsPanel_h
#define __qSlicerVolumeRenderingSettingsPanel_h

// CTK includes
#include <ctkSettingsPanel.h>

// VolumeRendering includes
#include "qSlicerVolumeRenderingModuleExport.h"
class qSlicerVolumeRenderingSettingsPanelPrivate;
class vtkSlicerVolumeRenderingLogic;

class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingSettingsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  Q_PROPERTY(int gpuMemory READ gpuMemory WRITE setGPUMemory NOTIFY gpuMemoryChanged)
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerVolumeRenderingSettingsPanel(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerVolumeRenderingSettingsPanel();

  /// Volume rendering logic is synchronized with the settings.
  /// \sa vtkSlicerVolumeRenderingLogic::SetDefaultRenderingMethod
  void setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic);
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic()const;

  int gpuMemory()const;
  void setGPUMemory(int gpuMemory);

signals:
  void gpuMemoryChanged(int);

protected slots:
  void onGPUMemoryChanged();
  void updateVolumeRenderingLogicDefaultRenderingMethod();

protected:
  QScopedPointer<qSlicerVolumeRenderingSettingsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingSettingsPanel);
  Q_DISABLE_COPY(qSlicerVolumeRenderingSettingsPanel);
};

#endif
