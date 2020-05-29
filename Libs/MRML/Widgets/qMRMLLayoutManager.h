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

#ifndef __qMRMLLayoutManager_h
#define __qMRMLLayoutManager_h

// Qt includes
#include <QStringList>
class QWidget;

// CTK includes
#include <ctkLayoutFactory.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLChartWidget;
class qMRMLPlotWidget;
class qMRMLTableWidget;
class qMRMLThreeDWidget;
class qMRMLSliceWidget;
class qMRMLLayoutManagerPrivate;
class qMRMLLayoutViewFactory;

class vtkMRMLColorLogic;
class vtkMRMLLayoutLogic;
class vtkMRMLScene;
class vtkMRMLChartNode;
class vtkMRMLNode;
class vtkMRMLChartViewNode;
class vtkMRMLTableNode;
class vtkMRMLTableViewNode;
class vtkMRMLViewNode;
class vtkMRMLPlotViewNode;

class vtkCollection;
class vtkRenderer;

/// MRML layout manager that instantiates the Qt widgets and layouts from the
/// layout node and the view nodes in the scene.
/// The management (creation, configuration and deletion) of the view widgets
/// are delegated to qMRMLLayoutViewFactory.
/// Example to change a default factory:
/// \code
/// qMRMLLayoutSliceViewFactory* mrmlSliceViewFactory =
///   qobject_cast<qMRMLLayoutSliceViewFactory*>(
///     app->layoutManager()->mrmlViewFactory("vtkMRMLSliceNode"));
/// MySliceViewFactory* mySliceViewFactory =
///   new MySliceViewFactory(app->layoutManager());
/// mySliceViewFactory->setSliceLogics(mrmlSliceViewFactory->sliceLogics());
///
/// app->layoutManager()->unregisterViewFactory(mrmlSliceViewFactory);
/// app->layoutManager()->registerViewFactory(mySliceViewFactory);
/// \endcode
/// You can also register ctkLayoutViewFactories that are not related to MRML
/// view nodes.
/// \sa ctkLayoutFactory, ctkLayoutManager, qMRMLLayoutViewFactory
class QMRML_WIDGETS_EXPORT qMRMLLayoutManager : public ctkLayoutFactory
{
  Q_OBJECT

  /// This property controls whether the layout manager reacts to layout node
  /// changes or note. When enabled (default), the layout is updated each time
  /// the layout node is modified and when the scene leaves batch-process state.
  /// It can be useful to temporarilly disable the manager when loading a scene,
  /// it could otherwise change the layout.
  /// \sa isEnabled(), setEnabled(), setMRMLScene()
  Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
  // The following properties are exposed so that they are available within python
  Q_PROPERTY(int layout READ layout WRITE setLayout NOTIFY layoutChanged DESIGNABLE false)
  Q_PROPERTY(int threeDViewCount READ threeDViewCount DESIGNABLE false)
  Q_PROPERTY(int chartViewCount READ chartViewCount DESIGNABLE false)
  Q_PROPERTY(int tableViewCount READ tableViewCount DESIGNABLE false)
  Q_PROPERTY(int plotViewCount READ plotViewCount DESIGNABLE false)

public:
  /// Superclass typedef
  typedef ctkLayoutFactory Superclass;

  /// Constructors
  explicit qMRMLLayoutManager(QObject* parent=nullptr);
  explicit qMRMLLayoutManager(QWidget* viewport, QObject* parent);
  ~qMRMLLayoutManager() override;

  /// Return the enabled property value.
  /// \sa enabled
  bool isEnabled()const;

  /// Reimplemented for internal reasons.
  /// If the registered view factory is a qMRMLLayoutViewFactory, then set
  /// its layoutManager and its mrmlScene.
  /// \sa ctkLayoutFactory::registerViewFactory(), unregisterViewFactory()
  Q_INVOKABLE virtual void registerViewFactory(ctkLayoutViewFactory* viewFactory);

  /// Return the list of registered MRML view factories.
  /// \sa registeredViewFactories(), registerViewFactory(),
  /// unregisterViewFactory()
  Q_INVOKABLE QList<qMRMLLayoutViewFactory*> mrmlViewFactories()const;

  /// Return the view factory that handles the viewClassName view nodes.
  /// This can be used to replace a view factory with another one.
  /// \sa mrmlViewFactories(), registerViewFactory(), unregisterViewFactory()
  Q_INVOKABLE qMRMLLayoutViewFactory* mrmlViewFactory(const QString& viewClassName)const;

  /// Return the mrml scene of the layout manager. It is the scene that is set
  /// by setMRMLScene().
  /// \sa setMRMLScene(), enabled
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  /// Get the view widget representing a particular node (can be used
  /// for SliceNodes or ViewNodes, returning qMRMLSliceWidget or
  /// qMRMLThreeDWidget respectively).
  Q_INVOKABLE QWidget* viewWidget(vtkMRMLNode* n) const;

  /// Get slice view widget identified by \a name
  Q_INVOKABLE qMRMLSliceWidget* sliceWidget(const QString& name)const;

  /// Get 3D widget identified by \a name
  Q_INVOKABLE qMRMLThreeDWidget* threeDWidget(const QString& name)const;

  /// Get the list of SliceWidgetNames
  /// All slice widget names are returned,
  /// even those from hidden and not currently used widgets.
  Q_INVOKABLE QStringList sliceViewNames() const;

  /// Return the number of instantiated ThreeDRenderView
  int threeDViewCount()const;
  int chartViewCount()const;
  int tableViewCount()const;
  int plotViewCount()const;

  /// Get ThreeDWidget identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qMRMLThreeDView (that should also be equal to the number
  /// of vtkMRMLViewNode)
  Q_INVOKABLE qMRMLThreeDWidget* threeDWidget(int id)const;
  Q_INVOKABLE qMRMLChartWidget* chartWidget(int id)const;
  Q_INVOKABLE qMRMLTableWidget* tableWidget(int id)const;
  Q_INVOKABLE qMRMLPlotWidget* plotWidget(int id)const;

  /// Return the up-to-date list of vtkMRMLSliceLogics associated to the slice views.
  /// The returned collection object is owned by the layout manager.
  Q_INVOKABLE vtkCollection* mrmlSliceLogics()const;

  /// Return the up-to-date list of vtkMRMLViewLogics associated to the threeD views.
  /// The returned collection object is owned by the layout manager.
  Q_INVOKABLE vtkCollection* mrmlViewLogics()const;

  Q_INVOKABLE void setMRMLColorLogic(vtkMRMLColorLogic* colorLogic);
  Q_INVOKABLE vtkMRMLColorLogic* mrmlColorLogic()const;

  /// Returns the current layout. it's the same value than
  /// vtkMRMLLayoutNode::ViewArrangement
  /// \sa vtkMRMLLayoutNode::SlicerLayout, layoutLogic()
  int layout()const;

  /// Return the layout logic instantiated and used by the manager.
  /// \sa setLayout(), layout()
  Q_INVOKABLE vtkMRMLLayoutLogic* layoutLogic()const;

  /// Return the view node of the active 3D view.
  /// \todo For now the active view is the first 3D view.
  /// \sa activeThreeDRenderer(), activeMRMLChartViewNode(),
  /// activeChartRenderer()
  Q_INVOKABLE vtkMRMLViewNode* activeMRMLThreeDViewNode()const;
  /// Return the renderer of the active 3D view.
  /// \todo For now the active view is the first 3D view.
  /// \sa  activeThreeDRenderer(), activeMRMLChartViewNode(),
  /// activeChartRenderer()
  Q_INVOKABLE vtkRenderer* activeThreeDRenderer()const;
  /// Return the view node of the active chart view.
  /// \todo For now the active view is the first chart view.
  /// \sa  activeChartRenderer(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkMRMLChartViewNode* activeMRMLChartViewNode()const;
  /// Return the renderer of the active chart view.
  /// \todo For now the active view is the first chart view.
  /// \sa  activeMRMLChartViewNode(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkRenderer* activeChartRenderer()const;
  /// Return the view node of the active table view.
  /// \todo For now the active view is the first table view.
  /// \sa  activeTableRenderer(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkMRMLTableViewNode* activeMRMLTableViewNode()const;
  /// Return the renderer of the active table view.
  /// \todo For now the active view is the first table view.
  /// \sa  activeMRMLTableViewNode(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkRenderer* activeTableRenderer()const;
  /// Return the view node of the active plot view.
  /// \todo For now the active view is the first plot view.
  /// \sa  activePlotRenderer(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkMRMLPlotViewNode* activeMRMLPlotViewNode()const;
  /// Return the renderer of the active plot view.
  /// \todo For now the active view is the first plot view.
  /// \sa  activeMRMLPlotViewNode(), activeMRMLThreeDViewNode(),
  /// activeThreeDRenderer()
  Q_INVOKABLE vtkRenderer* activePlotRenderer()const;


public slots:
  /// Set the enabled property value
  /// \sa enabled
  void setEnabled(bool enable);

  /// Set the MRML \a scene that should be listened for events
  /// \sa mrmlScene(), enabled
  void setMRMLScene(vtkMRMLScene* scene);

  /// Change the current layout (see vtkMRMLLayoutNode::SlicerLayout)
  /// It creates views if needed.
  void setLayout(int newLayout);

  /// Change the number of viewers in comparison modes
  /// It creates views if needed.
  void setLayoutNumberOfCompareViewRows(int num);
  void setLayoutNumberOfCompareViewColumns(int num);

  /// Reset focal view around volumes
  /// \sa ctkVTKRenderView::resetFocalPoint(), ctkVTKRenderView::resetCamera()
  void resetThreeDViews();

  /// Reset focal view around volumes
  /// \sa qMRMLSliceControllerWidget::fitSliceToBackground(), vtkMRMLSliceLogic::FitSliceToAll()
  void resetSliceViews();

  /// Calls setPauseRender(pause) on all slice and 3D views
  /// Tracks the previous pause state which is restored using resumeRender()
  /// Each pauseRender(true) should always be accompanied by a corresponding pauseRender(false) call
  /// \sa pauseRender
  void setRenderPaused(bool pause);

  /// Equivalent to setRenderPaused(true)
  /// \sa setRenderPaused
  void pauseRender();

  /// Equivalent to setRenderPaused(false)
  /// \sa setRenderPaused
  void resumeRender();

signals:
  void activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode* newActiveMRMLThreeDViewNode);
  void activeMRMLChartViewNodeChanged(vtkMRMLChartViewNode* newActiveMRMLChartViewNode);
  void activeMRMLTableViewNodeChanged(vtkMRMLTableViewNode* newActiveMRMLChartViewNode);
  void activeMRMLPlotViewNodeChanged(vtkMRMLPlotViewNode* newActiveMRMLPlotViewNode);
  void activeThreeDRendererChanged(vtkRenderer* newRenderer);
  void activeChartRendererChanged(vtkRenderer* newRenderer);
  void activeTableRendererChanged(vtkRenderer* newRenderer);
  void activePlotRendererChanged(vtkRenderer* newRenderer);
  void layoutChanged(int);

  /// Signal emitted when editing of a node is requested from within the layout
  void nodeAboutToBeEdited(vtkMRMLNode* node);

protected:
  QScopedPointer<qMRMLLayoutManagerPrivate> d_ptr;
  qMRMLLayoutManager(qMRMLLayoutManagerPrivate* obj, QWidget* viewport, QObject* parent);

  void onViewportChanged() override;

  using ctkLayoutManager::setLayout;
private:
  Q_DECLARE_PRIVATE(qMRMLLayoutManager);
  Q_DISABLE_COPY(qMRMLLayoutManager);
};

#endif
