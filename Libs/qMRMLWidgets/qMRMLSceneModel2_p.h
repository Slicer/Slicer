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

#ifndef __qMRMLSceneModel2_p_h
#define __qMRMLSceneModel2_p_h

// Qt includes
class QStandardItemModel;

// qMRML includes
#include "qMRMLSceneModel2.h"

// MRML includes
class vtkMRMLScene;

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
// qMRMLSceneModel2Private
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneModel2Private
{
  Q_DECLARE_PUBLIC(qMRMLSceneModel2);
protected:
  qMRMLSceneModel2* const q_ptr;
public:
  qMRMLSceneModel2Private(qMRMLSceneModel2& object);
  virtual ~qMRMLSceneModel2Private();
  void init();
  QStringList extraItems(QStandardItem* parent, const QString extraType)const;
  void removeAllExtraItems(QStandardItem* parent, const QString extraType);
  void listenNodeModifiedEvent();

  vtkSmartPointer<vtkCallbackCommand> CallBack;
  bool ListenNodeModifiedEvent;
  vtkMRMLScene* MRMLScene;
  mutable QModelIndexList LastMimeData;
};

#endif
