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

// Slicer includes
#include "qSlicerAbstractModulePanel.h"
#include "qSlicerModuleManager.h"

//---------------------------------------------------------------------------
class qSlicerAbstractModulePanelPrivate
{
public:
  qSlicerAbstractModulePanelPrivate() = default;
  qSlicerModuleManager* ModuleManager{nullptr};
};

//---------------------------------------------------------------------------
qSlicerAbstractModulePanel::qSlicerAbstractModulePanel(QWidget* _parent, Qt::WindowFlags f)
  : qSlicerWidget(_parent, f)
  , d_ptr(new qSlicerAbstractModulePanelPrivate)
{
}

//---------------------------------------------------------------------------
qSlicerAbstractModulePanel::~qSlicerAbstractModulePanel() = default;

//---------------------------------------------------------------------------
void qSlicerAbstractModulePanel::setModuleManager(qSlicerModuleManager* moduleManager)
{
  Q_D(qSlicerAbstractModulePanel);
  d->ModuleManager = moduleManager;
}

//---------------------------------------------------------------------------
qSlicerModuleManager* qSlicerAbstractModulePanel::moduleManager()const
{
  Q_D(const qSlicerAbstractModulePanel);
  return d->ModuleManager;
}

//---------------------------------------------------------------------------
void qSlicerAbstractModulePanel::addModule(const QString& moduleName)
{
  if (!this->moduleManager())
    {
    return;
    }
  qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
  Q_ASSERT(module);
  this->addModule(module);
}

//---------------------------------------------------------------------------
void qSlicerAbstractModulePanel::removeModule(const QString& moduleName)
{
  if (!this->moduleManager())
    {
    return;
    }
  qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
  Q_ASSERT(module);
  this->removeModule(module);
}
