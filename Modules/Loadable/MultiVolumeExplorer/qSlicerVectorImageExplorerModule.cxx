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
#include <vtkSlicerVectorImageExplorerLogic.h>

// ExtensionTemplate includes
#include "qSlicerVectorImageExplorerModule.h"
#include "qSlicerVectorImageExplorerModuleWidget.h"

// SlicerQT includes
#include <qSlicerUtils.h>
#include <qSlicerModuleManager.h>
#include <qSlicerScriptedLoadableModuleWidget.h>
#include <vtkSlicerConfigure.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVectorImageExplorerModule, qSlicerVectorImageExplorerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVectorImageExplorerModulePrivate
{
public:
  qSlicerVectorImageExplorerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVectorImageExplorerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVectorImageExplorerModulePrivate::qSlicerVectorImageExplorerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVectorImageExplorerModule methods

//-----------------------------------------------------------------------------
qSlicerVectorImageExplorerModule::qSlicerVectorImageExplorerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVectorImageExplorerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVectorImageExplorerModule::~qSlicerVectorImageExplorerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVectorImageExplorerModule::helpText()const
{
  return "This VectorImageExplorer module illustrates how a loadable module should "
      "be implemented.";
}

//-----------------------------------------------------------------------------
QString qSlicerVectorImageExplorerModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerVectorImageExplorerModule::icon()const
{
  return QIcon(":/Icons/VectorImageExplorer.png");
}

//-----------------------------------------------------------------------------
QString qSlicerVectorImageExplorerModule::category() const 
{ 
  return QString("Vector Image Tools"); 
}

//-----------------------------------------------------------------------------
QStringList qSlicerVectorImageExplorerModule::dependencies() const
{
  return QStringList();
}


//-----------------------------------------------------------------------------
void qSlicerVectorImageExplorerModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerVectorImageExplorerModule::createWidgetRepresentation()
{  
  QString pythonPath = qSlicerUtils::pathWithoutIntDir(
              QFileInfo(this->path()).path(), Slicer_QTLOADABLEMODULES_LIB_DIR);

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerVectorImageExplorerModuleWidget";
  bool ret = widget->setPythonSource(
        pythonPath + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVectorImageExplorerModule::createLogic()
{
  return vtkSlicerVectorImageExplorerLogic::New();
}
