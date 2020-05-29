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

#ifndef __qSlicerScriptedLoadableModuleFactory_h
#define __qSlicerScriptedLoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractFileBasedFactory.h>

// Slicer includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerScriptedLoadableModuleFactoryPrivate;

//----------------------------------------------------------------------------
class ctkFactoryScriptedItem : public ctkAbstractFactoryFileBasedItem<qSlicerAbstractCoreModule>
{
public:
  bool load() override;
protected:
  qSlicerAbstractCoreModule* instanciator() override;
};

//----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedLoadableModuleFactory :
  public ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerScriptedLoadableModuleFactory();
  ~qSlicerScriptedLoadableModuleFactory() override;

  bool registerScript(const QString& key, const QFileInfo& file);

  ///
  void registerItems() override;

protected:
  QScopedPointer<qSlicerScriptedLoadableModuleFactoryPrivate> d_ptr;

  bool isValidFile(const QFileInfo& file)const override;
  ctkAbstractFactoryItem<qSlicerAbstractCoreModule>*
    createFactoryFileBasedItem() override;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedLoadableModuleFactory);
  Q_DISABLE_COPY(qSlicerScriptedLoadableModuleFactory);
};

#endif
