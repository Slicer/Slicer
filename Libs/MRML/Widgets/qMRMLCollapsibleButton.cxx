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
#include <QEvent>
#include <QStyle>

// VTK includes
#include <vtkSmartPointer.h>

// MRML includes
#include "vtkMRMLScene.h"

// qMRML includes
#include "qMRMLCollapsibleButton.h"

// --------------------------------------------------------------------------
class qMRMLCollapsibleButtonPrivate
{
public:
  vtkSmartPointer<vtkMRMLScene> MRMLScene;
};

// --------------------------------------------------------------------------
// qMRMLCollapsibleButton methods

// --------------------------------------------------------------------------
qMRMLCollapsibleButton::qMRMLCollapsibleButton(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLCollapsibleButtonPrivate)
{
}

//-----------------------------------------------------------------------------
qMRMLCollapsibleButton::~qMRMLCollapsibleButton() = default;

//-----------------------------------------------------------------------------
void qMRMLCollapsibleButton::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCollapsibleButton);
  if (scene == d->MRMLScene)
  {
    return;
  }
  d->MRMLScene = scene;

  emit mrmlSceneChanged(scene);
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLCollapsibleButton::mrmlScene() const
{
  Q_D(const qMRMLCollapsibleButton);
  return d->MRMLScene;
}
