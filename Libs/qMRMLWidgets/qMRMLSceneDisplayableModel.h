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

#ifndef __qMRMLSceneDisplayableModel_h
#define __qMRMLSceneDisplayableModel_h

#include "qMRMLSceneModel.h"

class qMRMLSceneDisplayableModelPrivate;
class vtkMRMLNode;
class QMRML_WIDGETS_EXPORT qMRMLSceneDisplayableModel : public qMRMLSceneModel
{
  Q_OBJECT

public:
  qMRMLSceneDisplayableModel(QObject *parent=0);
  virtual ~qMRMLSceneDisplayableModel();

  virtual Qt::DropActions supportedDropActions()const;

  /// 
  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;
  //virtual int          nodeIndex(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a child.
  virtual bool         canBeAChild(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a parent.
  virtual bool         canBeAParent(vtkMRMLNode* node)const;
  /// if newParent == 0, set the node into the vtkMRMLScene
  virtual bool         reparent(vtkMRMLNode* node, vtkMRMLNode* newParent);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneDisplayableModel);
  Q_DISABLE_COPY(qMRMLSceneDisplayableModel);
};

#endif
