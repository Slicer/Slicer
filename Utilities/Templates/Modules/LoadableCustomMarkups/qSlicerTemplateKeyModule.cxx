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

// Qt includes
#include <QDebug>

// TemplateKey Logic includes
#include "vtkSlicerTemplateKeyLogic.h"

// TemplateKey Widgets includes
#include "qMRMLMarkupsTestLineWidget.h"

// Markups Widgets includes
#include "qMRMLMarkupsOptionsWidgetsFactory.h"

#include <qSlicerModuleManager.h>
#include <qSlicerApplication.h>

//-----------------------------------------------------------------------------
class qSlicerTemplateKeyModulePrivate
{
public:
  qSlicerTemplateKeyModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModulePrivate::qSlicerTemplateKeyModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModule methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::qSlicerTemplateKeyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTemplateKeyModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::~qSlicerTemplateKeyModule() {}

//-----------------------------------------------------------------------------
bool qSlicerTemplateKeyModule::isHidden() const
{
  // The module has no GUI.
  // Widget options will be shown in Markups module.
  return true;
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

  // This is a test class, therefore we do not register anything if
  // not in testing mode (to avoid cluttering the markups module).
  bool isTestingEnabled = qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  if (!isTestingEnabled)
  {
    return;
  }

  // Create and configure the options widgets
  auto optionsWidgetFactory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  optionsWidgetFactory->registerOptionsWidget(new qMRMLMarkupsTestLineWidget());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTemplateKeyModule::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTemplateKeyModule::createLogic()
{
  // This is a test class, therefore we do not register anything (to avoid cluttering the markups module)
  // unless the application is in testing mode.
  bool isTestingEnabled = qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  if (!isTestingEnabled)
  {
    return nullptr;
  }

  return vtkSlicerTemplateKeyLogic::New();
}
