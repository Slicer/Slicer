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

#ifndef __qMRMLLayoutManager_p_h
#define __qMRMLLayoutManager_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/// Qt includes
#include <QHash>
#include <QObject>

/// CTK includes
#include <ctkVTKObject.h>
#include <ctkLayoutManager_p.h>

// qMRML includes
#include "qMRMLWidgetsConfigure.h" // For MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutViewFactory.h"

// MRMLLogic includes
#include <vtkMRMLLayoutLogic.h>

// VTK includes
#include <vtkSmartPointer.h>

class QLayout;
class QGridLayout;
class QButtonGroup;
class qMRMLSliceWidget;
class qMRMLChartView;
class qMRMLChartWidget;
class qMRMLTableView;
class qMRMLTableWidget;
class qMRMLPlotView;
class qMRMLPlotWidget;
class qMRMLThreeDView;
class qMRMLThreeDWidget;
class vtkCollection;
class vtkObject;
class vtkMRMLLayoutLogic;
class vtkMRMLLayoutNode;
class vtkMRMLChartViewNode;
class vtkMRMLTableViewNode;
class vtkMRMLPlotViewNode;
class vtkMRMLViewNode;
class vtkMRMLSliceNode;
class vtkXMLDataElement;

//-----------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutManagerPrivate
  : public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLLayoutManager);
protected:
  qMRMLLayoutManager* const q_ptr;

public:
  qMRMLLayoutManagerPrivate(qMRMLLayoutManager& object);
  ~qMRMLLayoutManagerPrivate() override;

  virtual void init();

  void setMRMLLayoutNode(vtkMRMLLayoutNode* node);
  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * node);
  void setActiveMRMLChartViewNode(vtkMRMLChartViewNode * node);
  void setActiveMRMLTableViewNode(vtkMRMLTableViewNode * node);
  void setActiveMRMLPlotViewNode(vtkMRMLPlotViewNode * node);

  /// Enable/disable paint event associated with the TargetWidget
  //bool startUpdateLayout();
  //void endUpdateLayout(bool updateEnabled);

  /// Refresh the viewport with the current layout from the layout
  /// layout node. Empty the view if there is no layout node.
  void updateLayoutInternal();

  void setLayoutNumberOfCompareViewRowsInternal(int num);
  void setLayoutNumberOfCompareViewColumnsInternal(int num);

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName.
  qMRMLThreeDWidget* threeDWidget(vtkMRMLViewNode* node)const;
  qMRMLChartWidget* chartWidget(vtkMRMLChartViewNode* node)const;
  qMRMLTableWidget* tableWidget(vtkMRMLTableViewNode* node)const;
  qMRMLPlotWidget* plotWidget(vtkMRMLPlotViewNode* node)const;

  /// Convenient function allowing to get a reference to the sliceView widget
  /// identified by \a sliceViewName
  qMRMLSliceWidget* sliceWidget(vtkMRMLSliceNode* node)const;

  vtkMRMLNode* viewNode(QWidget* )const;
  QWidget* viewWidget(vtkMRMLNode* )const;

public slots:
  /// Handle MRML scene event
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();
  void onSceneRestoredEvent();

  /// Handle Layout node event
  void onLayoutNodeModifiedEvent(vtkObject* layoutNode);
  void updateLayoutFromMRMLScene();

  void onActiveThreeDViewNodeChanged(vtkMRMLAbstractViewNode*);
  void onActiveChartViewNodeChanged(vtkMRMLAbstractViewNode*);
  void onActiveTableViewNodeChanged(vtkMRMLAbstractViewNode*);
  void onActivePlotViewNodeChanged(vtkMRMLAbstractViewNode*);

  /// Show segmentation controls in slice widgets only if there is at
  /// least one segmentation node in the scene
  void updateSegmentationControls();

public:
  bool                    Enabled;
  vtkMRMLScene*           MRMLScene;
  vtkMRMLLayoutNode*      MRMLLayoutNode;
  vtkMRMLLayoutLogic*     MRMLLayoutLogic;
  vtkMRMLViewNode*        ActiveMRMLThreeDViewNode;
  vtkMRMLChartViewNode*   ActiveMRMLChartViewNode;
  vtkMRMLTableViewNode*   ActiveMRMLTableViewNode;
  vtkMRMLPlotViewNode*    ActiveMRMLPlotViewNode;
protected:
  void showWidget(QWidget* widget);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutThreeDViewFactory
  : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutThreeDViewFactory(QObject* parent = nullptr);
  ~qMRMLLayoutThreeDViewFactory() override;

  QString viewClassName()const override;

  vtkCollection* viewLogics()const;
  void setViewLogics(vtkCollection* viewLogics);

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override;
  void deleteView(vtkMRMLAbstractViewNode* viewNode) override;

  vtkCollection* ViewLogics;
};

//------------------------------------------------------------------------------
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
class QMRML_WIDGETS_EXPORT qMRMLLayoutChartViewFactory
  : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutChartViewFactory(QObject* parent = nullptr);

  QString viewClassName()const override;

  vtkMRMLColorLogic* colorLogic()const;
  void setColorLogic(vtkMRMLColorLogic* colorLogic);

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override;
  vtkMRMLColorLogic* ColorLogic{nullptr};
};
#endif

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutTableViewFactory
  : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutTableViewFactory(QObject* parent = nullptr);

  QString viewClassName()const override;

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutPlotViewFactory
  : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutPlotViewFactory(QObject* parent = nullptr);

  QString viewClassName()const override;

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override;
  vtkMRMLColorLogic* ColorLogic{nullptr};
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutSliceViewFactory
  : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutSliceViewFactory(QObject* parent = nullptr);
  ~qMRMLLayoutSliceViewFactory() override;

  QString viewClassName()const override;

  vtkCollection* sliceLogics()const;
  void setSliceLogics(vtkCollection* sliceLogics);

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override;
  void deleteView(vtkMRMLAbstractViewNode* viewNode) override;

  QButtonGroup* SliceControllerButtonGroup;
  vtkCollection* SliceLogics;
};

#endif
