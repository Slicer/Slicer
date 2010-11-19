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

#ifndef __qMRMLSceneAnnotationModel_h
#define __qMRMLSceneAnnotationModel_h

#include "qMRMLSceneDisplayableModel.h"

#include "qSlicerAnnotationModuleExport.h"

class qMRMLSceneAnnotationModelPrivate;
class vtkMRMLNode;
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qMRMLSceneAnnotationModel : public qMRMLSceneDisplayableModel
{
  Q_OBJECT

public:
  qMRMLSceneAnnotationModel(QObject *parent=0);
  virtual ~qMRMLSceneAnnotationModel();

private:
  Q_DISABLE_COPY(qMRMLSceneAnnotationModel);
};

#endif
