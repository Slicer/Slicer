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

#ifndef __qMRMLSceneColorTableModel2_h
#define __qMRMLSceneColorTableModel2_h

// qMRML includes
#include "qMRMLSceneCategoryModel2.h"

// MRML includes
class vtkMRMLColorTableNode;

class qMRMLSceneColorTableModel2Private;

class QMRML_WIDGETS_EXPORT qMRMLSceneColorTableModel2 : public qMRMLSceneCategoryModel2
{
  Q_OBJECT

public:
  qMRMLSceneColorTableModel2(QObject *parent=0);
  virtual ~qMRMLSceneColorTableModel2();

protected:
  QScopedPointer<qMRMLSceneColorTableModel2Private> d_ptr;

  virtual void updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  bool updateGradientFromNode(vtkMRMLColorTableNode* node)const;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneColorTableModel2);
  Q_DISABLE_COPY(qMRMLSceneColorTableModel2);
};

#endif
