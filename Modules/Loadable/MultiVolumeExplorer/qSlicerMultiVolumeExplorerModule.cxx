/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>
#include <QFileInfo>
#include <QtPlugin>
#include <QScopedPointer>

// ExtensionTemplate Logic includes
#include <vtkSlicerMultiVolumeExplorerLogic.h>

// ExtensionTemplate includes
#include "qSlicerMultiVolumeExplorerModule.h"
#include "qSlicerMultiVolumeExplorerModuleWidget.h"

// SlicerQT includes
#include <qSlicerUtils.h>
#include <qSlicerModuleManager.h>
#include <qSlicerScriptedLoadableModuleWidget.h>
#include <vtkSlicerConfigure.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMultiVolumeExplorerModule, qSlicerMultiVolumeExplorerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMultiVolumeExplorerModulePrivate
{
public:
  qSlicerMultiVolumeExplorerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModulePrivate::qSlicerMultiVolumeExplorerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModule methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModule::qSlicerMultiVolumeExplorerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMultiVolumeExplorerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModule::~qSlicerMultiVolumeExplorerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMultiVolumeExplorerModule::helpText()const
{
  return "This MultiVolumeExplorer module illustrates how a loadable module should "
      "be implemented.";
}

//-----------------------------------------------------------------------------
QString qSlicerMultiVolumeExplorerModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerMultiVolumeExplorerModule::icon()const
{
  return QIcon(":/Icons/MultiVolumeExplorer.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeExplorerModule::categories() const 
{ 
  return QStringList() << "Vector Image Tools"; 
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeExplorerModule::dependencies() const
{
  return QStringList();
}


//-----------------------------------------------------------------------------
void qSlicerMultiVolumeExplorerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMultiVolumeExplorerModule::createWidgetRepresentation()
{  
  QString pythonPath = qSlicerUtils::pathWithoutIntDir(
              QFileInfo(this->path()).path(), Slicer_QTLOADABLEMODULES_LIB_DIR);

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerMultiVolumeExplorerModuleWidget";
  bool ret = widget->setPythonSource(
        pythonPath + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMultiVolumeExplorerModule::createLogic()
{
  return vtkSlicerMultiVolumeExplorerLogic::New();
}
