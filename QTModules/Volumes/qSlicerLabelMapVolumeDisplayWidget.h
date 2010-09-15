/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerLabelMapVolumeDisplayWidget_h
#define __qSlicerLabelMapVolumeDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

#include "qSlicerVolumesModuleExport.h"

class vtkMRMLNode;
class vtkMRMLLabelMapVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qSlicerLabelMapVolumeDisplayWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerLabelMapVolumeDisplayWidget : public qSlicerWidget
{
  Q_OBJECT

public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerLabelMapVolumeDisplayWidget(QWidget* parent);
  virtual ~qSlicerLabelMapVolumeDisplayWidget();

  vtkMRMLScalarVolumeNode* volumeNode()const;
  vtkMRMLLabelMapVolumeDisplayNode* volumeDisplayNode()const;
public slots:

  ///
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setColorNode(vtkMRMLNode* colorNode);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerLabelMapVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerLabelMapVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerLabelMapVolumeDisplayWidget);
};

#endif
