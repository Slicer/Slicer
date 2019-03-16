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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

#ifndef __qMRMLSegmentationGeometryWidget_h
#define __qMRMLSegmentationGeometryWidget_h

// Segmentations includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkOrientedImageData;
class vtkMRMLNode;
class vtkMRMLSegmentationNode;
class qMRMLSegmentationGeometryWidgetPrivate;

/// \ingroup Slicer_QtModules_Segmentations_Widgets
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentationGeometryWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(double oversamplingFactor READ oversamplingFactor WRITE setOversamplingFactor)
  Q_PROPERTY(bool isotropicSpacing READ isotropicSpacing WRITE setIsotropicSpacing)

public:
  typedef qMRMLWidget Superclass;
  /// Constructor
  explicit qMRMLSegmentationGeometryWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLSegmentationGeometryWidget() override;

  /// Get segmentation MRML node
  Q_INVOKABLE vtkMRMLSegmentationNode* segmentationNode()const;
  Q_INVOKABLE QString segmentationNodeID()const;

  bool editEnabled()const;
  vtkMRMLNode* sourceNode()const;
  double oversamplingFactor()const;
  bool isotropicSpacing()const;

  void setSpacing(double aSpacing[3]);

  /// Get calculated geometry image data
  Q_INVOKABLE void geometryImageData(vtkOrientedImageData* outputGeometry);

public slots:
  /// Set segmentation MRML node
  void setSegmentationNode(vtkMRMLSegmentationNode* node);

  void setEditEnabled(bool aEditEnabled);
  void setSourceNode(vtkMRMLNode* sourceNode);
  void setOversamplingFactor(double aOversamplingFactor);
  void setIsotropicSpacing(bool aIsotropicSpacing);

  /// Set reference geometry conversion parameter to the one specified
  void setReferenceImageGeometryForSegmentationNode();

  /// Resample existing labelmaps in segmentation node with specified geometry
  void resampleLabelmapsInSegmentationNode();

protected slots:
  /// Calculate output geometry from input segmentation and source node and update UI
  void updateWidgetFromMRML();

  /// Calculate source axis permutation and then output geometry.
  void onSourceNodeChanged(vtkMRMLNode*);

  void onOversamplingFactorChanged(double);
  void onIsotropicSpacingChanged(bool);
  void onUserSpacingChanged(double*);

protected:
  QScopedPointer<qMRMLSegmentationGeometryWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentationGeometryWidget);
  Q_DISABLE_COPY(qMRMLSegmentationGeometryWidget);
};

#endif
