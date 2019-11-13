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

#ifndef __qMRMLSliceView_h
#define __qMRMLSliceView_h

// CTK includes
#include <ctkVTKSliceView.h>

// MRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLSliceViewPrivate;
class vtkCollection;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLScene;
class vtkMRMLSliceNode;
class vtkMRMLSliceViewInteractorStyle;

/// \brief 2D view for slice nodes.
/// For performance reasons, the view block refreshs when the scene is in
/// batch process state.
/// \sa qMRMLSliceWidget, qMRMLSliceControllerWidget, qMRMLThreeDView
class QMRML_WIDGETS_EXPORT qMRMLSliceView
  : public ctkVTKSliceView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKSliceView Superclass;

  /// Constructors
  explicit qMRMLSliceView(QWidget* parent = nullptr);
  ~qMRMLSliceView() override;

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 2D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 2D
  /// views (existing or future), you need to do it via
  /// vtkMRMLSliceViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLModelSliceDisplayableManager,
  /// vtkMRMLVolumeGlyphSliceDisplayableManager and
  /// vtkMRMLCrosshairDisplayableManager are registered.
  /// \sa getDisplayableManagers
  Q_INVOKABLE void addDisplayableManager(const QString& displayableManager);
  /// Get the displayable managers registered in this view
  /// \sa addDisplayableManager
  Q_INVOKABLE void getDisplayableManagers(vtkCollection *displayableManagers);

  /// Return a DisplayableManager given its class name
  Q_INVOKABLE  vtkMRMLAbstractDisplayableManager* displayableManagerByClassName(const char* className);

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLSliceNode* mrmlSliceNode()const;

  /// Returns the interactor style of the view
  Q_INVOKABLE vtkMRMLSliceViewInteractorStyle* sliceViewInteractorStyle()const;

  /// Convert device coordinates to XYZ coordinates. The x and y
  /// components of the return value are the positions within a
  /// LightBox pane and the z component of the return value (rounded
  /// to the nearest integer) is the pane in the LightBox
  Q_INVOKABLE QList<double> convertDeviceToXYZ(const QList<int>&xy)const;

  /// Convert RAS to XYZ coordinates. parameters ras and return value
  /// are of length 3. Z component of the return value is the pane in
  /// the LightBox.
  Q_INVOKABLE QList<double> convertRASToXYZ(const QList<double>& ras)const;

  /// Convert XYZ to RAS coordinates. parameters xyz and return value
  /// are of length 3. Z component of parameter xyz is the LightBox
  /// pane and the X and Y components of parameter xyz is the position
  /// in the LightBox pane.
  Q_INVOKABLE QList<double> convertXYZToRAS(const QList<double> &xyz)const;

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
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

protected:
  QScopedPointer<qMRMLSliceViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliceView);
  Q_DISABLE_COPY(qMRMLSliceView);
};

#endif
