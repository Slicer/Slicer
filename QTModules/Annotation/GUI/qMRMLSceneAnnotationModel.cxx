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

// Qt includes
#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLSceneAnnotationModel.h"
#include "qMRMLSceneDisplayableModel.h"
//#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>

// VTK includes
#include <vtkVariantArray.h>
#include <typeinfo>

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::qMRMLSceneAnnotationModel(QObject *vparent)
  :qMRMLSceneDisplayableModel(vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::~qMRMLSceneAnnotationModel()
{
}

