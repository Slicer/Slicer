/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy_Widgets includes
#include "qSlicerDICOMExportDialog.h"
#include "ui_qSlicerDICOMExportDialog.h"

#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSceneSubjectHierarchyModel.h"

// SubjectHierarchy includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// DICOMLib includes
#include "qSlicerDICOMExportable.h"

// Qt includes
#include <QDialog>
#include <QObject>
#include <QDebug>
#include <QItemSelection>
#include <QListWidgetItem>
#include <QDateTime>
#include <QTimer>

// PythonQt includes
#include "PythonQt.h"

// MRML includes
#include <vtkMRMLScene.h>

// SlicerApp includes
#include "qSlicerApplication.h"

// CTK includes
// XXX Avoid  warning: "HAVE_STAT" redefined
#undef HAVE_STAT
#include "ctkDICOMDatabase.h"
#include "ctkDICOMIndexer.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerDICOMExportDialogPrivate : public Ui_qSlicerDICOMExportDialog, public QDialog
{
  Q_DECLARE_PUBLIC(qSlicerDICOMExportDialog);
protected:
  qSlicerDICOMExportDialog* const q_ptr;
public:
  qSlicerDICOMExportDialogPrivate(qSlicerDICOMExportDialog& object);
  virtual ~qSlicerDICOMExportDialogPrivate();
public:
  void init();
private:
  vtkMRMLScene* Scene;
  vtkMRMLSubjectHierarchyNode* NodeToSelect;
  qSlicerDICOMExportable* SelectedExportable;
};

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialogPrivate::qSlicerDICOMExportDialogPrivate(qSlicerDICOMExportDialog& object)
  : q_ptr(&object)
  , Scene(NULL)
  , NodeToSelect(NULL)
  , SelectedExportable(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialogPrivate::~qSlicerDICOMExportDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialogPrivate::init()
{
  Q_Q(qSlicerDICOMExportDialog);

  qMRMLSceneSubjectHierarchyModel* sceneModel = (qMRMLSceneSubjectHierarchyModel*)this->SubjectHierarchyTreeView->sceneModel();

  // Set up tree view
  this->SubjectHierarchyTreeView->setMRMLScene(this->Scene);
  this->SubjectHierarchyTreeView->setShowScene(false);
  this->SubjectHierarchyTreeView->expandToDepth(4);
  this->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->idColumn());
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->visibilityColumn());
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->transformColumn());

  // Empty error label (was not empty to indicate its purpose in designer)
  this->ErrorLabel->setText(QString());

  // Set Slicer DICOM database folder as default output folder
  this->DirectoryButton_OutputFolder->setDirectory(qSlicerApplication::application()->dicomDatabase()->databaseDirectory());

  // Make connections
  connect(this->SubjectHierarchyTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(onCurrentNodeChanged(vtkMRMLNode*)));
  connect(this->ExportablesListWidget, SIGNAL(currentRowChanged(int)), q, SLOT(onExportableSelectedAtRow(int)));
  connect(this->ExportButton, SIGNAL(clicked()), q, SLOT(onExport()));
  connect(this->ExportSeriesRadioButton, SIGNAL(toggled(bool)), q, SLOT(onExportSeriesRadioButtonToggled(bool)) );
}

//-----------------------------------------------------------------------------
// qSlicerDICOMExportDialog methods

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialog::qSlicerDICOMExportDialog(QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerDICOMExportDialogPrivate(*this))
{
  Q_D(qSlicerDICOMExportDialog);
  d->setupUi(d);
}

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialog::~qSlicerDICOMExportDialog()
{
}

//-----------------------------------------------------------------------------
bool qSlicerDICOMExportDialog::exec(vtkMRMLSubjectHierarchyNode* nodeToSelect/*=NULL*/)
{
  Q_D(qSlicerDICOMExportDialog);

  // Initialize dialog
  d->init();

  // Make selection if requested
  d->NodeToSelect = nodeToSelect;
  QTimer::singleShot(0, this, SLOT( selectNode() ) );

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
    {
    return result;
    }

  // Perform actions after clean exit
  result = true;

  return result;
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerDICOMExportDialog);
  d->Scene = scene;
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::selectNode()
{
  Q_D(qSlicerDICOMExportDialog);
  if (!d->NodeToSelect)
    {
    return;
    }
  d->SubjectHierarchyTreeView->setCurrentNode(d->NodeToSelect);
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onCurrentNodeChanged(vtkMRMLNode* node)
{
  Q_UNUSED(node)
  Q_D(qSlicerDICOMExportDialog);

  // Clear error label
  d->ErrorLabel->setText(QString());

  // Get exportables from DICOM plugins
  this->examineSelectedNode();
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::examineSelectedNode()
{
  Q_D(qSlicerDICOMExportDialog);

  // Get current node (single-selection)
  vtkMRMLSubjectHierarchyNode* currentNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(
    d->SubjectHierarchyTreeView->currentNode() );
  if (!currentNode)
    {
    qCritical() << "qSlicerDICOMExportDialog::examineSelectedNode: Unable to get current subject hierarchy node!";
    return;
    }

  // Clear exportables list
  d->ExportablesListWidget->clear();
  d->DICOMTagEditorWidget->clear();

  // Get child series nodes if selected node is study
  QList<vtkMRMLSubjectHierarchyNode*> selectedSeriesNodes;
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    std::vector<vtkMRMLHierarchyNode*> childrenNodes = currentNode->GetChildrenNodes();
    for ( std::vector<vtkMRMLHierarchyNode*>::iterator childIt = childrenNodes.begin();
      childIt != childrenNodes.end(); ++childIt)
      {
      vtkMRMLSubjectHierarchyNode* subjectHierarchySeriesNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(*childIt);
      if (subjectHierarchySeriesNode)
        {
        selectedSeriesNodes.append(subjectHierarchySeriesNode);
        }
      }
    }
  else if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries()))
    {
    selectedSeriesNodes.append(currentNode);
    }
  else
    {
    qCritical() << "qSlicerDICOMExportDialog::examineSelectedNode: Can only export series or study!";
    return;
    }

  // Get exportables from DICOM plugins for selection
  // One plugin should return one exportable for one series, but nevertheless
  // a list is returned for convenient concatenation (without type check etc.)
  QList<QVariant> exportablesVariantList;
  foreach (vtkMRMLSubjectHierarchyNode* selectedSeriesNode, selectedSeriesNodes)
    {
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    context.evalScript( QString(
      "exportables = []\n"
      "selectedNode = slicer.mrmlScene.GetNodeByID('%1')\n"
      "for pluginClass in slicer.modules.dicomPlugins:\n"
      "  plugin = slicer.modules.dicomPlugins[pluginClass]()\n"
      "  exportables.extend(plugin.examineForExport(selectedNode))\n" )
      .arg(selectedSeriesNode->GetID()) );

    // Extract resulting exportables from python
    exportablesVariantList.append(context.getVariable("exportables").toList());
    }

  // Group exportables by provider plugin
  QMap<QString,QList<qSlicerDICOMExportable*> > exportablesByPlugin;
  foreach(QVariant exportableVariant, exportablesVariantList)
    {
    // Get exportable object (to compose item text)
    qSlicerDICOMExportable* exportable = qobject_cast<qSlicerDICOMExportable*>(
      exportableVariant.value<QObject*>() );
    if (!exportable)
      {
      qCritical() << "qSlicerDICOMExportDialog::examineSelectedNode: Invalid exportable returned by DICOM plugin for " << currentNode->GetNameWithoutPostfix().c_str();
      continue;
      }

    QString plugin = exportable->pluginClass();
    if (!exportablesByPlugin.contains(plugin))
      {
      QList<qSlicerDICOMExportable*> firstExportableForPlugin;
      firstExportableForPlugin.append(exportable);
      exportablesByPlugin[plugin] = firstExportableForPlugin;
      }
    else
      {
      exportablesByPlugin[plugin].append(exportable);
      }
    }
  // Map the grouped exportables by confidence values so that the highest confidence is on top
  QMap<double,QList<qSlicerDICOMExportable*> > exportablesByConfidence;
  foreach(QList<qSlicerDICOMExportable*> exportablesForPlugin, exportablesByPlugin)
    {
    // Geometric mean to emphasize larger values
    double meanConfidenceForPlugin = 0.0;
    foreach (qSlicerDICOMExportable* exportable, exportablesForPlugin)
      {
      meanConfidenceForPlugin += exportable->confidence();
      }
    meanConfidenceForPlugin /= exportablesForPlugin.count();

    // Add exportable to map with confidence as key. Confidence value is subtracted
    // from 1 so that iterating through the map automatically orders the exportables.
    exportablesByConfidence[1.0 - meanConfidenceForPlugin] = exportablesForPlugin;
    }

  // Populate the exportables list widget
  foreach (double inverseConfidence, exportablesByConfidence.keys())
    {
    // Get exportable lists for the confidence number (there might be equality!)
    QList<QList<qSlicerDICOMExportable*> > exportableLists = exportablesByConfidence.values(inverseConfidence);
    foreach(QList<qSlicerDICOMExportable*> exportables, exportableLists)
      {
      // Set exportable name as the first one in the list, giving also the
      // confidence number and plugin name in parentheses
      QString itemText = QString("%1 (%2%, %3 series) (%4)").arg(exportables[0]->name())
        .arg((1.0-inverseConfidence)*100.0, 0, 'f', 0).arg(exportables.count()).arg(exportables[0]->pluginClass());
      QListWidgetItem* exportableItem = new QListWidgetItem(itemText, d->ExportablesListWidget);
      exportableItem->setToolTip(exportables[0]->tooltip());
      // Construct data variant object
      QList<QVariant> itemData;
      foreach (qSlicerDICOMExportable* exportable, exportables)
        {
        itemData.append(QVariant::fromValue<QObject*>(exportable));
        }
      exportableItem->setData(Qt::UserRole, itemData);
      }
    }

  // Select exportable with highest confidence (top one)
  d->ExportablesListWidget->setCurrentRow(0);
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onExportableSelectedAtRow(int row)
{
  Q_D(qSlicerDICOMExportDialog);

  // Clear error label
  d->ErrorLabel->setText(QString());

  // Get exportable item from row number
  QListWidgetItem* exportableItem = d->ExportablesListWidget->item(row);
  if (!exportableItem)
    {
    return;
    }

  // Get exportable object from list item
  QList<qSlicerDICOMExportable*> exportableList;
  QList<QVariant> itemData = exportableItem->data(Qt::UserRole).toList();
  foreach (QVariant exportableVariant, itemData)
    {
    qSlicerDICOMExportable* exportable = qobject_cast<qSlicerDICOMExportable*>(
      exportableVariant.value<QObject*>() );
    if (!exportable)
      {
      QString errorMessage("Unable to extract exportable");
      qCritical() << "qSlicerDICOMExportDialog::onExportableSelectedAtRow: " << errorMessage;
      d->ErrorLabel->setText(errorMessage);
      return;
      }
    exportableList.append(exportable);
    }

  // Populate DICOM tag editor from exportable
  d->DICOMTagEditorWidget->setMRMLScene(d->Scene);
  QString error = d->DICOMTagEditorWidget->setExportables(exportableList);
  if (!error.isEmpty())
    {
    d->ErrorLabel->setText(error);
    }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onExportSeriesRadioButtonToggled(bool seriesOn)
{
  Q_D(qSlicerDICOMExportDialog);

  // Export series
  if (seriesOn)
    {
    d->groupBox_1SelectNode->setEnabled(true);
    d->groupBox_2SelectExportType->setEnabled(true);
    d->groupBox_3EditDICOMTags->setEnabled(true);
    d->SaveTagsCheckBox->setEnabled(true);
    d->DirectoryButton_OutputFolder->setEnabled(true);
    d->ErrorLabel->setText(QString());
    }
  // Export entire scene
  else
    {
    d->groupBox_1SelectNode->setEnabled(false);
    d->groupBox_2SelectExportType->setEnabled(false);
    d->groupBox_3EditDICOMTags->setEnabled(false);
    d->SaveTagsCheckBox->setEnabled(false);
    d->DirectoryButton_OutputFolder->setEnabled(false);
    d->ErrorLabel->setText(QString());
    }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onExport()
{
  Q_D(qSlicerDICOMExportDialog);

  // Clear error label
  d->ErrorLabel->setText(QString());

  // Call export function based on radio button choice
  if (d->ExportSeriesRadioButton->isChecked())
    {
    this->exportSeries();
    }
  else
    {
    this->exportEntireScene();
    }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::showUpdatedDICOMBrowser()
{
  // Show DICOM browser and update DICOM database
  // (no direct function for it, so re-set the folder)
  PythonQt::init();
  PythonQtObjectPtr openBrowserContext = PythonQt::self()->getMainModule();
  openBrowserContext.evalScript( QString(
    "dicomWidget = slicer.modules.dicom.widgetRepresentation().self()\n"
    "dicomWidget.dicomBrowser.databaseDirectory = '%1'\n"
    "dicomWidget.detailsPopup.open()\n" )
    .arg(qSlicerApplication::application()->dicomDatabase()->databaseDirectory()) );
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::exportSeries()
{
  Q_D(qSlicerDICOMExportDialog);

  // Get output directory
  QDir outputFolder(d->DirectoryButton_OutputFolder->directory());

  // Determine whether output directory is a Slicer DICOM database
  bool isDicomDatabaseFolder = outputFolder.entryList().contains("ctkDICOM.sql");

  // Set output folder to a temporary location if the output directory is a DICOM database
  if (isDicomDatabaseFolder)
    {
    // Save to temporary folder and store files in database directory when adding
    outputFolder.setPath(qSlicerApplication::application()->temporaryPath());
    QString tempSubDirName = QString("DICOMExportTemp_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    outputFolder.mkdir(tempSubDirName);
    outputFolder.cd(tempSubDirName);
    }

  // Commit changes to exported series node and their study and patient
  // parents after successful export if user requested it
  if (d->SaveTagsCheckBox->isChecked())
    {
    d->DICOMTagEditorWidget->commitChangesToNodes();
    }

  if (d->DICOMTagEditorWidget->exportables().isEmpty())
    {
    return;
    }

  // Assemble list of exportables to pass to the DICOM plugin.
  // Pass whole list of exportables to plugins instead of one by one, in case it is
  // a composite export where the series are referenced from each other, or if the
  // file naming is static (to avoid overwrite)
  QList<QVariant> exportableList;
  foreach (qSlicerDICOMExportable* exportable, d->DICOMTagEditorWidget->exportables())
    {
    // Set output directory
    exportable->setDirectory(outputFolder.absolutePath());
    exportableList.append(QVariant::fromValue<QObject*>(exportable));
    }

  // Call export function of python DICOM plugin to save DICOM files to output folder
  // (The user ultimately selects a DICOM plugin, so all exportables belong to the same one)
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  PythonQt::init();
  PythonQtObjectPtr exportContext = PythonQt::self()->getMainModule();
  exportContext.addVariable("exportables", exportableList);
  exportContext.evalScript( QString(
    "plugin = slicer.modules.dicomPlugins['%1']()\n"
    "errorMessage = plugin.export(exportables)\n" )
    .arg(d->DICOMTagEditorWidget->exportables()[0]->pluginClass()) );
  QApplication::restoreOverrideCursor();

  // Extract error message from python
  QString errorMessage = exportContext.getVariable("errorMessage").toString();
  if (errorMessage.isNull())
    {
    // Invalid return value from DICOM exporter
    d->ErrorLabel->setText("Exporter returned with invalid value");
    }
  else if (!errorMessage.isEmpty())
    {
    // Exporter encountered error
    d->ErrorLabel->setText(errorMessage);
    return;
    }

  // Add exported files to DICOM database
  ctkDICOMIndexer* indexer = new ctkDICOMIndexer();
  ctkDICOMDatabase* dicomDatabase = qSlicerApplication::application()->dicomDatabase();
  QString destinationFolderPath("");
  if (isDicomDatabaseFolder)
    {
    // If we export to the DICOM database folder, then we need a non-empty destination path
    destinationFolderPath = qSlicerApplication::application()->dicomDatabase()->databaseDirectory();
    }
  indexer->addDirectory(*dicomDatabase, outputFolder.absolutePath(), destinationFolderPath);
  delete indexer;

  // Remove temporary DICOM folder if exported to the DICOM database folder
  if (isDicomDatabaseFolder)
    {
    foreach(QString file, outputFolder.entryList())
      {
      outputFolder.remove(file);
      }
    QString tempSubDirName = outputFolder.dirName();
    outputFolder.cdUp();
    outputFolder.rmdir(tempSubDirName);
    }

  // Show and update DICOM browser to indicate success
  this->showUpdatedDICOMBrowser();

  // Close the export dialog after successful export
  d->done(0);
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::exportEntireScene()
{
  Q_D(qSlicerDICOMExportDialog);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  PythonQt::init();
  PythonQtObjectPtr exportContext = PythonQt::self()->getMainModule();
  exportContext.evalScript( QString(
    "import DICOMLib\n"
    "exporter = DICOMLib.DICOMExportScene()\n"
    "exporter.export()\n") );

  QApplication::restoreOverrideCursor();

  // Show and update DICOM browser to indicate success
  this->showUpdatedDICOMBrowser();

  // Close the export dialog after successful export
  d->done(0);
}
