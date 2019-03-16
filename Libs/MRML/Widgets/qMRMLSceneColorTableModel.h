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

#ifndef __qMRMLSceneColorTableModel_h
#define __qMRMLSceneColorTableModel_h

// qMRML includes
#include "qMRMLSceneCategoryModel.h"

// MRML includes
class vtkMRMLColorNode;

class qMRMLSceneColorTableModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneColorTableModel : public qMRMLSceneCategoryModel
{
  Q_OBJECT

public:
  qMRMLSceneColorTableModel(QObject *parent=nullptr);
  ~qMRMLSceneColorTableModel() override;

protected:
  QScopedPointer<qMRMLSceneColorTableModelPrivate> d_ptr;

  void updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column) override;
  bool updateGradientFromNode(vtkMRMLColorNode* node)const;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneColorTableModel);
  Q_DISABLE_COPY(qMRMLSceneColorTableModel);
};

#endif
