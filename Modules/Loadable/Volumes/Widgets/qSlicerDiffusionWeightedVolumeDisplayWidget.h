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

#ifndef __qSlicerDiffusionWeightedVolumeDisplayWidget_h
#define __qSlicerDiffusionWeightedVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLDiffusionWeightedVolumeDisplayNode;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMRMLDiffusionWeightedSliceDisplayNode;
class vtkMRMLGlyphableVolumeSliceDisplayNode;
class qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT qSlicerDiffusionWeightedVolumeDisplayWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerDiffusionWeightedVolumeDisplayWidget(QWidget* parent = nullptr);
  ~qSlicerDiffusionWeightedVolumeDisplayWidget() override;

  vtkMRMLDiffusionWeightedVolumeNode* volumeNode()const;
  vtkMRMLDiffusionWeightedVolumeDisplayNode* volumeDisplayNode()const;
  QList<vtkMRMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes()const;
public slots:

  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLDiffusionWeightedVolumeNode* volumeNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setDWIComponent(int component);
protected slots:
  void updateWidgetFromVolumeNode();
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDiffusionWeightedVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerDiffusionWeightedVolumeDisplayWidget);
};

#endif
