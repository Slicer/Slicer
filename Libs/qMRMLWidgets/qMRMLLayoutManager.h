/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qMRMLLayoutManager_h
#define __qMRMLLayoutManager_h

// CTK includes
#include <ctkLayoutManager.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLThreeDWidget;
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
  Q_INVOKABLE qMRMLSliceWidget* sliceWidget(const QString& name)const;

  /// Return the number of instantiated ThreeDRenderView
  int threeDViewCount()const;

  /// Get ThreeDRenderView identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qMRMLThreeDView (that should also be equal to the number
  /// of vtkMRMLViewNode)
  Q_INVOKABLE qMRMLThreeDWidget* threeDWidget(int id)const;

  /// Return the up-to-date list of vtkMRMLSliceLogics associated to the slice views.
  Q_INVOKABLE vtkCollection* mrmlSliceLogics()const;

  /// Returns the current layout. it's the same value than
  /// vtkMRMLLayoutNode::ViewArrangement
  /// \sa vtkMRMLLayoutNode::SlicerLayout
  int layout()const;

  vtkMRMLViewNode* activeMRMLThreeDViewNode()const;
  vtkRenderer* activeThreeDRenderer()const;

  Q_INVOKABLE vtkMRMLLayoutLogic* layoutLogic()const;
public slots:

  ///
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  /// Change the current layout (see vtkMRMLLayoutNode::SlicerLayout)
  /// It creates views if needed.
  void setLayout(int newLayout);

signals:
  void activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);
  void activeThreeDRendererChanged(vtkRenderer* newRenderer);
  void layoutChanged(int);

protected:
  QScopedPointer<qMRMLLayoutManagerPrivate> d_ptr;
  qMRMLLayoutManager(qMRMLLayoutManagerPrivate* obj, QWidget* viewport, QObject* parent);

  virtual void onViewportChanged();
  virtual void setupView(QDomElement viewElement, QWidget* view);
  virtual QWidget* viewFromXML(QDomElement layoutElement);
  virtual QList<QWidget*> viewsFromXML(QDomElement layoutElement);

  using ctkLayoutManager::setLayout;
private:
  Q_DECLARE_PRIVATE(qMRMLLayoutManager);
  Q_DISABLE_COPY(qMRMLLayoutManager);
};

#endif
