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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerReformatModuleWidget_h
#define __qSlicerReformatModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerReformatModuleExport.h"

class qSlicerReformatModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Reformat
class Q_SLICER_QTMODULES_Reformat_EXPORT
qSlicerReformatModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerReformatModuleWidget(QWidget *parent=0);
  virtual ~qSlicerReformatModuleWidget();

  enum OriginReferenceType {ONPLANE, INVOLUME};

protected:
    virtual void setup();

public slots:
  /// Set slice \a offset. Used to set a single value.
  void setSliceOffsetValue(double offset);

  /// Recenter the active node given its coordinates reference.
  void centerSliceNode();

protected slots:
  /// Triggered upon MRML transform node updates
  void onMRMLSliceNodeModified(vtkObject* caller);

  /// Set slice offset. Used when events will come is rapid succession.
  void onTrackSliceOffsetValueChanged(double offset);

  void onNodeSelected(vtkMRMLNode* node);
  void onSliceVisibilityChanged(bool visible);
  void onReformatWidgetVisibilityChanged(bool visible);

  void onOriginCoordinateReferenceButtonPressed(int reference);
  void onWorldPositionChanged();

  void onSliceNormalChanged();
  void onSliceOrientationChanged(const QString& orientation);
  void onSliderRotationChanged(double rotationX);

protected:
  QScopedPointer<qSlicerReformatModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerReformatModuleWidget);
  Q_DISABLE_COPY(qSlicerReformatModuleWidget);
};

#endif
