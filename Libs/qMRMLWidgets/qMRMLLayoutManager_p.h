/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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

// qMRML includes
#include "qMRMLLayoutManager.h"

// VTK includes
#include <vtkSmartPointer.h>

class QLayout;
class QGridLayout;
class QButtonGroup;
class qMRMLSliceWidget;
class qMRMLThreeDView;
class vtkCollection;
class vtkObject;
class vtkMRMLLayoutLogic;
class vtkMRMLLayoutNode;
class vtkMRMLViewNode;
class vtkMRMLSliceNode;
class vtkXMLDataElement;

//-----------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLLayoutManagerPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLLayoutManager);
protected:
  qMRMLLayoutManager* const q_ptr;
public:
  qMRMLLayoutManagerPrivate(qMRMLLayoutManager& object);
  ~qMRMLLayoutManagerPrivate();

  void setMRMLScene(vtkMRMLScene* scene);
  void setMRMLLayoutNode(vtkMRMLLayoutNode* node);
  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * node);

  /// Instantiate a slice viewer corresponding to \a sliceViewName
  virtual QWidget* createSliceWidget(vtkMRMLSliceNode* sliceNode);

  /// Delete slice viewer associated with \a sliceNode
  void removeSliceView(vtkMRMLSliceNode* sliceNode);

  /// Instantiate a 3D Viewer corresponding to \a viewNode
  virtual qMRMLThreeDView* createThreeDView(vtkMRMLViewNode* viewNode);
  /// Convenient function that creates a vtkMRMLViewNode and a qMRMLThreeDView
  qMRMLThreeDView* createThreeDView();

  /// Delete 3D Viewer associated with \a viewNode
  void removeThreeDView(vtkMRMLViewNode* viewNode);

  ///
  void initialize();

  /// Enable/disable paint event associated with the TargetWidget
  bool startUpdateLayout();
  void endUpdateLayout(bool updateEnabled);

  /// Actually set the layout type to the node
  void updateLayoutNode(int layout);

  /// Hide and remove all widgets from the current layout
  void clearLayout(QLayout* layout);
  void setupLayout();

  QLayout* addLayoutElement(vtkXMLDataElement* );
  void     addItemElement(vtkXMLDataElement*, QObject*);
  QWidget* addViewElement(vtkXMLDataElement*);

  void setLayoutInternal(int layout);
  void setConventionalView();
  void setOneUp3DView();
  void setOneUpSliceView(const QString& sliceViewName);
  void setFourUpView();
  void setTabbed3DView();
  void setTabbedSliceView();
  void setLightboxView();
  void setCompareView();
  void setSideBySideLightboxView();
  void setDual3DView();
  void setNone();

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName.
  qMRMLThreeDView* threeDView(vtkMRMLViewNode* node)const;
  qMRMLThreeDView* threeDView(int id)const;
  qMRMLThreeDView* threeDViewCreateIfNeeded(int id);

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
  void onSceneImportedEvent();
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();

  /// Handle Layout node event
  void onLayoutNodeModifiedEvent(vtkObject* layoutNode);
protected slots:
  void initializeNode(vtkMRMLNode*);
public:
  QString            ScriptedDisplayableManagerDirectory;
  vtkMRMLScene*      MRMLScene;
  vtkMRMLLayoutNode* MRMLLayoutNode;
  vtkMRMLLayoutLogic*MRMLLayoutLogic;
  vtkMRMLViewNode*   ActiveMRMLThreeDViewNode;
  int                SavedCurrentViewArrangement;
  QGridLayout*       GridLayout;
  QWidget*           TargetWidget;
  QButtonGroup*      SliceControllerButtonGroup;
  vtkCollection*     MRMLSliceLogics;

  QList<qMRMLThreeDView*>           ThreeDViewList;
  QList<qMRMLSliceWidget*>          SliceWidgetList;
protected:
  void showWidget(QWidget* widget);
};

#endif
