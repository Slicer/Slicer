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

// SlicerQt includes
#include "qSlicerCLILoadableModule.h"

// STD includes
#include <cstdio>

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModulePrivate
{
public:
  qSlicerCLILoadableModulePrivate();
  typedef qSlicerCLILoadableModulePrivate Self;

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
  , d_ptr(new qSlicerCLILoadableModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCLILoadableModule::~qSlicerCLILoadableModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModule::setup()
{
  Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::entryPoint()const
{
  Q_D(const qSlicerCLILoadableModule);
  char buffer[256];
  // The entry point address must be encoded the same way it is decoded. As it
  // is decoded using  sscanf, it must be encoded using sprintf
  sprintf(buffer, "slicer:%p", d->EntryPoint);
  return QString(buffer); 
}

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerCLILoadableModule, ModuleEntryPointType, setEntryPoint, EntryPoint);

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::moduleType()const
{
  return QLatin1String("SharedObjectModule");
}
