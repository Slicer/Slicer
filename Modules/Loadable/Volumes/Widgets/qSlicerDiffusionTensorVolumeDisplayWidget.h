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

#ifndef __qSlicerDiffusionTensorVolumeDisplayWidget_h
#define __qSlicerDiffusionTensorVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLDiffusionTensorVolumeDisplayNode;
class vtkMRMLDiffusionTensorVolumeNode;
class vtkMRMLDiffusionTensorSliceDisplayNode;
class vtkMRMLGlyphableVolumeSliceDisplayNode;
class qSlicerDiffusionTensorVolumeDisplayWidgetPrivate;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT qSlicerDiffusionTensorVolumeDisplayWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerDiffusionTensorVolumeDisplayWidget(QWidget* parent = nullptr);
  ~qSlicerDiffusionTensorVolumeDisplayWidget() override;

  vtkMRMLDiffusionTensorVolumeNode* volumeNode()const;
  vtkMRMLDiffusionTensorVolumeDisplayNode* volumeDisplayNode()const;
  QList<vtkMRMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes()const;
public slots:

  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLDiffusionTensorVolumeNode* volumeNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setVolumeScalarInvariant(int scalarInvariant);
  void setRedSliceVisible(bool visible);
  void setYellowSliceVisible(bool visible);
  void setGreenSliceVisible(bool visible);
protected slots:
  void updateWidgetFromMRML();
  void synchronizeSliceDisplayNodes();

protected:
  QScopedPointer<qSlicerDiffusionTensorVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDiffusionTensorVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerDiffusionTensorVolumeDisplayWidget);
};

#endif
