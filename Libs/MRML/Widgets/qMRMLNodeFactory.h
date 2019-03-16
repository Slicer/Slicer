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

#ifndef __qMRMLNodeFactory_h
#define __qMRMLNodeFactory_h

// Qt includes
#include <QObject>
#include <QHash>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLScene;
class qMRMLNodeFactoryPrivate;

/// Node factory that can be used by qMRML widgets to easily create nodes
/// If you want more control over the node creation, you can add attributes,
/// specify a base node name or connect to signals to customize the node
class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  Q_OBJECT
public:

  /// Convenient typedef
  typedef QHash<QString,QString> AttributeType;

  /// Constructors
  typedef QObject Superclass;
  explicit qMRMLNodeFactory(QObject* parent = nullptr);
  ~qMRMLNodeFactory() override;

  /// Get MRML scene.
  /// By default, there is no scene.
  vtkMRMLScene* mrmlScene()const;

  ///
  /// Create and add a node given its \a className to the scene associated
  /// with the factory. The function will fire the signals:
  /// \a nodeInstantiated(vtkMRMLNode*),
  /// \a nodeInitialized(vtkMRMLNode*),
  /// \a nodeAdded(vtkMRMLNode*)
  /// on that order. It allows the user to add custom steps by connecting slots
  /// to the emitted signals.
  /// No-op if the scene is 0 or if className is empty/null. However, if className
  /// is not empty but invalid (not an existing node), an assert throws an exception.
  /// Note: The attributes will be applied to the node before being added into
  /// the scene. The scene takes the ownership of the node and is responsible
  /// to delete it
  /// If the node is a singleton that already exists in the scene, the existing
  /// node is returned.
  /// \sa vtkMRMLScene::AddNode
  Q_INVOKABLE vtkMRMLNode* createNode(const QString& className);

  ///
  /// Convenient method allowing to create a new node and add it to the \a scene
  static vtkMRMLNode* createNode(vtkMRMLScene* scene, const QString& className,
    const AttributeType& attributes = AttributeType());

  ///
  /// Add attribute that will be passed to any new created node.
  /// TODO: Support attributes for more than 1 node class
  /// Note: If an attribute already exist, it's value will be overwritten.
  Q_INVOKABLE void addAttribute(const QString& attributeName, const QString& attributeValue);
  Q_INVOKABLE void removeAttribute(const QString& attributeName);
  Q_INVOKABLE QString attribute(const QString& attributeName)const;

  /// Base name used to generate a name for create node.
  Q_INVOKABLE void setBaseName(const QString& className, const QString& baseName);
  Q_INVOKABLE QString baseName(const QString& className)const;

public slots:
  ///
  /// Set MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);

signals:
  /// Fired right after the instantiation of the node
  /// (before any initialization)
  /// Connecting to the following signal allows a custom node creation
  void nodeInstantiated(vtkMRMLNode* node);
  /// Eventually fired by the function createNode
  /// You can add the node into the scene directly here.
  /// If no slot adds the node into the scene(node->GetScene() == 0), then the
  /// node factory takes care of adding the node into the scene (default
  /// behavior).
  void nodeInitialized(vtkMRMLNode* node);
  /// Fired at the end when the node is added into the scene. It is emitted
  /// even if the node has been added to the scene by a custom slot connected
  /// to \a nodeInitialized(vtkMRMLNode*)
  void nodeAdded(vtkMRMLNode* node);

protected:
  QScopedPointer<qMRMLNodeFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeFactory);
  Q_DISABLE_COPY(qMRMLNodeFactory);
};

#endif
