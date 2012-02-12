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

#ifndef __qMRMLLayoutManager_p_h
#define __qMRMLLayoutManager_p_h

/// Qt includes
#include <QHash>
#include <QObject>

/// CTK includes
#include <ctkVTKObject.h>
#include <ctkLayoutManager_p.h>

// qMRML includes
#include "qMRMLLayoutManager.h"

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
class qMRMLThreeDView;
class qMRMLThreeDWidget;
class vtkCollection;
class vtkObject;
class vtkMRMLLayoutLogic;
class vtkMRMLLayoutNode;
class vtkMRMLChartViewNode;
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
  virtual ~qMRMLLayoutManagerPrivate();

  virtual void init();

  void setMRMLLayoutNode(vtkMRMLLayoutNode* node);
  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * node);
  void setActiveMRMLChartViewNode(vtkMRMLChartViewNode * node);

  /// Instantiate a slice viewer corresponding to \a sliceViewName
  virtual QWidget* createSliceWidget(vtkMRMLSliceNode* sliceNode);

  /// Delete slice viewer associated with \a sliceNode
  void removeSliceView(vtkMRMLSliceNode* sliceNode);

  /// Instantiate a 3D Viewer corresponding to \a viewNode
  virtual qMRMLThreeDWidget* createThreeDWidget(vtkMRMLViewNode* viewNode);
  virtual qMRMLChartWidget* createChartWidget(vtkMRMLChartViewNode* viewNode);

  /// Delete 3D Viewer associated with \a viewNode
  void removeThreeDWidget(vtkMRMLViewNode* viewNode);
  void removeChartWidget(vtkMRMLChartViewNode* viewNode);

  /// Enable/disable paint event associated with the TargetWidget
  //bool startUpdateLayout();
  //void endUpdateLayout(bool updateEnabled);

  /// Actually set the layout type to the node
  void updateLayoutNode(int layout);

  void setLayoutInternal(int layout);
  void setLayoutNumberOfCompareViewRowsInternal(int num);
  void setLayoutNumberOfCompareViewColumnsInternal(int num);

  vtkMRMLLayoutLogic::ViewAttributes attributesFromXML(QDomElement viewElement)const;
  vtkMRMLLayoutLogic::ViewProperties propertiesFromXML(QDomElement viewElement)const;
  vtkMRMLLayoutLogic::ViewProperty propertyFromXML(QDomElement propertyElement)const;

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName.
  qMRMLThreeDWidget* threeDWidget(vtkMRMLViewNode* node)const;
  qMRMLThreeDWidget* threeDWidget(int id)const;
  qMRMLChartWidget* chartWidget(vtkMRMLChartViewNode* node)const;
  qMRMLChartWidget* chartWidget(int id)const;

  /// Convenient function allowing to get a reference to the sliceView widget
  /// identified by \a sliceViewName
  qMRMLSliceWidget* sliceWidget(const QString& sliceLayoutName)const;
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
  void updateWidgetsFromViewNodes();
  void updateLayoutFromMRMLScene();

public:
  vtkMRMLScene*           MRMLScene;
  vtkMRMLLayoutNode*      MRMLLayoutNode;
  vtkMRMLLayoutLogic*     MRMLLayoutLogic;
  vtkMRMLViewNode*        ActiveMRMLThreeDViewNode;
  vtkMRMLChartViewNode*   ActiveMRMLChartViewNode;
  int                     SavedCurrentViewArrangement;
  QGridLayout*            GridLayout;
  QWidget*                TargetWidget;
  QButtonGroup*           SliceControllerButtonGroup;
  vtkCollection*          MRMLSliceLogics;
  vtkMRMLColorLogic*      MRMLColorLogic;

  QList<qMRMLThreeDWidget*>         ThreeDWidgetList;
  QList<qMRMLChartWidget*>          ChartWidgetList;
  QList<qMRMLSliceWidget*>          SliceWidgetList;
protected:
  void showWidget(QWidget* widget);
};

#endif
