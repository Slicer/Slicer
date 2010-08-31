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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QTextStream>

// SlicerQt includes
#include "qSlicerCLILoadableModule.h"

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModulePrivate: public ctkPrivate<qSlicerCLILoadableModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerCLILoadableModule);
  typedef qSlicerCLILoadableModulePrivate Self;
  qSlicerCLILoadableModulePrivate();

  qSlicerCLILoadableModule::ModuleEntryPointType EntryPoint; 
};

//-----------------------------------------------------------------------------
// qSlicerCLILoadableModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCLILoadableModulePrivate::qSlicerCLILoadableModulePrivate()
{
  this->EntryPoint = 0; 
}

//-----------------------------------------------------------------------------
// qSlicerCLILoadableModule methods

//-----------------------------------------------------------------------------
qSlicerCLILoadableModule::qSlicerCLILoadableModule(QWidget* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerCLILoadableModule);
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModule::setup()
{
  Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::entryPoint()const
{
  CTK_D(const qSlicerCLILoadableModule);
  QString str;
  QTextStream(&str) << "slicer:" << (void*)(d->EntryPoint);
  return str; 
}

//-----------------------------------------------------------------------------
CTK_SET_CXX(qSlicerCLILoadableModule, ModuleEntryPointType, setEntryPoint, EntryPoint);

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::moduleType()const
{
  return QLatin1String("SharedObjectModule");
}
