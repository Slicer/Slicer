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

// Qt includes
#include <QAction>

// Slicer includes
#include "qSlicerAbstractModule.h"

class qSlicerAbstractModulePrivate
{
public:
  qSlicerAbstractModulePrivate();
  QAction* Action;
};

//-----------------------------------------------------------------------------
qSlicerAbstractModulePrivate::qSlicerAbstractModulePrivate()
{
  this->Action = nullptr;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule::qSlicerAbstractModule(QObject* parentObject)
  :Superclass(parentObject)
  , d_ptr(new qSlicerAbstractModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule::~qSlicerAbstractModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerAbstractModule::icon()const
{
  return QIcon(":Icons/Extension.png");
}

//-----------------------------------------------------------------------------
QImage qSlicerAbstractModule::logo()const
{
  return QImage();
}

//-----------------------------------------------------------------------------
QAction* qSlicerAbstractModule::action()
{
  Q_D(qSlicerAbstractModule);
  if (d->Action == nullptr)
    {
    d->Action = new QAction(this->icon(), this->title(), this);
    d->Action->setObjectName(QString("action%1").arg(this->name()));
    d->Action->setData(this->name());
    d->Action->setIconVisibleInMenu(true);
    d->Action->setProperty("index", this->index());
    }
  return d->Action;
}
