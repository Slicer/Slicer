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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by CI3.

==============================================================================*/

#ifndef __qMRMLSliceVerticalControllerWidget_h
#define __qMRMLSliceVerticalControllerWidget_h

// qMRMLWidget includes
#include "qMRMLWidget.h"
#include "qMRMLWidgetsExport.h"
#include <vtkVersion.h>

// CTK includes
#include <ctkVTKObject.h>

class qMRMLSliceVerticalControllerWidgetPrivate;
class qMRMLSliderWidget;
class vtkMRMLScene;
class vtkMRMLSliceLogic;
class vtkMRMLSliceNode;

/// qMRMLSliceVerticalControllerWidget offers controls to a slice view (vtkMRMLSliceNode).
///
/// To be valid, it needs at least a MRML scene and a MRML slice node:
/// \code
/// qMRMLSliceVerticalControllerWidget controllerWidget;
/// controllerWidget.setMRMLScene(scene);
/// controllerWidget.setMRMLSliceNode(sliceNode);
/// \endcode
class QMRML_WIDGETS_EXPORT qMRMLSliceVerticalControllerWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool showSliceOffsetSlider READ showSliceOffsetSlider WRITE setShowSliceOffsetSlider)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  qMRMLSliceVerticalControllerWidget(QWidget* parent = nullptr);
  ~qMRMLSliceVerticalControllerWidget() override;

  /// Get \a sliceNode
  /// \sa setMRMLSliceCompositeNode();
  Q_INVOKABLE vtkMRMLSliceNode* mrmlSliceNode()const;

  /// Set slice offset range
  Q_INVOKABLE void setSliceOffsetRange(double min, double max);

  /// Set slice offset \a resolution (increment)
  void setSliceOffsetResolution(double resolution);

  /// Get slice offset \a resolution (increment)
  double sliceOffsetResolution();

  /// Get SliceLogic
  Q_INVOKABLE vtkMRMLSliceLogic* sliceLogic();

  /// Set \a newSliceLogic
  Q_INVOKABLE void setSliceLogic(vtkMRMLSliceLogic * newSliceLogic);

  /// Get the slice slider widget (shown in the controller bar).
  Q_INVOKABLE qMRMLSliderWidget* sliceVerticalOffsetSlider();

  /// Get the slice offset slider visibility.
  bool showSliceOffsetSlider()const;

public slots:

  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set slice offset slider visibility.
  void setShowSliceOffsetSlider(bool show);

  /// Set slice \a offset. Used to set a single value.
  void setSliceOffsetValue(double offset);

  /// Set slice offset. Used when events will come is rapid succession.
  void trackSliceOffsetValue(double offset);

protected slots:
  void onSliceLogicModifiedEvent();

signals:
  void renderRequested();

protected:
  QScopedPointer<qMRMLSliceVerticalControllerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliceVerticalControllerWidget);
  Q_DISABLE_COPY(qMRMLSliceVerticalControllerWidget);
};

#endif
