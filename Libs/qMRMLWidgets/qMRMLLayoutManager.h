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

#ifndef __qMRMLLayoutManager_h
#define __qMRMLLayoutManager_h

// CTK includes
#include <ctkLayoutManager.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>

class qMRMLThreeDView;
class qMRMLSliceWidget;
class qMRMLLayoutManagerPrivate;
class vtkCollection;
class vtkMRMLLayoutLogic;
class vtkMRMLScene;
class vtkMRMLViewNode;
class vtkRenderer;

class QMRML_WIDGETS_EXPORT qMRMLLayoutManager : public ctkLayoutManager
{
  Q_OBJECT
  // The following properties are exposed so that they are available within python
  Q_PROPERTY(int layout READ layout WRITE setLayout NOTIFY layoutChanged DESIGNABLE false)
  Q_PROPERTY(int threeDViewCount READ threeDViewCount DESIGNABLE false)
public:
  /// Superclass typedef
  typedef ctkLayoutManager Superclass;

  /// Constructors
  explicit qMRMLLayoutManager(QObject* parent=0);
  explicit qMRMLLayoutManager(QWidget* viewport, QObject* parent);
  virtual ~qMRMLLayoutManager();

  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

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
  vtkRenderer* activeThreeDRenderer()const;

  vtkMRMLLayoutLogic* layoutLogic()const;
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
  inline void switchToSideBySideLightboxView();
  inline void switchToDual3DView();
  inline void switchToNone();

  /// Generic function
  void setLayout(int);

signals:
  void activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);
  void activeThreeDRendererChanged(vtkRenderer* newRenderer);
  void layoutChanged(int);

protected:
  QScopedPointer<qMRMLLayoutManagerPrivate> d_ptr;
  qMRMLLayoutManager(qMRMLLayoutManagerPrivate* obj, QWidget* viewport, QObject* parent);

  virtual void onViewportChanged();
  virtual QWidget* processViewElement(QDomElement viewElement);
  virtual QWidget* viewFromXML(QDomElement layoutElement);

  using ctkLayoutManager::setLayout;
private:
  Q_DECLARE_PRIVATE(qMRMLLayoutManager);
  Q_DISABLE_COPY(qMRMLLayoutManager);
};

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToConventionalView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToOneUp3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToOneUpRedSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToOneUpYellowSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToOneUpGreenSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToFourUpView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToTabbed3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToTabbedSliceView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToLightboxView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutLightboxView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToCompareView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToSideBySideLightboxView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToDual3DView()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutDual3DView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToNone()
{
  this->setLayout(vtkMRMLLayoutNode::SlicerLayoutNone);
}

#endif
