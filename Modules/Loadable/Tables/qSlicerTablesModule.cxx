/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Qt includes
#include <QCoreApplication>
#include <QInputDialog>
#include <QLineEdit>

// Slice includes
#include <qSlicerCoreApplication.h>

// Tables Logic includes
#include <vtkSlicerTablesLogic.h>
#include <vtkSlicerTablesReader.h>
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLFileIOManager.h>

// Tables includes
#include "qSlicerTablesModule.h"
#include "qSlicerTablesModuleWidget.h"
// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTablesPlugin.h"

//-----------------------------------------------------------------------------
class qSlicerTablesModulePrivate
{
public:
  qSlicerTablesModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTablesModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTablesModulePrivate::qSlicerTablesModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerTablesModule methods

//-----------------------------------------------------------------------------
qSlicerTablesModule::qSlicerTablesModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTablesModule::~qSlicerTablesModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerTablesModule::icon() const
{
  return QIcon(":/Icons/Tables.png");
}

//-----------------------------------------------------------------------------
QString qSlicerTablesModule::helpText() const
{
  QString help = tr("The Tables module allows displaying and editing of spreadsheets.") + "<br>";
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerTablesModule::acknowledgementText() const
{
  return tr("This work was was partially funded by OCAIRO, the Applied"
            " Cancer Research Unit program of Cancer Care Ontario, and Department of"
            " Anesthesia and Critical Care Medicine,"
            " Children's Hospital of Philadelphia.");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andras Lasso (PerkLab), Kevin Wang (PMH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::categories() const
{
  return QStringList() << qSlicerAbstractCoreModule::tr("Informatics");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTablesModule::setup()
{
  this->Superclass::setup();

  // Readers and writers are registered by the module logic
  vtkSlicerTablesReader* tablesReader =
    vtkSlicerTablesReader::SafeDownCast(this->appLogic() ? this->appLogic()->GetFileIOManager()->GetReaderByClassName("vtkSlicerTablesReader") : nullptr);
  // Ask the user for the password if a database cannot be opened without a password
  qvtkConnect(tablesReader, vtkSlicerTablesReader::PasswordRequestedEvent, this, SLOT(onPasswordRequested(vtkObject*, void*)));
  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyTablesPlugin());
}

//-----------------------------------------------------------------------------
void qSlicerTablesModule::onPasswordRequested(vtkObject* vtkNotUsed(caller), void* callData)
{
  std::string* password = reinterpret_cast<std::string*>(callData);
  if (!password)
  {
    return;
  }
  bool ok = false;
  QString text = QInputDialog::getText(nullptr,
                                       QCoreApplication::translate("qSlicerTablesReader", "QInputDialog::getText()"),
                                       QCoreApplication::translate("qSlicerTablesReader", "Database Password:"),
                                       QLineEdit::Normal,
                                       "",
                                       &ok);
  if (ok && !text.isEmpty())
  {
    *password = text.toStdString();
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTablesModule::createWidgetRepresentation()
{
  return new qSlicerTablesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTablesModule::createLogic()
{
  return vtkSlicerTablesLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLTableNode";
}
