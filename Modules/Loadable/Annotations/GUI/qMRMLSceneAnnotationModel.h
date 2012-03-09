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

#ifndef __qMRMLSceneAnnotationModel_h
#define __qMRMLSceneAnnotationModel_h

#include "qMRMLSceneDisplayableModel.h"

// Annotation QT includes
#include "qSlicerAnnotationsModuleExport.h"

// Logic includes
class vtkSlicerAnnotationModuleLogic;

class qMRMLSceneAnnotationModelPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qMRMLSceneAnnotationModel : public qMRMLSceneDisplayableModel
{
  Q_OBJECT

public:
  qMRMLSceneAnnotationModel(QObject *parent=0);
  virtual ~qMRMLSceneAnnotationModel();

  // Register the logic
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);

  // Enum for the different columns
  enum Columns{
    CheckedColumn = 0,
    VisibilityColumn = 1,
    LockColumn = 2,
    EditColumn = 3,
    NameColumn = 5,
    ValueColumn = 4,
    TextColumn = 6
  };

  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);

protected:

  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;

  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);
  
  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;

private:
  Q_DISABLE_COPY(qMRMLSceneAnnotationModel);

  vtkSlicerAnnotationModuleLogic* m_Logic;

};

#endif
