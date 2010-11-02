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

///
/// vtkMRMLNodeInitializer is a default functor that should be dervied
/// in case specific node initialization steps are required
struct QMRML_WIDGETS_EXPORT vtkMRMLNodeInitializer
{
  virtual ~vtkMRMLNodeInitializer(){}
  virtual void operator()(vtkMRMLNode* node)const
    {
    Q_UNUSED(node);
    }
};

/// Node factory that can be used by qMRML widgets to easily create nodes
/// If you want more control over the node creation, you can add attributes,
/// specify a base node name or reimplement a vtkMRMLNodeInitializer
class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  Q_OBJECT
public:

  /// Convenient typedef 
  typedef QHash<QString,QString> AttributeType;
  
  /// Constructors
  typedef QObject Superclass;
  explicit qMRMLNodeFactory(QObject* parent = 0);
  virtual ~qMRMLNodeFactory();
  
  /// 
  /// Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  /// 
  /// Create and add a node given its \a className to the scene associated with the factory
  /// The attributes will be applied to the node after being initialized
  /// Note: The scene takes the ownership of the node and is responsible to delete it.
  vtkMRMLNode* createNode(const QString& className,
                          const vtkMRMLNodeInitializer & initializer = vtkMRMLNodeInitializer());

  /// 
  /// Convenient method allowing to create a new node and add it to the \a scene
  static vtkMRMLNode* createNode(vtkMRMLScene* scene, const QString& className,
    const vtkMRMLNodeInitializer & initializer = vtkMRMLNodeInitializer(),
    const AttributeType& attributes = AttributeType());

  /// 
  /// Add attribute that will be passed to any new created node.
  /// TODO: Support attributes for more than 1 node class
  /// Note: If an attribute already exist, it's value will be overwritten.
  void addAttribute(const QString& name, const QString& value);
  void removeAttribute(const QString& name);

  ///
  /// Base name used to generate a name for create node.
  void setBaseName(const QString& className, const QString& baseName);
  QString baseName(const QString& className)const;

public slots:
  /// 
  /// Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  
protected:
  QScopedPointer<qMRMLNodeFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeFactory);
  Q_DISABLE_COPY(qMRMLNodeFactory);
}; 

#endif
