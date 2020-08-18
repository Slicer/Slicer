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

class qMRMLThreeDViewPrivate;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLCameraNode;
class vtkMRMLScene;
class vtkMRMLViewNode;
class vtkCollection;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshs when the scene is in
/// batch process state.
/// \sa qMRMLThreeDWidget, qMRMLThreeDViewControllerWidget, qMRMLSliceView
class QMRML_WIDGETS_EXPORT qMRMLThreeDView : public ctkVTKRenderView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;

  /// Constructors
  explicit qMRMLThreeDView(QWidget* parent = nullptr);
  ~qMRMLThreeDView() override;

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
  Q_INVOKABLE  vtkMRMLAbstractDisplayableManager* displayableManagerByClassName(const char* className);

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLViewNode* mrmlViewNode()const;

  /// Returns the interactor style of the view
  //vtkInteractorObserver* interactorStyle()const;

  /// Methods to rotate/reset the camera,
  /// Can defined a view axis by its index (from 0 to 5)
  /// or its label (defined in vtkMRMLViewNode::AxisLabels)
  /// to rotate to the axis ranged in that order:
  /// -X, +X, -Y, +Y, -Z, +Z
  Q_INVOKABLE void rotateToViewAxis(unsigned int axisId);
  Q_INVOKABLE void rotateToViewAxis(const std::string& axisLabel);
  Q_INVOKABLE void resetCamera(bool resetRotation = true,
                               bool resetTranslation = true,
                               bool resetDistance = true);

  /// Returns camera node of the 3D view
  Q_INVOKABLE vtkMRMLCameraNode* cameraNode();

  /// Set cursor in the view area
  Q_INVOKABLE void setViewCursor(const QCursor &);

  /// Restore default cursor in the view area
  Q_INVOKABLE void unsetViewCursor();

  /// Set default cursor in the view area
  Q_INVOKABLE void setDefaultViewCursor(const QCursor &cursor);

public slots:

  /// Set the MRML \a scene that should be listened for events
  /// When the scene is in batch process state, the view blocks all refresh.
  /// \sa renderEnabled
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);

  /// Look from a given axis, need a mrml view node to be set
  void lookFromViewAxis(const ctkAxesWidget::Axis& axis);

  /// Reimplemented to hide items to not take into
  /// account when computing the boundaries
  virtual void resetFocalPoint();

protected:
  QScopedPointer<qMRMLThreeDViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDView);
  Q_DISABLE_COPY(qMRMLThreeDView);
};

#endif
