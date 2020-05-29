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

#ifndef __qSlicerDTISliceDisplayWidget_h
#define __qSlicerDTISliceDisplayWidget_h

// Qt includes

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include <qSlicerWidget.h>
#include "qSlicerVolumesModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLDiffusionTensorVolumeSliceDisplayNode;
class vtkMRMLDiffusionTensorDisplayPropertiesNode;
class qSlicerDTISliceDisplayWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT qSlicerDTISliceDisplayWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool visibilityHidden READ isVisibilityHidden WRITE setVisibilityHidden )

public:
  /// Constructors
  typedef qSlicerWidget Superclass;
  explicit qSlicerDTISliceDisplayWidget(QWidget* parent=nullptr);
  ~qSlicerDTISliceDisplayWidget() override;

  vtkMRMLDiffusionTensorVolumeSliceDisplayNode* displayNode()const;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* displayPropertiesNode()const;

  /// True by default
  bool isVisibilityHidden()const;
  void setVisibilityHidden(bool hide);

public slots:
  /// Set the MRML node of interest
  void setMRMLDTISliceDisplayNode(vtkMRMLDiffusionTensorVolumeSliceDisplayNode* displayNode);
  /// Utility function to easily connect signals/slots
  void setMRMLDTISliceDisplayNode(vtkMRMLNode* displayNode);

  void setColorGlyphBy(int);
  void setColorMap(vtkMRMLNode* colorNode);
  void setOpacity(double);
  void setVisibility(bool);
  void setManualScalarRange(bool);
  void setScalarRange(double, double);
  void setGlyphGeometry(int);
  void setGlyphScaleFactor(double);
  void setGlyphSpacing(double);
  void setGlyphEigenVector(int);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerDTISliceDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDTISliceDisplayWidget);
  Q_DISABLE_COPY(qSlicerDTISliceDisplayWidget);
};

#endif
