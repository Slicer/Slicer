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

#ifndef __qSlicerLayoutManager_h
#define __qSlicerLayoutManager_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>
#include "qSlicerBaseQTGUIExport.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>

class qMRMLThreeDView;
class qMRMLSliceWidget;
class qSlicerLayoutManagerPrivate;
class vtkCollection;
class vtkMRMLScene;
class vtkMRMLViewNode;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLayoutManager : public QObject
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QObject Superclass;
  
  /// Constructors
  explicit qSlicerLayoutManager(QWidget* widget);
  virtual ~qSlicerLayoutManager(){}

  void setViewport(QWidget* widget);
  QWidget* viewport()const;

  vtkMRMLScene* mrmlScene()const;

  /// Set the directory from which build-in scripted
  /// displayableManagers should be sourced from.
  void setScriptedDisplayableManagerDirectory(const QString& scriptedDisplayableManagerDirectory);

  /// Get SliceViewWidget identified by \a name
  qMRMLSliceWidget* sliceWidget(const QString& name)const;

  /// Return the number of instantiated ThreeDRenderView
  int threeDViewCount()const;
  
  /// Get ThreeDRenderView identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qMRMLThreeDView (that should also be equal to the number
  /// of vtkMRMLViewNode)
  qMRMLThreeDView* threeDView(int id)const;

  /// Return the up-to-date list of vtkMRMLSliceLogics associated to the slice views.
  vtkCollection* mrmlSliceLogics()const;

  int layout()const;

  vtkMRMLViewNode* activeMRMLThreeDViewNode()const;

public slots:

  /// 
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  /// Switch to the different layout
  /// TODO A better way would be to register layout classes with the manager
  inline void switchToConventionalView();
  inline void switchToOneUp3DView();
  inline void switchToOneUpRedSliceView();
  inline void switchToOneUpYellowSliceView();
  inline void switchToOneUpGreenSliceView();
  void switchToOneUpSliceView(const QString& sliceViewName);
  inline void switchToFourUpView();
  inline void switchToTabbed3DView();
  inline void switchToTabbedSliceView();
  inline void switchToLightboxView();
  inline void switchToCompareView();
  inline void switchToSideBySideCompareView();
  inline void switchToDual3DView();
  inline void switchToNone();

  /// Generic function
  void setLayout(int);

signals:
  void activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);
  
private:
  CTK_DECLARE_PRIVATE(qSlicerLayoutManager);
};

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToConventionalView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUp3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpRedSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpYellowSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpGreenSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToFourUpView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToTabbed3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToTabbedSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToLightboxView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutLightboxView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToCompareView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToSideBySideCompareView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutSideBySideCompareView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToDual3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutDual3DView);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToNone()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutNone);
}

#endif
