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

#ifndef __qMRMLSceneFactoryWidget_h
#define __qMRMLSceneFactoryWidget_h

// Qt includes
#include <QPushButton>
#include <QString>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLSceneFactoryWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneFactoryWidget : public QWidget
{
  Q_OBJECT
public:
  qMRMLSceneFactoryWidget(QWidget* parent = nullptr);
  ~qMRMLSceneFactoryWidget() override;

  vtkMRMLScene* mrmlScene()const;

public slots:
  void generateScene();
  void deleteScene();
  ///
  /// Create and add a node given its classname to the scene associated with the factory
  /// Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* generateNode(const QString& mrmlNodeClassName);
  void deleteNode(const QString& mrmlNodeID);

  vtkMRMLNode* generateNode();
  void deleteNode();

signals:
  void mrmlSceneChanged(vtkMRMLScene* scene);
  void mrmlNodeAdded(vtkMRMLNode* node);
  void mrmlNodeRemoved(vtkMRMLNode* node);

protected:
  QScopedPointer<qMRMLSceneFactoryWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneFactoryWidget);
  Q_DISABLE_COPY(qMRMLSceneFactoryWidget);
};

#endif
