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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSceneTransformModel2_h
#define __qMRMLSceneTransformModel2_h

#include "qMRMLSceneModel2.h"

class qMRMLSceneTransformModel2Private;

class QMRML_WIDGETS_EXPORT qMRMLSceneTransformModel2 : public qMRMLSceneModel2
{
  Q_OBJECT

public:
  qMRMLSceneTransformModel2(QObject *parent=0);
  virtual ~qMRMLSceneTransformModel2();

  virtual Qt::DropActions supportedDropActions()const;

protected:
  virtual void populateScene();
  virtual void insertNode(vtkMRMLNode* node);
  using qMRMLSceneModel2::insertNode;
  virtual void updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual void updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneTransformModel2);
  Q_DISABLE_COPY(qMRMLSceneTransformModel2);
};

#endif
