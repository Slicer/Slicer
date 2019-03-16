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

#ifndef __qSlicerSettingsCachePanel_h
#define __qSlicerSettingsCachePanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>
#include <ctkVTKObject.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerSettingsCachePanelPrivate;
class vtkCacheManager;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsCachePanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsCachePanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSettingsCachePanel() override;

  virtual void setCacheManager(vtkCacheManager* cacheManager);

public slots:
  void setCachePath(const QString& path);
  void setCacheSize(int sizeInMB);
  void setCacheFreeBufferSize(int sizeInMB);
  void setForceRedownload(bool force);
  void clearCache();

protected slots:
  void updateFromCacheManager();

protected:
  QScopedPointer<qSlicerSettingsCachePanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsCachePanel);
  Q_DISABLE_COPY(qSlicerSettingsCachePanel);
};

#endif
