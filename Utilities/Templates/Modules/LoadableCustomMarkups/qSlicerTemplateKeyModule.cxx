/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "qSlicerTemplateKeyModule.h"

#include "vtkMRMLMarkupsTestLineNode.h"
#include "qSlicerMarkupsTestLineWidget.h"
#include "qSlicerMarkupsAdditionalOptionsWidgetsFactory.h"

// Qt includes
#include <QDebug>

// Liver Markups Logic includes
#include "vtkSlicerTemplateKeyLogic.h"

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>

// Markups VTKWidgets includes
#include <vtkSlicerLineWidget.h>

// Liver Markups VTKWidgets includes
#include <vtkSlicerTestLineWidget.h>

#include <qSlicerModuleManager.h>
#include <qSlicerApplication.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTemplateKeyModulePrivate
{
public:
  qSlicerTemplateKeyModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModulePrivate::qSlicerTemplateKeyModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModule methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::qSlicerTemplateKeyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTemplateKeyModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::~qSlicerTemplateKeyModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTemplateKeyModule::helpText() const
{
  return "This module contains fundamental markups to be used in the Slicer-Liver extension.";
}

//-----------------------------------------------------------------------------
QString qSlicerTemplateKeyModule::acknowledgementText() const
{
  return "This work has been partially funded by The Research Council of Norway (grant nr. 311393)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Rafael Palomar (Oslo University Hospital / NTNU)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTemplateKeyModule::icon() const
{
  return QIcon(":/Icons/TemplateKey.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::categories() const
{
  return QStringList() << "Testing.TestCases";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::dependencies() const
{
  return QStringList() << "Markups";
}

//-----------------------------------------------------------------------------
void qSlicerTemplateKeyModule::setup()
{
  this->Superclass::setup();

 vtkSlicerApplicationLogic* appLogic = this->appLogic();
 if (!appLogic)
   {
   qCritical() << Q_FUNC_INFO << " : invalid application logic.";
   return;
   }

 vtkSlicerMarkupsLogic* markupsLogic =
   vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
 if (!markupsLogic)
   {
   qCritical() << Q_FUNC_INFO << " : invalid markups logic.";
   return;
   }

  // If testing is enabled then we register the new markup
  bool isTestingEnabled = qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  if (isTestingEnabled)
    {
    // Register markups
    vtkNew<vtkMRMLMarkupsTestLineNode> markupsTestLineNode;
    vtkNew<vtkSlicerTestLineWidget> testLineWidget;
    markupsLogic->RegisterMarkupsNode(markupsTestLineNode, testLineWidget);
    }

  auto optionsWidgetFactory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  optionsWidgetFactory->registerAdditionalOptionsWidget(new qSlicerMarkupsTestLineWidget());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTemplateKeyModule
::createWidgetRepresentation()
{
    return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTemplateKeyModule::createLogic()
{
  return vtkSlicerTemplateKeyLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::associatedNodeTypes() const
{
 return QStringList() << "vtkMRMLMarkupsTestLineNode";
}

//-----------------------------------------------------------------------------
void qSlicerTemplateKeyModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  vtkSlicerTemplateKeyLogic* logic =
    vtkSlicerTemplateKeyLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << " failed: logic is invalid";
    return;
    }
}
