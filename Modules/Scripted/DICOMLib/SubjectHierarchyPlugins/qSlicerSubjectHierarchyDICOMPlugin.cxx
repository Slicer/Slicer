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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDICOMPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// DICOMLib includes
#include "qSlicerDICOMExportDialog.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QRandomGenerator>
#include <QStandardItem>
#include <QMessageBox>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// STD includes
#include <algorithm>

// DCMTK includes
#include "dcmtk/dcmdata/dcuid.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyDICOMPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyDICOMPlugin);
protected:
  qSlicerSubjectHierarchyDICOMPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyDICOMPluginPrivate(qSlicerSubjectHierarchyDICOMPlugin& object);
  ~qSlicerSubjectHierarchyDICOMPluginPrivate() override;
  void init();
public:
  QIcon PatientIcon;
  QIcon StudyIcon;

  QAction* CreatePatientAction;
  QAction* CreateStudyAction;
  QAction* ConvertFolderToPatientAction;
  QAction* ConvertFolderToStudyAction;
  QAction* OpenDICOMExportDialogAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDICOMPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPluginPrivate::qSlicerSubjectHierarchyDICOMPluginPrivate(qSlicerSubjectHierarchyDICOMPlugin& object)
: q_ptr(&object)
{
  this->PatientIcon = QIcon(":Icons/Patient.png");
  this->StudyIcon = QIcon(":Icons/Study.png");

  this->CreatePatientAction = nullptr;
  this->CreateStudyAction = nullptr;
  this->ConvertFolderToPatientAction = nullptr;
  this->ConvertFolderToStudyAction = nullptr;
  this->OpenDICOMExportDialogAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyDICOMPlugin);

  // Place DICOM folder actions after core folder actions
  const int folderActionsBaseWeight = 20;

  this->CreatePatientAction = new QAction("Create new subject",q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->CreatePatientAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, folderActionsBaseWeight);
  QObject::connect(this->CreatePatientAction, SIGNAL(triggered()), q, SLOT(createSubjectItem()));

  this->CreateStudyAction = new QAction("Create child study",q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->CreateStudyAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, folderActionsBaseWeight+1);
  QObject::connect(this->CreateStudyAction, SIGNAL(triggered()), q, SLOT(createChildStudyUnderCurrentItem()));

  this->ConvertFolderToPatientAction = new QAction("Convert folder to subject",q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->ConvertFolderToPatientAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, folderActionsBaseWeight+2);
  QObject::connect(this->ConvertFolderToPatientAction, SIGNAL(triggered()), q, SLOT(convertCurrentItemToPatient()));

  this->ConvertFolderToStudyAction = new QAction("Convert folder to study",q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->ConvertFolderToStudyAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, folderActionsBaseWeight+3);
  QObject::connect(this->ConvertFolderToStudyAction, SIGNAL(triggered()), q, SLOT(convertCurrentItemToStudy()));

  this->OpenDICOMExportDialogAction = new QAction("Export to DICOM...",q);
  // Place DICOM export action lower in the default actions section because it is
  // better to have export features towards the end (after editing and importing actions).
  const int dicomExportActionWeight = 30;
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->OpenDICOMExportDialogAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, dicomExportActionWeight);
  QObject::connect(this->OpenDICOMExportDialogAction, SIGNAL(triggered()), q, SLOT(openDICOMExportDialog()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPluginPrivate::~qSlicerSubjectHierarchyDICOMPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDICOMPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPlugin::qSlicerSubjectHierarchyDICOMPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyDICOMPluginPrivate(*this) )
{
  this->m_Name = QString("DICOM");

  Q_D(qSlicerSubjectHierarchyDICOMPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDICOMPlugin::~qSlicerSubjectHierarchyDICOMPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyDICOMPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(node);
  Q_UNUSED(parentItemID);

  // The DICOM plugin is a subclass of the Folder plugin, but cannot add any node to subject hierarchy
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyDICOMPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // Patient level
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return 0.9;
    }
  // Study level
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return 0.9;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::roleForPlugin()const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return "Error!";
    }

  // Get current item to determine tole
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return "Error!";
    }

  // Patient level
  if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return "Subject"; // Show the role Subject to the user, while internally it is used for the patient notation defined in DICOM
    }
  // Study level
  if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return "Study";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDICOMPlugin::helpText()const
{
  return QString(
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Create new Subject from scratch"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click the top-level item 'Scene' (or the empty area if scene item is not visible) and select 'Create new subject'."
    "</span>"
    "</p>\n"
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Create new hierarchy item"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click on an existing item and select 'Create ...'. The possible types depend on the parent."
    "</span>"
    "</p>");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDICOMPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyDICOMPlugin);

  // Patient icon
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    return d->PatientIcon;
    }
  // Study icon
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    return d->StudyIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDICOMPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyDICOMPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyDICOMPlugin);

  QList<QAction*> actions;
  actions << d->CreateStudyAction << d->ConvertFolderToPatientAction << d->ConvertFolderToStudyAction << d->OpenDICOMExportDialogAction;
  return actions;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyDICOMPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyDICOMPlugin);

  QList<QAction*> actions;
  actions << d->CreatePatientAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyDICOMPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Scene
  if (itemID == shNode->GetSceneItemID())
    {
    d->CreatePatientAction->setVisible(true);
    return;
    }

  // Patient
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
    {
    d->CreateStudyAction->setVisible(true);
    }
  // Folder
  else if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    d->ConvertFolderToPatientAction->setVisible(true);
    d->ConvertFolderToStudyAction->setVisible(true);
    }
  // Study
  else if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    //if (this->canBeExported(node)) //TODO:
      {
      d->OpenDICOMExportDialogAction->setVisible(true);
      }
    }
  // Data node (Series)
  else if (shNode->GetItemDataNode(itemID))
    {
    //if (this->canBeExported(node)) //TODO:
      {
      d->OpenDICOMExportDialogAction->setVisible(true);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::editProperties(vtkIdType itemID)
{
  Q_UNUSED(itemID);
  //TODO: Show DICOM tag editor?
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::createSubjectItem()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // It is called Subject to the user, while internally it is used for the patient notation defined in DICOM
  std::string name = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix() + "Subject";
  name = shNode->GenerateUniqueItemName(name);

  // Create patient subject hierarchy item
  vtkIdType patientItemID = shNode->CreateSubjectItem(shNode->GetSceneItemID(), name);

  emit requestExpandItem(patientItemID);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::createChildStudyUnderCurrentItem()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  std::string name = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy();
  name = shNode->GenerateUniqueItemName(name);

  // Create study subject hierarchy item
  vtkIdType studyItemID = shNode->CreateStudyItem(currentItemID, name);

  emit requestExpandItem(studyItemID);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::convertCurrentItemToPatient()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Set level to patient to indicate new role
  shNode->SetItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::convertCurrentItemToStudy()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Set level to patient to indicate new role
  shNode->SetItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDICOMPlugin::openDICOMExportDialog()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Check whether there is a study and patient for the exported item
  bool validSelection = false;
  if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
    {
    if (shNode->IsItemLevel(shNode->GetItemParent(currentItemID), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
      {
      // Valid if current item is study and has a patient parent
      validSelection = true;
      }
    }
  else // Item belongs to a data node (i.e. series)
    {
    vtkIdType studyItemID = shNode->GetItemParent(currentItemID);
    if ( studyItemID && shNode->IsItemLevel(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy())
      && shNode->IsItemLevel(shNode->GetItemParent(studyItemID), vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()) )
      {
      // Valid if current item is a series and has a study parent which has a patient parent
      validSelection = true;
      }
    }
  if (!validSelection)
    {
    QString message = tr("Data to export need to be under a study item with a parent patient.\n"
                              "Default patient and study will be created and the selected data and its related datasets "
                              "will be moved in it for export.\n\n"
                              "If you'd like to create the hierarchy manually, please click Cancel, "
                              "then create a patient by right-clicking the empty area in Subject hierarchy in the Data module "
                              "and choosing 'Create new subject'. Study can be similarly created under the patient. The data "
                              "to export can be drag&dropped under the study.");
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Create new patient and study for DICOM export?"), message,
      QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
    if (answer == QMessageBox::Ok)
      {

      if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
        {
        // Create parent patient
        vtkIdType patientItemID = this->createDefaultPatientItem();
        // Move existing study under new patient
        shNode->SetItemParent(currentItemID, patientItemID);
        }
      else
        {
        // Collect referenced items for the selected data item
        // DICOM references
        std::vector<vtkIdType> referencedItems = shNode->GetItemsReferencedFromItemByDICOM(currentItemID);
        // MRML references available in SH
        vtkSmartPointer<vtkCollection> referencedNodes;
        referencedNodes.TakeReference(shNode->GetScene()->GetReferencedNodes(shNode->GetItemDataNode(currentItemID)));
        for (int index=0; index!=referencedNodes->GetNumberOfItems(); ++index)
          {
          vtkIdType nodeItemID = shNode->GetItemByDataNode(vtkMRMLNode::SafeDownCast(referencedNodes->GetItemAsObject(index)));
          if (nodeItemID && (std::find(referencedItems.begin(), referencedItems.end(), nodeItemID) == referencedItems.end()))
            {
            referencedItems.push_back(nodeItemID);
            }
          }

        // Check if referenced items are already in a study, and use that if found
        vtkIdType studyItemID = 0;
        for (std::vector<vtkIdType>::iterator itemIt=referencedItems.begin(); itemIt!=referencedItems.end(); itemIt++)
          {
          vtkIdType currentParentStudyItemID = shNode->GetItemAncestorAtLevel(
            *itemIt, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy() );
          if (currentParentStudyItemID && !studyItemID)
            {
            studyItemID = currentParentStudyItemID;
            }
          else if (currentParentStudyItemID)
            {
            qWarning() << Q_FUNC_INFO << ": Items referenced by '" << shNode->GetItemName(currentItemID).c_str()
              << "' are in multiple studies. The first found study will be used as parent for referenced data";
            }
          }

        // Create new study if no referenced data item is in a study already
        if (!studyItemID)
          {
          studyItemID = this->createDefaultStudyItem();

          // Create parent patient for new study
          vtkIdType patientItemID = this->createDefaultPatientItem();
          shNode->SetItemParent(studyItemID, patientItemID);
          }
        else if (!shNode->GetItemAncestorAtLevel(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient()))
          {
          // Create parent patient for found study if it had none
          vtkIdType patientItemID = this->createDefaultPatientItem();
          shNode->SetItemParent(studyItemID, patientItemID);
          }

        // Move selected item and all items it references under the study
        shNode->SetItemParent(currentItemID, studyItemID);
        for (std::vector<vtkIdType>::iterator itemIt=referencedItems.begin(); itemIt!=referencedItems.end(); itemIt++)
          {
          shNode->SetItemParent(*itemIt, studyItemID);
          }
        }
      }
    else
      {
      qWarning() << Q_FUNC_INFO << ": DICOM export cannot be performed without a parent patient and study. Manual interaction has been selected.";
      return;
      }
    }

  // Open export dialog
  qSlicerDICOMExportDialog* exportDialog = new qSlicerDICOMExportDialog(nullptr);
  exportDialog->setMRMLScene(qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene());
  exportDialog->exec(currentItemID);

  delete exportDialog;
}

//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyDICOMPlugin::createDefaultPatientItem()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Generate patient name
  QString patientName = tr("Anonymous");

  // Generate random patient ID
  const int patientIdLength = 6;
  QString patientID;
  for (int i = 0; i < patientIdLength; ++i)
    {
    patientID += QChar('A' + char(QRandomGenerator::global()->generate() % ('Z' - 'A')));
    }

  QString patientItemName = QString("%1 (%2)").arg(patientName).arg(patientID);
  vtkIdType patientItemID = shNode->CreateSubjectItem(shNode->GetSceneItemID(), patientItemName.toStdString());

  shNode->SetItemAttribute(patientItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMPatientNameAttributeName(), patientName.toStdString());
  shNode->SetItemAttribute(patientItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMPatientIDAttributeName(), patientID.toStdString());
  return patientItemID;
}

//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyDICOMPlugin::createDefaultStudyItem()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  QString studyDescription = tr("No study description");
  QDateTime studyDateTime = QDateTime::currentDateTime();
  QString studyDateStr = studyDateTime.toString("yyyyMMdd");
  QString studyTimeStr = studyDateTime.toString("hhmmss");
  char uidBuffer[100]; // must be at least 65 characters long
  QString studyInstanceUid = QString::fromLatin1(dcmGenerateUniqueIdentifier(uidBuffer, SITE_STUDY_UID_ROOT));

  // Create new study
  vtkIdType studyItemID = shNode->CreateStudyItem(shNode->GetSceneItemID(), QString("%1 (%2)").arg(studyDescription).arg(studyDateStr).toStdString());
  shNode->SetItemUID(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMUIDName(), studyInstanceUid.toStdString());
  shNode->SetItemAttribute(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMStudyInstanceUIDAttributeName(), studyInstanceUid.toStdString());
  shNode->SetItemAttribute(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDescriptionAttributeName(), studyDescription.toStdString());
  shNode->SetItemAttribute(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDateAttributeName(), studyDateStr.toStdString());
  shNode->SetItemAttribute(studyItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMStudyTimeAttributeName(), studyTimeStr.toStdString());

  return studyItemID;
}
