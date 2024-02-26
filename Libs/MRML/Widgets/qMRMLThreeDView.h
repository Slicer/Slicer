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

#ifndef __qMRMLThreeDView_h
#define __qMRMLThreeDView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

#include "qMRMLWidgetsExport.h"

class QDropEvent;
class qMRMLThreeDViewPrivate;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLCameraNode;
class vtkMRMLScene;
class vtkMRMLThreeDViewInteractorStyle;
class vtkMRMLViewNode;
class vtkCollection;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshes when the scene is in
/// batch process state.
/// \sa qMRMLThreeDWidget, qMRMLThreeDViewControllerWidget, qMRMLSliceView
class QMRML_WIDGETS_EXPORT qMRMLThreeDView : public ctkVTKRenderView
{
  Q_OBJECT
  /// Show shadows to improve depth perception.
  /// Currently, only ambient shadows (screen-space ambient occlusion) method is supported and AmbientShadowsSizeScale
  /// and AmbientShadowsVolumeOpacityThreshold parameters control its appearance.
  Q_PROPERTY(bool shadowsVisibility READ shadowsVisibility WRITE setShadowsVisibility)
  /// Ambient shadows size scale.
  /// Specifies size of features to be emphasized by shadows.The scale is logarithmic, default (0.0) corresponds to
  /// object size of about 100 (in scene physical units).
  Q_PROPERTY(double ambientShadowsSizeScale READ ambientShadowsSizeScale WRITE setAmbientShadowsSizeScale)
  /// Volume rendering opacity above this value will cast shadows.
  Q_PROPERTY(double ambientShadowsVolumeOpacityThreshold READ ambientShadowsVolumeOpacityThreshold WRITE
               setAmbientShadowsVolumeOpacityThreshold)

public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;

  /// Constructors
  explicit qMRMLThreeDView(QWidget* parent = nullptr);
  ~qMRMLThreeDView() override;

  /// Sets the interactor of the view
  void setInteractor(vtkRenderWindowInteractor* interactor) override;

  /// Returns the interactor observer of the view
  Q_INVOKABLE vtkMRMLThreeDViewInteractorStyle* interactorObserver() const;

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkMRMLThreeDViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLCameraDisplayableManager,
  /// vtkMRMLViewDisplayableManager and vtkMRMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection* displayableManagers);

  /// Return a DisplayableManager given its class name
  Q_INVOKABLE vtkMRMLAbstractDisplayableManager* displayableManagerByClassName(const char* className);

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLViewNode* mrmlViewNode() const;

  /// Methods to rotate/reset the camera,
  /// Can defined a view axis by its index (from 0 to 5)
  /// or its label (defined in vtkMRMLViewNode::AxisLabels)
  /// to rotate to the axis ranged in that order:
  /// -X, +X, -Y, +Y, -Z, +Z
  Q_INVOKABLE void rotateToViewAxis(unsigned int axisId);
  Q_INVOKABLE void rotateToViewAxis(const std::string& axisLabel);
  Q_INVOKABLE void resetCamera(bool resetRotation = true, bool resetTranslation = true, bool resetDistance = true);

  /// Returns camera node of the 3D view
  Q_INVOKABLE vtkMRMLCameraNode* cameraNode();

  /// Set cursor in the view area
  Q_INVOKABLE void setViewCursor(const QCursor&);

  /// Restore default cursor in the view area
  Q_INVOKABLE void unsetViewCursor();

  /// Set default cursor in the view area
  Q_INVOKABLE void setDefaultViewCursor(const QCursor& cursor);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

  bool shadowsVisibility() const;
  double ambientShadowsSizeScale() const;
  double ambientShadowsVolumeOpacityThreshold() const;

public slots:

  /// Set the MRML \a scene that should be listened for events
  /// When the scene is in batch process state, the view blocks all refresh.
  /// \sa renderEnabled
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);

  /// \deprecated Use lookFromAxis instead.
  void lookFromViewAxis(const ctkAxesWidget::Axis& axis)
  {
    qWarning("This function is deprecated. Use lookFromAxis(const ctkAxesWidget::Axis& axis) instead.");
    this->lookFromAxis(axis);
  };

  /// Reimplemented to hide items to not take into
  /// account when computing the boundaries
  virtual void resetFocalPoint();

  void setShadowsVisibility(bool);
  void setAmbientShadowsSizeScale(double);
  void setAmbientShadowsVolumeOpacityThreshold(double);

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDView);
  Q_DISABLE_COPY(qMRMLThreeDView);
};

#endif
