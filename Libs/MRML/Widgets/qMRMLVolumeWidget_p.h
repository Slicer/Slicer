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

#ifndef __qMRMLVolumeWidget_p_h
#define __qMRMLVolumeWidget_p_h

// qMRML includes
#include "qMRMLVolumeWidget.h"

class ctkPopupWidget;
class ctkRangeWidget;

// -----------------------------------------------------------------------------
class qMRMLVolumeWidgetPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qMRMLVolumeWidget);
protected:
  qMRMLVolumeWidget* const q_ptr;

public:
  qMRMLVolumeWidgetPrivate(qMRMLVolumeWidget& object);
  virtual ~qMRMLVolumeWidgetPrivate();

  virtual void init();

  /// Update the range and single step of the input GUI elements such as
  /// sliders and spinboxes.
  void updateRangeForVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode*);
  /// Block all the signals emitted by the widgets that are impacted by the
  /// range.
  /// To be reimplemented in subclasses
  virtual bool blockSignals(bool block);
  /// Compute the scalar range of the volume display node.
  /// It can then be used to set the range of the sliders, spinboxes, etc.
  void scalarRange(vtkMRMLScalarVolumeDisplayNode* displayNode, double range[2]);
  /// Compute the ideal singleStep based on a range.
  /// Set the single step to the widgets such as sliders, spinboxes...
  void updateSingleStep(double min, double max);

public slots:
  virtual void setRange(double min, double max);
  virtual void setDecimals(int decimals);
  virtual void setSingleStep(double singleStep);

protected:
  vtkMRMLScalarVolumeNode* VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
  ctkPopupWidget* PopupWidget;
  ctkRangeWidget* RangeWidget;
  double DisplayScalarRange[2];
};

#endif
