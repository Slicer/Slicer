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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSliceControllerWidget_h
#define __qMRMLSliceControllerWidget_h

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"

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
class QMRML_WIDGETS_EXPORT qMRMLSliceControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  Q_PROPERTY(QString sliceViewName READ sliceViewName WRITE setSliceViewName)
public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;
  
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

  /// Return the color associated to the slice view
  Q_INVOKABLE static QColor sliceViewColor(const QString& sliceViewName);

  /// Set the label for the slice view (abbreviation for the view
  /// name)
  void setSliceViewLabel(const QString& newSliceViewLabel);

  /// Get the label for the slice view (abbreviation for the view name)
  QString sliceViewLabel()const;

  /// Set the color for the slice view (hex string #RRGGBB or SVG
  /// color name)
  void setSliceViewColor(const QString& newSliceViewColor);

  /// Get the color for the slice view (as a string)
  QString sliceViewColor()const;

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

  /// Set slice \a offset. Used to set a single value.
  void setSliceOffsetValue(double offset);

  /// Set slice offset. Used when events will come is rapid succession.
  void trackSliceOffsetValue(double offset);

  /// Set slice visible.
  void setSliceVisible(bool visible);

  /// Link/Unlink the slice controls across all slice viewer
  void setSliceLink(bool linked);

  /// Set the link mode to hot linked. When on, slice interactions affect other
  /// slices immediately. When off, slice interactions affect other
  /// slices after the interaction completes.
  void setHotLinked(bool hot);


  // Advanced options
  /// Rotate to volume plane
  void rotateSliceToBackground();
  
  void setLabelMapHidden(bool hide);
  void setForegroundHidden(bool hide);
  void setBackgroundHidden(bool hide);

  /// Label opacity
  void setLabelMapOpacity(double opacity);
  void setForegroundOpacity(double opacity);
  void setBackgroundOpacity(double opacity);
  
  /// Label outline
  void showLabelOutline(bool show);
  /// Reformat widget
  void showReformatWidget(bool show);
  void lockReformatWidgetToCamera(bool lock);
  /// Compositing
  void setCompositing(int mode);
  void setCompositingToAlphaBlend();
  void setCompositingToReverseAlphaBlend();
  void setCompositingToAdd();
  void setCompositingToSubtract();
  /// Slice spacing
  void setSliceSpacingMode(bool automatic);
  void setSliceSpacing(double spacing);
  void setSliceFOV(double fov);
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
  void setForegroundInterpolation(bool nearestNeighbor);
  void setBackgroundInterpolation(bool nearestNeighbor);
  
  void setNavigatorEnabled(bool enable);
  void setCrosshairMode(int mode);
  void setCrosshairThickness(int thickness);
  void setSliceIntersectionVisible(bool show);
signals:

  /// This signal is emitted when the given \a imageData is modified.
  void imageDataChanged(vtkImageData * imageData);
  void renderRequested();

private:
  Q_DECLARE_PRIVATE(qMRMLSliceControllerWidget);
  Q_DISABLE_COPY(qMRMLSliceControllerWidget);
};

#endif
