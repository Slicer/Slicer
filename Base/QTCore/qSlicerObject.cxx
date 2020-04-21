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

// Qt includes

// QTCore includes
#include "qSlicerObject.h"

// VTK includes
#include "vtkMRMLScene.h"
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class qSlicerObjectPrivate
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
qSlicerObject::qSlicerObject(): d_ptr(new qSlicerObjectPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerObject::~qSlicerObject() = default;

//-----------------------------------------------------------------------------
void qSlicerObject::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerObject);
  if (scene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = scene;
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerObject::mrmlScene()const
{
  Q_D(const qSlicerObject);
  return d->MRMLScene;
}
