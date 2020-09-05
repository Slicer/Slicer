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

// DICOMLib includes
#include "qSlicerDICOMTagEditorWidget.h"
#include "qSlicerDICOMExportable.h"

#include <qSlicerApplication.h>

// Qt includes
#include <QDebug>
#include <QHeaderView>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

// STD includes
#include <algorithm>

// SubjectHierarchy includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerDICOMTagEditorWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerDICOMTagEditorWidget);
protected:
  qSlicerDICOMTagEditorWidget* const q_ptr;
public:
  qSlicerDICOMTagEditorWidgetPrivate(qSlicerDICOMTagEditorWidget& object);
  virtual void init();
  void setupUi(QWidget *qSlicerDICOMTagEditorWidget);

  /// Clear tags table and add divider rows for patient and study
  void initializeTagsTable();

  /// Find header row index of the topmost series (to be able to insert study tags)
  unsigned int topSeriesHeaderRow();

  /// Find exportable for tags table row index
  /// \return nullptr if patient or study row, or if header row. The corresponding exportable otherwise.
  qSlicerDICOMExportable* exportableForRowIndex(unsigned int row);

  /// Insert empty row in tags table and make sure the header row indices are updated accordingly
  void insertTagsTableRow(unsigned int row);

public:
  QList<qSlicerDICOMExportable*> Exportables;
  vtkMRMLScene* Scene;

  /// Table containing all the tags (patient, study, series)
  QTableWidget* TagsTable;

  /// Row index of the study tags header (to be able to insert patient tags). Patient header is always at row 0
  unsigned int StudyTagsHeaderRow;
  /// Row indices of the series tags headers (to be able to insert study and series tags),
  /// associated with the corresponding exportable object
  QMap<unsigned int, qSlicerDICOMExportable*> SeriesTagsHeaderRows;

  /// Color of the header items
  QColor HeaderItemColor;
  /// Font of the header items
  QFont HeaderItemFont;
};

//------------------------------------------------------------------------------
qSlicerDICOMTagEditorWidgetPrivate::qSlicerDICOMTagEditorWidgetPrivate(qSlicerDICOMTagEditorWidget& object)
  : q_ptr(&object)
{
  this->Scene = nullptr;
  this->TagsTable = nullptr;
  this->StudyTagsHeaderRow = 0;

  QPalette palette = qSlicerApplication::application()->palette();

  this->HeaderItemColor = palette.color(QPalette::AlternateBase);
  this->HeaderItemFont = QFont();
  this->HeaderItemFont.setBold(true);
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidgetPrivate::init()
{
  Q_Q(qSlicerDICOMTagEditorWidget);

  // Clear tag editor table and exportables
  q->clear();

  // Make connections for setting edited values for common tags
  QObject::connect( this->TagsTable, SIGNAL(cellChanged(int,int)), q, SLOT(tagsTableCellChanged(int,int)) );
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidgetPrivate::setupUi(QWidget *qSlicerDICOMTagEditorWidget)
{
  if (qSlicerDICOMTagEditorWidget->objectName().isEmpty())
    {
    qSlicerDICOMTagEditorWidget->setObjectName(QString::fromUtf8("qSlicerDICOMTagEditorWidget"));
    }

  // Create layout
  QVBoxLayout* mainLayout = new QVBoxLayout(qSlicerDICOMTagEditorWidget);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Create tags table
  this->TagsTable = new QTableWidget(qSlicerDICOMTagEditorWidget);
  this->TagsTable->setColumnCount(2);
  this->TagsTable->horizontalHeader()->setVisible(false);
  this->TagsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  this->TagsTable->verticalHeader()->setVisible(false);
  this->TagsTable->horizontalHeader()->setStretchLastSection(true);
  this->TagsTable->setSelectionMode(QAbstractItemView::NoSelection);
  this->TagsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainLayout->addWidget(this->TagsTable);
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidgetPrivate::initializeTagsTable()
{
  // Reset study header
  this->StudyTagsHeaderRow = 1;
  // Clear series headers
  this->SeriesTagsHeaderRows.clear();

  // Empty tags table
  this->TagsTable->setRowCount(2);

  // Populate patient and study headers
  QTableWidgetItem* patientHeaderNameItem = new QTableWidgetItem("Patient tag name", 1);
  patientHeaderNameItem->setBackgroundColor(this->HeaderItemColor);
  patientHeaderNameItem->setFont(this->HeaderItemFont);
  patientHeaderNameItem->setFlags(Qt::ItemIsEnabled); // Non-editable
  this->TagsTable->setItem(0,0,patientHeaderNameItem);

  QTableWidgetItem* patientHeaderValueItem = new QTableWidgetItem("Value", 1);
  patientHeaderValueItem->setBackgroundColor(this->HeaderItemColor);
  patientHeaderValueItem->setFont(this->HeaderItemFont);
  patientHeaderValueItem->setFlags(Qt::ItemIsEnabled); // Non-editable
  this->TagsTable->setItem(0,1,patientHeaderValueItem);

  QTableWidgetItem* studyHeaderNameItem = new QTableWidgetItem("Study tag name", 1);
  studyHeaderNameItem->setBackgroundColor(this->HeaderItemColor);
  studyHeaderNameItem->setFont(this->HeaderItemFont);
  studyHeaderNameItem->setFlags(Qt::ItemIsEnabled); // Non-editable
  this->TagsTable->setItem(1,0,studyHeaderNameItem);

  QTableWidgetItem* studyHeaderValueItem = new QTableWidgetItem("Value", 1);
  studyHeaderValueItem->setBackgroundColor(this->HeaderItemColor);
  studyHeaderValueItem->setFont(this->HeaderItemFont);
  studyHeaderValueItem->setFlags(Qt::ItemIsEnabled); // Non-editable
  this->TagsTable->setItem(1,1,studyHeaderValueItem);
}

//------------------------------------------------------------------------------
unsigned int qSlicerDICOMTagEditorWidgetPrivate::topSeriesHeaderRow()
{
  // Return number of rows if there are no series tables
  if (this->SeriesTagsHeaderRows.isEmpty())
    {
    return this->TagsTable->rowCount();
    }

  // Get minimum of the series tags header row indices
  return this->SeriesTagsHeaderRows.keys()[0];
}

//------------------------------------------------------------------------------
qSlicerDICOMExportable* qSlicerDICOMTagEditorWidgetPrivate::exportableForRowIndex(unsigned int row)
{
  // If patient or study tag, then it corresponds to no exportable
  if (row < this->topSeriesHeaderRow())
    {
    return nullptr;
    }

  unsigned int foundSeriesHeaderRowIndex = 0; // 0 is invalid value as it is the patient header
  QList<unsigned int> headerRowIndices = this->SeriesTagsHeaderRows.keys();
  if (headerRowIndices.contains(row))
    {
    // If row is a header, then return nullptr without logging error
    return nullptr;
    }

  // Iterate through the series header indices from the bottom up
  for (int headerRowIndexIndex = headerRowIndices.size()-1; headerRowIndexIndex >= 0 ; --headerRowIndexIndex)
    {
    // If edited row is greater than the current header index, but smaller than the
    // previous ones, and it is not a header, then we found the series section
    unsigned int currentHeaderRowIndex = headerRowIndices[headerRowIndexIndex];
    if (row > currentHeaderRowIndex)
      {
      foundSeriesHeaderRowIndex = currentHeaderRowIndex;
      break;
      }
    }
  if (foundSeriesHeaderRowIndex == 0)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find series section for row index " << row;
    return nullptr;
    }

  return this->SeriesTagsHeaderRows[foundSeriesHeaderRowIndex];
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidgetPrivate::insertTagsTableRow(unsigned int row)
{
  // Insert row in tags table
  this->TagsTable->insertRow(row);

  // Increase study tags header row index if row is inserted above it
  if (row <= this->StudyTagsHeaderRow)
    {
    this->StudyTagsHeaderRow++;
    }

  // Update those series header row indices above which the row is inserted
  QMap<unsigned int, qSlicerDICOMExportable*> updatedSeriesTagsHeaderRows;
  foreach(unsigned int seriesHeaderRow, this->SeriesTagsHeaderRows.keys())
    {
    // If inserted above the series header then increase it
    if (row <= seriesHeaderRow)
      {
      updatedSeriesTagsHeaderRows[seriesHeaderRow+1] = this->SeriesTagsHeaderRows[seriesHeaderRow];
      }
    // If below then keep the same
    else
      {
      updatedSeriesTagsHeaderRows[seriesHeaderRow] = this->SeriesTagsHeaderRows[seriesHeaderRow];
      }
    }
  // Replace series headers with updated one
  this->SeriesTagsHeaderRows = updatedSeriesTagsHeaderRows;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qSlicerDICOMTagEditorWidget::qSlicerDICOMTagEditorWidget(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new qSlicerDICOMTagEditorWidgetPrivate(*this))
{
  Q_D(qSlicerDICOMTagEditorWidget);
  d->setupUi(this);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerDICOMTagEditorWidget::~qSlicerDICOMTagEditorWidget() = default;

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerDICOMTagEditorWidget);
  d->Scene = scene;
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidget::clear()
{
  Q_D(qSlicerDICOMTagEditorWidget);

  // Reset study header
  d->StudyTagsHeaderRow = 0;
  // Clear series headers
  d->SeriesTagsHeaderRows.clear();

  // Empty tags table
  d->TagsTable->setRowCount(0);

  // Empty exportables list
  d->Exportables.clear();
}

//------------------------------------------------------------------------------
QString qSlicerDICOMTagEditorWidget::setExportables(QList<qSlicerDICOMExportable*> exportables)
{
  Q_D(qSlicerDICOMTagEditorWidget);

  if (!d->Scene)
    {
    QString error("Invalid MRML scene");
    qCritical() << Q_FUNC_INFO << ": " << error;
    return error;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(d->Scene);
  if (!shNode)
    {
    QString error("Invalid subject hierarchy");
    qCritical() << Q_FUNC_INFO << ": " << error;
    return error;
    }

  // Set exportable in class
  d->Exportables = exportables;
  // Clear argument exportables to prevent modifying the wrong list
  exportables.clear();

  // Check if the exportables are in the same study
  vtkIdType studyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  foreach (qSlicerDICOMExportable* exportable, d->Exportables)
    {
    vtkIdType seriesItemID = exportable->subjectHierarchyItemID();
    if (seriesItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      qCritical() << Q_FUNC_INFO << ": Exportable '" << exportable->name() << QString("' points to invalid item");
      continue;
      }

      vtkIdType parentItemID = shNode->GetItemParent(seriesItemID);
      if (studyItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        studyItemID = parentItemID;
        }
      if (studyItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        QString error("No study found");
        qCritical() << Q_FUNC_INFO << ": " << error;
        return error;
        }
      else if (studyItemID != parentItemID)
        {
        QString error("Exportables are not in the same study");
        qCritical() << Q_FUNC_INFO << ": " << error;
        return error;
        }
    }

  // Populate patient section

  // Get patient item
  vtkIdType patientItemID = shNode->GetItemParent(studyItemID);
  if (patientItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    QString error("No patient found");
    qCritical() << Q_FUNC_INFO << ": " << error;
    return error;
    }

  // Disconnect signal handling edited cells for the time of populating the table
  // At this line, because there must not be return statements between disconnect and connect (unless connect is called before return!)
  QObject::disconnect( d->TagsTable, SIGNAL(cellChanged(int,int)), this, SLOT(tagsTableCellChanged(int,int)) );

  // Initialize header row containers and add patient and study headers
  d->initializeTagsTable();

  // Add missing patient tags with empty values to patient item so that they are displayed in the table
  std::vector<std::string> patientItemAttributeNames = shNode->GetItemAttributeNames(patientItemID);
  std::vector<std::string> patientTagNames = vtkMRMLSubjectHierarchyConstants::GetDICOMPatientTagNames();
  for ( std::vector<std::string>::iterator patientTagIt = patientTagNames.begin();
    patientTagIt != patientTagNames.end(); ++patientTagIt )
    {
    std::string tagAttributeName = vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + (*patientTagIt);
    if (std::find(patientItemAttributeNames.begin(), patientItemAttributeNames.end(), tagAttributeName) == patientItemAttributeNames.end())
      {
      shNode->SetItemAttribute(patientItemID, tagAttributeName, "");
      }
    }
  // Get attribute names again in case some were missing
  patientItemAttributeNames = shNode->GetItemAttributeNames(patientItemID);
  // Create a row in table widget for each tag and populate exportables with patient tags
  // (all tags are acquired from the exportable on export)
  for (std::vector<std::string>::iterator it = patientItemAttributeNames.begin(); it != patientItemAttributeNames.end(); ++it)
    {
    std::string attributeName = (*it);
    if (attributeName.size() < vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size())
      {
      continue;
      }

    // Get attribute prefix from attribute name, and tag name (attribute name after prefix) and value
    std::string attributePrefix = attributeName.substr(0, vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size());
    QString tagName(attributeName.substr(vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size()).c_str());
    QString tagValue(shNode->GetItemAttribute(patientItemID, attributeName).c_str());

    // If DICOM tag attribute (i.e. has the prefix), then add to the table and exportable
    if (!attributePrefix.compare(vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix()))
      {
      // Add patient tag in a new row in the tags table
      unsigned int row = d->StudyTagsHeaderRow;
      d->insertTagsTableRow(row);
      d->TagsTable->setItem(row, 0, new QTableWidgetItem(tagName));
      d->TagsTable->setItem(row, 1, new QTableWidgetItem(tagValue));
      // Make sure tag name is not edited
      d->TagsTable->item(row, 0)->setFlags(Qt::ItemIsEnabled);

      // Also add it to the exportables (needed there for export)
      foreach (qSlicerDICOMExportable* exportable, d->Exportables)
        {
        exportable->setTag(tagName, tagValue);
        }
      }
    }


  // Populate study section (we already have the study item, no need to get it here)

  // Add missing study tags with empty values to study item so that they are displayed in the table
  std::vector<std::string> studyItemAttributeNames = shNode->GetItemAttributeNames(studyItemID);
  std::vector<std::string> studyTagNames = vtkMRMLSubjectHierarchyConstants::GetDICOMStudyTagNames();
  for (std::vector<std::string>::iterator studyTagIt = studyTagNames.begin(); studyTagIt != studyTagNames.end(); ++studyTagIt)
    {
    std::string tagAttributeName = vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + (*studyTagIt);
    if (std::find(studyItemAttributeNames.begin(), studyItemAttributeNames.end(), tagAttributeName) == studyItemAttributeNames.end())
      {
      shNode->SetItemAttribute(studyItemID, tagAttributeName, "");
      }
    }

  // Get attribute names again in case some were missing
  studyItemAttributeNames = shNode->GetItemAttributeNames(studyItemID);
  // Create a row in table widget for each tag and populate exportables with study tags
  // (all tags are acquired from the exportable on export)
  for (std::vector<std::string>::iterator it = studyItemAttributeNames.begin(); it != studyItemAttributeNames.end(); ++it)
    {
    std::string attributeName = (*it);
    if (attributeName.size() < vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size())
      {
      continue;
      }

    // Get attribute prefix from attribute name, and tag name (attribute name after prefix) and value
    std::string attributePrefix = attributeName.substr(0, vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size());
    QString tagName(attributeName.substr(vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix().size()).c_str());
    QString tagValue(shNode->GetItemAttribute(studyItemID, attributeName).c_str());
    // If DICOM tag attribute (i.e. has the prefix), then add to the table and exportable
    if (!attributePrefix.compare(vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix()))
      {
      // Add study tag in a new row in the study table
      unsigned int row = d->topSeriesHeaderRow();
      d->insertTagsTableRow(row);
      d->TagsTable->setItem(row, 0, new QTableWidgetItem(tagName));
      d->TagsTable->setItem(row, 1, new QTableWidgetItem(tagValue));
      // Make sure tag name is not edited
      d->TagsTable->item(row, 0)->setFlags(Qt::ItemIsEnabled);

      // Also add it to the exportables (needed there for export)
      foreach (qSlicerDICOMExportable* exportable, d->Exportables)
        {
        exportable->setTag(tagName, tagValue);
        }
      }
    }


  // Create series sections for each exportable
  foreach (qSlicerDICOMExportable* exportable, d->Exportables)
    {
    // Get exportable series item
    vtkIdType seriesItemID = exportable->subjectHierarchyItemID();

    // Add header row for new series
    unsigned int row = d->TagsTable->rowCount();
    d->insertTagsTableRow(row);
    QTableWidgetItem* seriesHeaderNameItem = new QTableWidgetItem(
      QString("'%1' series tag name").arg(shNode->GetItemName(seriesItemID).c_str()), 1 );
    seriesHeaderNameItem->setBackgroundColor(d->HeaderItemColor);
    seriesHeaderNameItem->setFont(d->HeaderItemFont);
    seriesHeaderNameItem->setFlags(Qt::ItemIsEnabled); // Non-editable
    d->TagsTable->setItem(row,0,seriesHeaderNameItem);
    QTableWidgetItem* seriesHeaderValueItem = new QTableWidgetItem("Value", 1);
    seriesHeaderValueItem->setBackgroundColor(d->HeaderItemColor);
    seriesHeaderValueItem->setFont(d->HeaderItemFont);
    seriesHeaderValueItem->setFlags(Qt::ItemIsEnabled); // Non-editable
    d->TagsTable->setItem(row,1,seriesHeaderValueItem);

    // Save series header row index and exportable
    d->SeriesTagsHeaderRows[row] = exportable;

    // Get series tags from exportable and populate table with them
    QMap<QString,QString> exportableTagsMap = exportable->tags();
    foreach (QString tagName, exportableTagsMap.keys())
      {
      // Only use series tags
      if ( vtkSlicerSubjectHierarchyModuleLogic::IsPatientTag(tagName.toUtf8().constData())
        || vtkSlicerSubjectHierarchyModuleLogic::IsStudyTag(tagName.toUtf8().constData()) )
        {
        continue;
        }

      // Add new row in series table for series tag
      row = d->TagsTable->rowCount();
      d->insertTagsTableRow(row);
      d->TagsTable->setItem(row, 0, new QTableWidgetItem(tagName));
      // Make sure tag name is not edited
      d->TagsTable->item(row, 0)->setFlags(Qt::ItemIsEnabled);

      // If series item contains tag then use that value
      std::string tagAttributeName = vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix()
        + std::string(tagName.toUtf8().constData());
      std::string tagAttributeValue = shNode->GetItemAttribute(seriesItemID, tagAttributeName);
      if (shNode->HasItemAttribute(seriesItemID, tagAttributeName))
        {
        exportable->setTag(tagName, tagAttributeValue.c_str());
        d->TagsTable->setItem(row, 1, new QTableWidgetItem(QString(tagAttributeValue.c_str())));
        }
      // Use default value from exportable otherwise
      else
        {
        d->TagsTable->setItem(row, 1, new QTableWidgetItem(exportableTagsMap[tagName]));
        }
      }
    }

  // Fit tag name column to contents
  //d->TagsTable->resizeColumnToContents(0); //TODO

  // Re-connect signal handling editing values
  QObject::connect( d->TagsTable, SIGNAL(cellChanged(int,int)), this, SLOT(tagsTableCellChanged(int,int)) );

  // Return empty error message indicating success
  return QString();
}

//------------------------------------------------------------------------------
QList<qSlicerDICOMExportable*> qSlicerDICOMTagEditorWidget::exportables()const
{
  Q_D(const qSlicerDICOMTagEditorWidget);
  return d->Exportables;
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidget::commitChangesToItems()
{
  Q_D(qSlicerDICOMTagEditorWidget);

  if (!d->Scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(d->Scene);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  std::string dicomAttributePrefix = vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix();
  vtkIdType studyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

  // Commit changes to series
  for (int row=d->topSeriesHeaderRow(); row<d->TagsTable->rowCount(); ++row)
    {
    qSlicerDICOMExportable* exportable = d->exportableForRowIndex(row);
    if (!exportable) // Header row
      {
      continue;
      }

    // Get subject hierarchy series item from exportable
    vtkIdType seriesItemID = exportable->subjectHierarchyItemID();

    // Save study item if not set yet
    if (studyItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      studyItemID = shNode->GetItemParent(seriesItemID);
      }

    QString tagName = d->TagsTable->item(row, 0)->text();
    QString tagAttributeName = QString(dicomAttributePrefix.c_str()) + tagName;
    QString tagValue = d->TagsTable->item(row, 1)->text();
    shNode->SetItemAttribute(seriesItemID, tagAttributeName.toUtf8().constData(), tagValue.toUtf8().constData());
    }

  // Commit changes to study
  if (studyItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get study item";
    return;
    }
  // Write tags from study section to study item
  for (unsigned int row=d->StudyTagsHeaderRow+1; row<d->topSeriesHeaderRow(); ++row)
    {
    QString tagName = d->TagsTable->item(row, 0)->text();
    QString tagAttributeName = QString(dicomAttributePrefix.c_str()) + tagName;
    QString tagValue = d->TagsTable->item(row, 1)->text();
    shNode->SetItemAttribute(studyItemID, tagAttributeName.toUtf8().constData(), tagValue.toUtf8().constData());
    }

  // Commit changes to patient
  vtkIdType patientItemID = shNode->GetItemParent(studyItemID);
  if (patientItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get patient item";
    return;
    }
  // Write tags from patient table to patient item
  for (unsigned int row=1; row<d->StudyTagsHeaderRow; ++row)
    {
    QString tagName = d->TagsTable->item(row, 0)->text();
    QString tagAttributeName = QString(dicomAttributePrefix.c_str()) + tagName;
    QString tagValue = d->TagsTable->item(row, 1)->text();
    shNode->SetItemAttribute(patientItemID, tagAttributeName.toUtf8().constData(), tagValue.toUtf8().constData());
    }
}

//------------------------------------------------------------------------------
void qSlicerDICOMTagEditorWidget::tagsTableCellChanged(int row, int column)
{
  Q_D(qSlicerDICOMTagEditorWidget);
  if (column != 1)
    {
    // Only the value column is editable
    return;
    }

  // If patient or study tag (need to set value to each exportable)
  if (row < (int)d->topSeriesHeaderRow())
    {
    // Set new tag value in each exportable
    foreach (qSlicerDICOMExportable* exportable, d->Exportables)
      {
      exportable->setTag(d->TagsTable->item(row,0)->text(), d->TagsTable->item(row,1)->text());
      }
    }
  // Series tag edited. Find exportable and set value in only that one
  else
    {
    qSlicerDICOMExportable* exportable = d->exportableForRowIndex(row);
    if (!exportable)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to get exportable for row index " << row;
      return;
      }

    // Set tag in exportable
    exportable->setTag(d->TagsTable->item(row,0)->text(), d->TagsTable->item(row,1)->text());
    }

  // Emit signal outside so that changes can be used immediately if needed
  emit tagEdited();
}
