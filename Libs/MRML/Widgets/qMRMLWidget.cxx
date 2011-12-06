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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLWidgets includes
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qMRMLWidgetPrivate
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
// qMRMLWidget methods

//-----------------------------------------------------------------------------
qMRMLWidget::qMRMLWidget(QWidget * _parent, Qt::WindowFlags f):Superclass(_parent, f)
  , d_ptr(new qMRMLWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qMRMLWidget::~qMRMLWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLWidget);
  if (newScene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = newScene;
  emit mrmlSceneChanged(newScene);
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLWidget::mrmlScene() const
{
  Q_D(const qMRMLWidget);
  return d->MRMLScene;
}
