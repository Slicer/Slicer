/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSliceControllerWidget_h
#define __qMRMLSliceControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class QButtonGroup;
class qMRMLSliceControllerWidgetPrivate;
class vtkCollection;
class vtkImageData;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSliceCompositeNode;
class vtkMRMLSliceLogic;
class vtkMRMLSliceNode;

///
/// qMRMLSliceControllerWidget offers controls to a slice view (vtkMRMLSliceNode
///  and vtkMRMLSliceCompositeNode). It internally creates a slice logic that
/// be changed. 
class QMRML_WIDGETS_EXPORT qMRMLSliceControllerWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceViewName READ sliceViewName WRITE setSliceViewName)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLSliceControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLSliceControllerWidget();

  /// Are the slices linked to each other
  bool isLinked()const;

  /// Is the view a compare view
  bool isCompareView()const;

  /// Get slice orientation
  /// \sa setSliceOrientation(QString);
  QString sliceOrientation()const;

  /// Get imageData
  vtkImageData* imageData()const;

  /// Get \a sliceNode
  /// \sa setMRMLSliceCompositeNode();
  vtkMRMLSliceNode* mrmlSliceNode()const;

  /// Get sliceCompositeNode
  /// \sa vtkMRMLSliceLogic::GetSliceCompositeNode();
  vtkMRMLSliceCompositeNode* mrmlSliceCompositeNode()const;

  /// Set slice view name
  /// \note SliceViewName should be set before setMRMLSliceNode() is called
  void setSliceViewName(const QString& newSliceViewName);

  /// Get slice view name
  QString sliceViewName()const;

  /// Set slice offset range
  void setSliceOffsetRange(double min, double max);

  /// Set slice offset \a resolution (increment)
  void setSliceOffsetResolution(double resolution);

  /// Get SliceLogic
  vtkMRMLSliceLogic* sliceLogic()const;

  /// Set \a newSliceLogic
  /// Use if two instances of the controller need to observe the same logic.
  void setSliceLogic(vtkMRMLSliceLogic * newSliceLogic);

  /// Set controller widget group
  /// All controllers of a same group will be set visible or hidden if at least
  /// one of the sliceCollapsibleButton of the group is clicked.
  void setControllerButtonGroup(QButtonGroup* group);

  /// TODO:
  /// Ideally the slice logics should be retrieved by the sliceLogic
  /// until then, we manually set them.
  void setSliceLogics(vtkCollection* logics);

public slots:

  virtual void setMRMLScene(vtkMRMLScene* newScene);

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set a new imageData.
  void setImageData(vtkImageData* newImageData);

  /// \sa fitSliceToBackground();
  void setSliceViewSize(const QSize& newSize);

  /// Fit slices to background. A No-op if no SliceView has been set.
  /// \sa setSliceView();
  void fitSliceToBackground();

  /// Set slice orientation.
  /// \note Orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat".
  void setSliceOrientation(const QString& orientation);

  /// Set slice \a offset.
  void setSliceOffsetValue(double offset);

  /// Set slice visible.
  void setSliceVisible(bool visible);

  /// Link/Unlink the slice controls across all slice viewer
  void setSliceLink(bool linked);

  // Advanced options
  /// Rotate to volume plane
  void rotateSliceToBackground();
  /// Label opacity
  void setLabelOpacity(double opacity);
  /// Label outline
  void showLabelOutline(bool show);
  /// Reformat widget
  void showReformatWidget(bool show);
  /// Compositing
  void setCompositing(int mode);
  void setCompositingToAlphaBlend();
  void setCompositingToReverseAlphaBlend();
  void setCompositingToAdd();
  void setCompositingToSubtract();
  /// Slice spacing
  void setSliceSpacingMode(bool automatic);
  void setSliceSpacing(double);
  // Lightbox
  void setLightbox(int rows, int columns);
  void setLightboxTo1x1();
  void setLightboxTo1x2();
  void setLightboxTo1x3();
  void setLightboxTo1x4();
  void setLightboxTo1x6();
  void setLightboxTo1x8();
  void setLightboxTo2x2();
  void setLightboxTo3x3();
  void setLightboxTo6x6();
  // interpolation
  void setForegroundInterpolation(bool);
  void setBackgroundInterpolation(bool);
signals:

  /// This signal is emitted when the given \a imageData is modified.
  void imageDataChanged(vtkImageData * imageData);
  void renderRequested();

protected:
  QScopedPointer<qMRMLSliceControllerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliceControllerWidget);
  Q_DISABLE_COPY(qMRMLSliceControllerWidget);
};

#endif
