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

// Segmentations includes
#include "qMRMLSegmentationConversionParametersWidget.h"
#include "ui_qMRMLSegmentationConversionParametersWidget.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkSegmentation.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"

// MRMLWidgets includes
#include "qMRMLNodeComboBox.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>
#include <QVariant>
#include <QPushButton>
#include <QMessageBox>

// --------------------------------------------------------------------------
class qMRMLSegmentationConversionParametersWidgetPrivate: public Ui_qMRMLSegmentationConversionParametersWidget
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationConversionParametersWidget);
protected:
  qMRMLSegmentationConversionParametersWidget* const q_ptr;
public:
  qMRMLSegmentationConversionParametersWidgetPrivate(qMRMLSegmentationConversionParametersWidget& object);
  void init();

  /// Return the column index for a given string in the paths table, -1 if not a valid header
  int pathsColumnIndex(QString label);

  /// Return the column index for a given string in the parameters table, -1 if not a valid header
  int parametersColumnIndex(QString label);

public:
  /// Segmentation MRML node containing shown segments
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

  /// Target representation name
  QString TargetRepresentationName;

  /// Possible conversion paths for the current segmentation and target representation
  std::vector<vtkSegmentationConverter::ConversionPathType> PossiblePaths;

private:
  QStringList PathsColumnLabels;
  QStringList ParametersColumnLabels;
};

// --------------------------------------------------------------------------
qMRMLSegmentationConversionParametersWidgetPrivate::qMRMLSegmentationConversionParametersWidgetPrivate(qMRMLSegmentationConversionParametersWidget& object)
  : q_ptr(&object)
{
  this->SegmentationNode = NULL;
}

// --------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidgetPrivate::init()
{
  Q_Q(qMRMLSegmentationConversionParametersWidget);
  this->setupUi(q);

  // Make connections
  QObject::connect(this->PathsTable, SIGNAL(itemSelectionChanged()),
                   q, SLOT(populateParametersTable()));
  QObject::connect(this->ParametersTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                   q, SLOT(onParameterChanged(QTableWidgetItem*)));
  QObject::connect(this->pushButton_Convert, SIGNAL(clicked()),
                   q, SLOT(applyConversion()));

  // Set up initial look of the tables
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  this->PathsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
  this->PathsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
  this->PathsTable->horizontalHeader()->setStretchLastSection(1);
  this->PathsTable->setSelectionMode(QAbstractItemView::SingleSelection);
  this->PathsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->PathsColumnLabels << "Cost" << "Path";
  this->PathsTable->setColumnCount(this->PathsColumnLabels.size());

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  this->ParametersTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
  this->ParametersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
  this->ParametersTable->horizontalHeader()->setStretchLastSection(1);
  this->ParametersColumnLabels << "Name" << "Value";
  this->ParametersTable->setColumnCount(this->ParametersColumnLabels.size());
  this->ParametersTable->setSelectionMode(QAbstractItemView::NoSelection);
}

//-----------------------------------------------------------------------------
int qMRMLSegmentationConversionParametersWidgetPrivate::pathsColumnIndex(QString label)
{
  return this->PathsColumnLabels.indexOf(label);
}

//-----------------------------------------------------------------------------
int qMRMLSegmentationConversionParametersWidgetPrivate::parametersColumnIndex(QString label)
{
  return this->ParametersColumnLabels.indexOf(label);
}

// --------------------------------------------------------------------------
// qMRMLSegmentationConversionParametersWidget methods

// --------------------------------------------------------------------------
qMRMLSegmentationConversionParametersWidget::qMRMLSegmentationConversionParametersWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLSegmentationConversionParametersWidgetPrivate(*this))
{
  Q_D(qMRMLSegmentationConversionParametersWidget);
  d->init();
  this->populatePathsTable();
  this->populateParametersTable();
}

// --------------------------------------------------------------------------
qMRMLSegmentationConversionParametersWidget::~qMRMLSegmentationConversionParametersWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::setSegmentationNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  d->SegmentationNode = segmentationNode;
  if (segmentationNode)
    {
    d->label_SegmentationName->setText(QString(segmentationNode->GetName()));
    }
  else
    {
    d->label_SegmentationName->setText("Invalid segmentation");
    }

  this->populatePathsTable();
  this->populateParametersTable();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentationConversionParametersWidget::segmentationNode()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::setTargetRepresentationName(QString representationName)
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  d->TargetRepresentationName = representationName;

  if (!d->TargetRepresentationName.isEmpty())
    {
    d->label_RepresentationName->setText(representationName);
    }
  else
    {
    d->label_RepresentationName->setText("Invalid representation");
    }

  this->populatePathsTable();
  this->populateParametersTable();
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationConversionParametersWidget::targetRepresentationName()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  return d->TargetRepresentationName;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::populatePathsTable()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  // Block signals so that connected function is not called when populating
  d->PathsTable->blockSignals(true);

  d->PathsTable->clearContents();
  d->PossiblePaths.clear();

  if (!d->SegmentationNode || d->TargetRepresentationName.isEmpty())
    {
    d->PathsTable->setRowCount(0);
    d->PathsTable->blockSignals(false);
    return;
    }
  if (d->TargetRepresentationName.isEmpty())
    {
    d->PathsTable->setRowCount(0);
    d->PathsTable->blockSignals(false);
    return;
    }

  // Get possible paths
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  vtkSegmentationConverter::ConversionPathAndCostListType pathsCosts;
  segmentation->GetPossibleConversions(d->TargetRepresentationName.toLatin1().constData(), pathsCosts);

  if (pathsCosts.empty())
  {
    d->PathsTable->setRowCount(1);
    QTableWidgetItem* noPathsItem = new QTableWidgetItem("No path available!");
    QFont boldFont;
    boldFont.setWeight(QFont::Bold);
    noPathsItem->setFont(boldFont);
    noPathsItem->setFlags(noPathsItem->flags() & ~Qt::ItemIsEditable);
    d->PathsTable->setItem(0, d->pathsColumnIndex("Path"), noPathsItem);

    d->PathsTable->blockSignals(false);
    return;
  }

  vtkSegmentationConverter::ConversionPathAndCostListType::iterator pathIt;
  d->PathsTable->setRowCount(pathsCosts.size());
  int row = 0;
  for (pathIt=pathsCosts.begin(); pathIt!=pathsCosts.end(); ++pathIt, ++row)
    {
    // Path cost
    QString costString = QString::number(pathIt->second);
    QTableWidgetItem* costItem = new QTableWidgetItem(costString);
    costItem->setFlags(costItem->flags() & ~Qt::ItemIsEditable);
    costItem->setData(Qt::UserRole, QVariant(row));
    d->PathsTable->setItem(row, d->pathsColumnIndex("Cost"), costItem);

    // Path
    vtkSegmentationConverter::ConversionPathType path = pathIt->first;
    d->PossiblePaths.push_back(path);
    vtkSegmentationConverterRule* firstRule = (*path.begin());
    QString pathString(firstRule->GetSourceRepresentationName()); // Add source (master) representation

    vtkSegmentationConverter::ConversionPathType::iterator ruleIt;
    for (ruleIt=path.begin(); ruleIt!=path.end(); ++ruleIt)
      {
      vtkSegmentationConverterRule* currentRule = (*ruleIt);
      pathString.append(" -> ");
      pathString.append(currentRule->GetTargetRepresentationName());
      }
    QTableWidgetItem* pathItem = new QTableWidgetItem(pathString);
    pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
    pathItem->setData(Qt::UserRole, QVariant(row));
    d->PathsTable->setItem(row, d->pathsColumnIndex("Path"), pathItem);
    }

  // Unblock signals
  d->PathsTable->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::populateParametersTable()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  // Block signals so that connected function is not called when populating
  d->ParametersTable->blockSignals(true);

  d->ParametersTable->clearContents();

  if (!d->SegmentationNode)
    {
    d->ParametersTable->setRowCount(0);
    d->ParametersTable->blockSignals(false);
    return;
    }

  vtkSegmentationConverter::ConversionPathType selectedPath = this->selectedPath();
  if (selectedPath.empty())
  {
    d->ParametersTable->setRowCount(1);
    QTableWidgetItem* noParametersItem = new QTableWidgetItem("No path selected");
    QFont boldFont;
    boldFont.setWeight(QFont::Bold);
    noParametersItem->setFont(boldFont);
    noParametersItem->setFlags(noParametersItem->flags() & ~Qt::ItemIsEditable);
    d->ParametersTable->setItem(0, d->parametersColumnIndex("Value"), noParametersItem);

    d->ParametersTable->blockSignals(false);
    return;
  }

  // Get parameters for selected path
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  vtkSegmentationConverterRule::ConversionParameterListType parameters;
  segmentation->GetConversionParametersForPath(parameters, selectedPath);

  if (parameters.empty())
  {
    d->ParametersTable->setRowCount(1);
    QTableWidgetItem* noParametersItem = new QTableWidgetItem("No parameters for selected path");
    QFont boldFont;
    boldFont.setWeight(QFont::Bold);
    noParametersItem->setFont(boldFont);
    noParametersItem->setFlags(noParametersItem->flags() & ~Qt::ItemIsEditable);
    d->ParametersTable->setItem(0, d->parametersColumnIndex("Value"), noParametersItem);

    d->ParametersTable->blockSignals(false);
    return;
  }

  vtkSegmentationConverterRule::ConversionParameterListType::iterator paramIt;
  d->ParametersTable->setRowCount(parameters.size());
  int row = 0;
  for (paramIt=parameters.begin(); paramIt!=parameters.end(); ++paramIt, ++row)
  {
    // Parameter name
    QString parameterName(paramIt->first.c_str());
    QString parameterDescription(paramIt->second.second.c_str());
    QTableWidgetItem* nameItem = new QTableWidgetItem(parameterName);
    nameItem->setToolTip(parameterDescription);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    d->ParametersTable->setItem(row, d->parametersColumnIndex("Name"), nameItem);

    // Parameter value
    QString parameterValue(paramIt->second.first.c_str());

    // Special case: reference image geometry
    if (!parameterName.compare(vtkSegmentationConverter::GetReferenceImageGeometryParameterName().c_str()))
    {
      QWidget* geometryWidget = new QWidget(d->ParametersTable);
      QHBoxLayout* geometryLayout = new QHBoxLayout(geometryWidget);
      geometryLayout->setContentsMargins(2, 2, 2, 0);
      geometryLayout->setSpacing(4);

      QLabel* textValueLabel = new QLabel(parameterValue, geometryWidget);
      geometryLayout->addWidget(textValueLabel);

      QPushButton* setGeometryFromVolumeButton = new QPushButton(tr("Set geometry from volume"), geometryWidget);
      setGeometryFromVolumeButton->setFixedWidth(160);
      QObject::connect(setGeometryFromVolumeButton, SIGNAL(clicked()),
                       this, SLOT(onSetReferenceImageGeometryFromVolumeClicked()));
      geometryLayout->addWidget(setGeometryFromVolumeButton);

      //QTableWidgetItem* geometryItem = new QTableWidgetItem();
      //geometryItem->setFlags(geometryItem->flags() & ~Qt::ItemIsEditable);
      geometryWidget->setToolTip(parameterDescription);
      d->ParametersTable->setCellWidget(row, d->parametersColumnIndex("Value"), geometryWidget);
    }
    // Generic parameter
    else
    {
      QTableWidgetItem* valueItem = new QTableWidgetItem(parameterValue);
      valueItem->setToolTip(parameterDescription);
      d->ParametersTable->setItem(row, d->parametersColumnIndex("Value"), valueItem);
    }
  }

  // Unblock signals
  d->ParametersTable->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::applyConversion()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (!d->SegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: segmentation node is invalid";
    return;
    }

  // Perform conversion using selected path and chosen conversion parameters
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  if (!d->SegmentationNode->GetSegmentation()->CreateRepresentation(
        this->selectedPath(), this->conversionParameters()))
  {
    QString message = QString("Failed to convert %1 to %2!").arg(d->SegmentationNode->GetName()).arg(d->TargetRepresentationName);
    QMessageBox::warning(NULL, tr("Conversion failed"), message);
  }
  QApplication::restoreOverrideCursor();

  emit conversionDone();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::onParameterChanged(QTableWidgetItem* changedItem)
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (!d->SegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: segmentation node is invalid";
    return;
    }

  // Get name item (safe to assume that changed item is the value, as it is the only editable one)
  int row = changedItem->row();
  QTableWidgetItem* nameItem = d->ParametersTable->item(row, d->parametersColumnIndex("Name"));

  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  segmentation->SetConversionParameter(nameItem->text().toLatin1().constData(), changedItem->text().toLatin1().constData());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::onSetReferenceImageGeometryFromVolumeClicked()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (!d->SegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: segmentation node is invalid";
    return;
    }

  QDialog* getGeometryFromVolumeDialog = new QDialog(NULL, Qt::Dialog);
  QVBoxLayout* geometryLayout = new QVBoxLayout(getGeometryFromVolumeDialog);
  geometryLayout->setContentsMargins(6, 6, 6, 6);
  geometryLayout->setSpacing(12);

  QLabel* label = new QLabel(tr("Please select a volume node that defines the geometry of the converted volume representation.\n(If you choose None, then the parameter will remain the same)"), getGeometryFromVolumeDialog);
  label->setWordWrap(true);
  geometryLayout->addWidget(label);

  qMRMLNodeComboBox* volumeCombobox = new qMRMLNodeComboBox(getGeometryFromVolumeDialog);
  volumeCombobox->setNodeTypes( QStringList() << "vtkMRMLScalarVolumeNode" << "vtkMRMLLabelMapVolumeNode" );
  volumeCombobox->setShowChildNodeTypes(false);
  volumeCombobox->setNoneEnabled(true);
  volumeCombobox->setMRMLScene(d->SegmentationNode->GetScene());
  geometryLayout->addWidget(volumeCombobox);

  QPushButton* applyButton = new QPushButton(tr("Apply"), getGeometryFromVolumeDialog);
  applyButton->setFixedWidth(96);
  geometryLayout->addWidget(applyButton);
  geometryLayout->setAlignment(applyButton, Qt::AlignCenter);

  // Connect conversion done event to dialog close
  QObject::connect(applyButton, SIGNAL(clicked()),
                   getGeometryFromVolumeDialog, SLOT(accept()));

  // Show dialog
  getGeometryFromVolumeDialog->exec();

  // Set geometry from selected volume
  this->setReferenceImageGeometryParameterFromVolumeNode(volumeCombobox->currentNode());

  // Delete dialog when done
  delete getGeometryFromVolumeDialog;

  // Refresh parameters table
  this->populateParametersTable();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationConversionParametersWidget::setReferenceImageGeometryParameterFromVolumeNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (!d->SegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: segmentation node is invalid";
    return;
    }

  // If selection is 'None', then do not change reference image geometry parameter
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (!volumeNode || !volumeNode->GetImageData())
  {
    return;
  }

  d->SegmentationNode->SetReferenceImageGeometryParameterFromVolumeNode(volumeNode);
}

//-----------------------------------------------------------------------------
vtkSegmentationConverter::ConversionPathType qMRMLSegmentationConversionParametersWidget::selectedPath()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (d->PossiblePaths.empty())
  {
    vtkSegmentationConverter::ConversionPathType emptyPath;
    return emptyPath;
  }

  QList<QTableWidgetItem*> selectedItems = d->PathsTable->selectedItems();
  if (selectedItems.isEmpty())
  {
    vtkSegmentationConverter::ConversionPathType emptyPath;
    return emptyPath;
  }

  QTableWidgetItem* firstItem = (*selectedItems.begin());
  int selectedRow = firstItem->data(Qt::UserRole).toInt();

  return d->PossiblePaths[selectedRow];
}

//-----------------------------------------------------------------------------
vtkSegmentationConverterRule::ConversionParameterListType qMRMLSegmentationConversionParametersWidget::conversionParameters()
{
  Q_D(qMRMLSegmentationConversionParametersWidget);

  if (!d->SegmentationNode)
    {
    vtkSegmentationConverterRule::ConversionParameterListType emptyParameters;
    return emptyParameters;
    }

  vtkSegmentationConverter::ConversionPathType selectedPath = this->selectedPath();
  if (selectedPath.empty())
  {
    vtkSegmentationConverterRule::ConversionParameterListType emptyParameters;
    return emptyParameters;
  }

  // Get parameters for selected path
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  vtkSegmentationConverterRule::ConversionParameterListType parameters;
  segmentation->GetConversionParametersForPath(parameters, selectedPath);
  return parameters;
}
