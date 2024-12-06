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
#include "qSlicerDICOMExportDialog.h"
#include "ui_qSlicerDICOMExportDialog.h"

#include "qSlicerDICOMExportable.h"

// Subject Hierarchy Widgets includes
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSubjectHierarchyModel.h"

// MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"

// Qt includes
#include <QDialog>
#include <QObject>
#include <QDebug>
#include <QItemSelection>
#include <QListWidgetItem>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QUiLoader>

// Printer
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QPdfWriter>
#include <qfontdatabase.h>

// PythonQt includes
#include "PythonQt.h"

// MRML includes
#include <vtkImageData.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// SlicerApp includes
#include <qSlicerApplication.h>
#include <qSlicerCorePythonManager.h>
//#include <qSlicerAbstractCoreModule.h>

// CTK includes
// XXX Avoid  warning: "HAVE_XXXX" redefined
#undef HAVE_STAT
#undef HAVE_FTIME
#undef HAVE_GETPID
#undef HAVE_IO_H
#undef HAVE_STRERROR
#undef HAVE_SYS_UTIME_H
#undef HAVE_TEMPNAM
#undef HAVE_TMPNAM
#undef HAVE_LONG_LONG
// XXX Fix windows build error
#undef HAVE_INT64_T
#include "ctkDICOMDatabase.h"
#include "ctkDICOMIndexer.h"

//-----------------------------------------------------------------------------
class qSlicerDICOMExportDialogPrivate : public Ui_qSlicerDICOMExportDialog, public QDialog
{
  Q_DECLARE_PUBLIC(qSlicerDICOMExportDialog);
protected:
  qSlicerDICOMExportDialog* const q_ptr;
public:
  qSlicerDICOMExportDialogPrivate(qSlicerDICOMExportDialog& object);
  ~qSlicerDICOMExportDialogPrivate() override;
public:
  void init();
private:
  vtkMRMLScene* Scene;
  vtkIdType ItemToSelect;
  qSlicerDICOMExportable* SelectedExportable;
};

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialogPrivate::qSlicerDICOMExportDialogPrivate(qSlicerDICOMExportDialog& object)
  : q_ptr(&object)
  , Scene(nullptr)
  , ItemToSelect(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , SelectedExportable(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerDICOMExportDialogPrivate::~qSlicerDICOMExportDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialogPrivate::init()
{
  Q_Q(qSlicerDICOMExportDialog);

  // Set up tree view
  qMRMLSubjectHierarchyModel* sceneModel = (qMRMLSubjectHierarchyModel*)this->SubjectHierarchyTreeView->model();
  this->SubjectHierarchyTreeView->setMRMLScene(this->Scene);
  this->SubjectHierarchyTreeView->expandToDepth(4);
  this->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->idColumn());
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->visibilityColumn());
  this->SubjectHierarchyTreeView->hideColumn(sceneModel->transformColumn());

  // Empty error label (was not empty to indicate its purpose in designer)
  this->ErrorLabel->setText(QString());

  // Make connections
  connect(this->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    q, SLOT(onCurrentItemChanged(vtkIdType)));
  connect(this->ExportablesListWidget, SIGNAL(currentRowChanged(int)),
    q, SLOT(onExportableSelectedAtRow(int)));
  connect(this->DICOMTagEditorWidget, SIGNAL(tagEdited()),
    q, SLOT(onTagEdited()));
  connect(this->ExportButton, SIGNAL(clicked()),
    q, SLOT(onExport()));
  connect(this->PrintButton, SIGNAL(clicked()),
      q, SLOT(onPrint()));
  connect(this->SaveTagsCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(onSaveTagsCheckBoxToggled(bool)) );
  connect(this->ExportToFolderCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(onExportToFolderCheckBoxToggled(bool)) );
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
qSlicerDICOMExportDialog::~qSlicerDICOMExportDialog() = default;

//-----------------------------------------------------------------------------
bool qSlicerDICOMExportDialog::exec(vtkIdType itemToSelect/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)
{
  Q_D(qSlicerDICOMExportDialog);

  // Initialize dialog
  d->init();

  // Make selection if requested
  d->ItemToSelect = itemToSelect;
  QTimer::singleShot(0, this, SLOT( makeDialogSelections() ) );

  // Show dialog
  if (d->exec() != QDialog::Accepted)
  {
    return false;
  }

  // Perform actions after clean exit
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerDICOMExportDialog);
  d->Scene = scene;
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::makeDialogSelections()
{
  Q_D(qSlicerDICOMExportDialog);

  // Select item marked to select
  if (d->ItemToSelect)
  {
    d->SubjectHierarchyTreeView->setCurrentItem(d->ItemToSelect);
  }

  // Set checkbox state from application settings
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("DICOM/ExportToFolder"))
  {
    bool exportToFolder = settings->value("DICOM/ExportToFolder").toBool();
    d->ExportToFolderCheckBox->setChecked(exportToFolder);
  }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onCurrentItemChanged(vtkIdType itemID)
{
  Q_UNUSED(itemID);
  Q_D(qSlicerDICOMExportDialog);

  // Clear error label
  d->ErrorLabel->setText(QString());

  // Get exportables from DICOM plugins
  this->examineSelectedItem();
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::examineSelectedItem()
{
  Q_D(qSlicerDICOMExportDialog);

  // Get current item (single-selection)
  vtkIdType currentItemID = d->SubjectHierarchyTreeView->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to get current subject hierarchy item";
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get subject hierarchy node";
    return;
  }

  // Clear exportables list
  d->ExportablesListWidget->clear();
  d->DICOMTagEditorWidget->clear();

  // Get child series items if selected node is study
  QList<vtkIdType> selectedSeriesItemIDs;
  if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy()))
  {
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(currentItemID, childItemIDs);
    std::vector<vtkIdType>::iterator childIt;
    for (childIt = childItemIDs.begin(); childIt != childItemIDs.end(); ++childIt)
    {
      selectedSeriesItemIDs.append(*childIt);
    }
  }
  else if (shNode->GetItemDataNode(currentItemID))
  {
    selectedSeriesItemIDs.append(currentItemID);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Can only export data node or study item";
    return;
  }

  // Get exportables from DICOM plugins for selection
  QMultiMap<QString,qSlicerDICOMExportable*> exportablesByPlugin;
  foreach (vtkIdType selectedSeriesItemID, selectedSeriesItemIDs)
  {
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    // A plugin should return one exportable for one series, but nevertheless
    // a list is returned for convenient concatenation (without type check etc.)
    context.evalScript(QString(
      "exportables = []\n"
      "for pluginClass in slicer.modules.dicomPlugins:\n"
      "  plugin = slicer.modules.dicomPlugins[pluginClass]()\n"
      "  try:\n"
      "    exportables.extend(plugin.examineForExport(%1))\n"
      "  except Exception as e:\n"
      "    logging.error(f'Failed to get exportables from plugin {type(plugin).__name__}')\n"
      "    import traceback\n"
      "    traceback.print_exc()\n")
      .arg(selectedSeriesItemID) );

    // Extract resulting exportables from python
    QVariantList exportablesVariantList = context.getVariable("exportables").toList();

    // Group exportables by provider plugin
    foreach(QVariant exportableVariant, exportablesVariantList)
    {
      qSlicerDICOMExportable* exportable = qobject_cast<qSlicerDICOMExportable*>(
        exportableVariant.value<QObject*>() );
      if (!exportable)
      {
        qCritical() << Q_FUNC_INFO << ": Invalid exportable returned by DICOM plugin for " << shNode->GetItemName(currentItemID).c_str();
        continue;
      }
      exportable->setParent(this); // Take ownership to prevent destruction
      QString plugin = exportable->pluginClass();
      exportablesByPlugin.insert(plugin, exportable);
    }
  }
  // Map the grouped exportables by confidence values so that the highest confidence is on top
  QMultiMap<double,QList<qSlicerDICOMExportable*> > exportablesByConfidence;
  foreach(const QString& plugin, exportablesByPlugin.uniqueKeys())
  {
    // Geometric mean to emphasize larger values
    double meanConfidenceForPlugin = 0.0;
    QList<qSlicerDICOMExportable*> exportablesForPlugin = exportablesByPlugin.values(plugin);
    foreach (qSlicerDICOMExportable* exportable, exportablesForPlugin)
    {
      meanConfidenceForPlugin += exportable->confidence();
    }
    meanConfidenceForPlugin /= exportablesForPlugin.count();

    // Add exportable to map with confidence as key. Confidence value is subtracted
    // from 1 so that iterating through the map automatically orders the exportables.
    exportablesByConfidence.insert(1.0 - meanConfidenceForPlugin, exportablesForPlugin);
  }

  // Populate the exportables list widget
  foreach (double inverseConfidence, exportablesByConfidence.uniqueKeys())
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
      qCritical() << Q_FUNC_INFO << ": " << errorMessage;
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
void qSlicerDICOMExportDialog::onTagEdited()
{
  Q_D(qSlicerDICOMExportDialog);

  // Commit changes to exported series item(s) and their study and patient parents
  if (d->SaveTagsCheckBox->isChecked())
  {
    d->DICOMTagEditorWidget->commitChangesToItems();
  }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onExport()
{
  Q_D(qSlicerDICOMExportDialog);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  if (d->ExportToFolderCheckBox->isChecked())
  {
    d->PathLineEdit_OutputFolder->addCurrentPathToHistory();
  }

  // Clear error label
  d->ErrorLabel->setText(QString());

  // Get output directory
  QDir outputFolder(d->PathLineEdit_OutputFolder->currentPath());

  // Determine whether output directory is a Slicer DICOM database
  bool exportToDatabase = !d->ExportToFolderCheckBox->isChecked();

  // Set output folder to a temporary location if the output directory is a DICOM database
  if (exportToDatabase)
  {
    // Save to temporary folder and store files in database directory when adding
    outputFolder.setPath(qSlicerApplication::application()->temporaryPath());

    // Force using en-US locale, otherwise for example on a computer with
    // Egyptian Arabic (ar-EG) locale, Arabic numerals may be used.
    QLocale enUsLocale = QLocale(QLocale::English, QLocale::UnitedStates);
    QString tempSubDirName = QString("DICOMExportTemp_%1").arg(enUsLocale.toString(QDateTime::currentDateTime(), "yyyyMMdd_hhmmss"));

    outputFolder.mkdir(tempSubDirName);
    outputFolder.cd(tempSubDirName);
  }

  // Call export function based on radio button choice
  bool exportSuccess = this->exportSeries(outputFolder);

  if (exportToDatabase)
  {
    // Add exported files to DICOM database
    if (exportSuccess)
    {
      ctkDICOMDatabase* dicomDatabase = qSlicerApplication::application()->dicomDatabase();
      if (!dicomDatabase)
      {
        d->ErrorLabel->setText("No DICOM database is set, so the data (that was successfully exported) cannot be imported back");
        return;
      }
      ctkDICOMIndexer indexer;
      indexer.setDatabase(dicomDatabase);
      indexer.addDirectory(outputFolder.absolutePath(), true);
    }
    // Remove temporary DICOM folder if exported to the DICOM database folder
    foreach(QString file, outputFolder.entryList())
    {
      outputFolder.remove(file);
    }
    QString tempSubDirName = outputFolder.dirName();
    outputFolder.cdUp();
    outputFolder.rmdir(tempSubDirName);
  }

  QApplication::restoreOverrideCursor();

  if (exportSuccess)
  {
    // Show result popup
    QString message = tr("DICOM dataset successfully exported to ");
    if (exportToDatabase)
    {
      message += tr("the DICOM database");
    }
    else
    {
      message += outputFolder.absolutePath();
    }
    QMessageBox::information(nullptr, tr("Export successful"), message);

    if (exportToDatabase)
    {
      // Show and update DICOM browser if it was requested
      this->showUpdatedDICOMBrowser();
    }

    // Close the export dialog after successful export
    d->done(0);
  }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onPrint()
{
    Q_D(qSlicerDICOMExportDialog);

    // Read all image for vtkMRMLNode
    QList<QImage> imageList;
    vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
    vtkMRMLNode* node = shNode->GetItemDataNode(d->SubjectHierarchyTreeView->currentItem());
    //vtkMRMLNode* node = d->Scene->GetFirstNodeByClass("vtkMRMLScalarVolumeNode");
    if (node)
    {
        vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
        if (volumeNode)
        {
            vtkImageData* imageData = volumeNode->GetImageData();
            if (imageData)
            {
                double range[2];
                imageData->GetScalarRange(range);
                int scalarType = imageData->GetScalarType();
                // qDebug() << "Scalar range: [" << range[0] << ", " << range[1] << "]" << scalarType;
                // TODO:Temporary!, dicom piexl data have int and short? no uint
                if (scalarType == VTK_INT) {
                    int* scalarPointer = static_cast<int*>(imageData->GetScalarPointer());
                    int extent[6];
                    imageData->GetExtent(extent);
                    int width = extent[1] - extent[0] + 1;
                    int height = extent[3] - extent[2] + 1;
                    int depth = extent[5] - extent[4] + 1;
                    // qDebug() << width << height << depth;
                    QImage qImage(width, height, QImage::Format_Grayscale8);
                    for (int z = 0; z < depth; ++z) {
                        QImage qImage(width, height, QImage::Format_Grayscale8);
                        for (int y = 0; y < height; ++y)
                        {
                            for (int x = 0; x < width; ++x)
                            {
                                int value = scalarPointer[z * height * width + y * width + x];
                                int normalizedValue = static_cast<int>((value - range[0]) * 255 / (range[1] - range[0]));
                                qImage.bits()[y * qImage.bytesPerLine() + x] = static_cast<uchar>(normalizedValue);
                            }
                        }
                        imageList.append(qImage);
                    }
                }
                else if (scalarType == VTK_SHORT) {
                    short* scalarPointer = static_cast<short*>(imageData->GetScalarPointer());
                    int extent[6];
                    imageData->GetExtent(extent);
                    int width = extent[1] - extent[0] + 1;
                    int height = extent[3] - extent[2] + 1;
                    int depth = extent[5] - extent[4] + 1;
                    // qDebug() << width << height << depth;
                    for (int z = 0; z < depth; ++z) {
                        QImage qImage(width, height, QImage::Format_Grayscale8);
                        for (int y = 0; y < height; ++y)
                        {
                            for (int x = 0; x < width; ++x)
                            {
                                short value = scalarPointer[z * height * width + y * width + x];
                                short normalizedValue = static_cast<short>((value - range[0]) * 255 / (range[1] - range[0]));
                                qImage.bits()[y * qImage.bytesPerLine() + x] = static_cast<uchar>(normalizedValue);
                            }
                        }
                        imageList.append(qImage);
                    }
                }
                else if(scalarType == VTK_FLOAT){
                    // CTLiver 数据重新get到了这个问题，可能需要总结一下imageData数据类型
                    float* scalarPointer = static_cast<float*>(imageData->GetScalarPointer());
                    int extent[6];
                    imageData->GetExtent(extent);
                    int width = extent[1] - extent[0] + 1;
                    int height = extent[3] - extent[2] + 1;
                    int depth = extent[5] - extent[4] + 1;
                    // qDebug() << width << height << depth;
                    QImage qImage(width, height, QImage::Format_Grayscale8);
                    for (int z = 0; z < depth; ++z) {
                        QImage qImage(width, height, QImage::Format_Grayscale8);
                        for (int y = 0; y < height; ++y)
                        {
                            for (int x = 0; x < width; ++x)
                            {
                                float value = scalarPointer[z * height * width + y * width + x];
                                float normalizedValue = static_cast<float>((value - range[0]) * 255 / (range[1] - range[0]));
                                qImage.bits()[y * qImage.bytesPerLine() + x] = static_cast<uchar>(normalizedValue);
                            }
                        }
                        imageList.append(qImage);
                    }

                }
                else {
                    qDebug() << "Cant Print";
                    return;
                }
            }
        }
    }
    int size_list= imageList.size();
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    QPrintPreviewDialog previewDialog(&printer);
    /*qreal pixelRatio = qApp->devicePixelRatio();*/
    previewDialog.resize(printer.width(), printer.height());
    previewDialog.setWindowTitle("Print");

    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this, [&,imageList](QPrinter* this_printer) {
        QPainter painter(this_printer);
        QUiLoader loader;
        QFile file(":Ui/Print.ui");
        QWidget* print_widget = loader.load(&file);

        QMap<QString, QString> m_qmap = d->DICOMTagEditorWidget->exportables()[0]->tags();

        print_widget->resize(this_printer->width(), this_printer->height());
        file.close();

        //double pixelRatio = qApp->devicePixelRatio();
        //qDebug() << "-------->" << pixelRatio<<"--->"<< this_printer->width()<<this_printer->height();
        //print_widget->resize(this_printer->width(), this_printer->height());
        //widget->render(&painter);
        //QPixmap pixmap(this_printer->width(), this_printer->height());
        //print_widget->render(&pixmap);
        //pixmap = pixmap.scaled(QSize(this_printer->width() * pixelRatio,
        // /this_printer->height()* pixelRatio) , Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        //pixmap.setDevicePixelRatio(pixelRatio);
        //qDebug() << "======>" << pixmap.size() << pixmap.size();
        //painter.drawPixmap(0,0,pixmap);

        // R G Y Page
        // // 1st Page
        print_widget->findChild<QWidget*>("widget_0");
        print_widget->findChild<QWidget*>("widget_1")->hide();

        print_widget->findChild<QLabel*>("label")->setText(" ");
        print_widget->findChild<QLabel*>("page_number")->setText(QString("Page %1 of %2").arg(1).arg(imageList.size()));

        print_widget->findChild<QLabel*>("lb_patient_name")->setText("PatientName: "+m_qmap.value("PatientName"));
        print_widget->findChild<QLabel*>("lb_patient_id")->setText("PatientID: "+m_qmap.value("PatientID"));
        print_widget->findChild<QLabel*>("lb_patient_age")->setText("PatientBirthDate: "+m_qmap.value("PatientBirthDate"));
        print_widget->findChild<QLabel*>("lb_patient_sex")->setText("PatientSex: "+m_qmap.value("PatientSex"));

        print_widget->findChild<QLabel*>("lb_ct_11")->setText("ContentDate: "+m_qmap.value("ContentDate"));
        print_widget->findChild<QLabel*>("lb_ct_12")->setText("SeriesDate: "+m_qmap.value("SeriesDate"));
        print_widget->findChild<QLabel*>("lb_ct_21")->setText("ContentTime: "+m_qmap.value("ContentTime"));
        print_widget->findChild<QLabel*>("lb_ct_22")->setText("SeriesNumber: "+m_qmap.value("SeriesNumber"));
        print_widget->findChild<QLabel*>("lb_ct_31")->setText("SeriesDescription: "+m_qmap.value("SeriesDescription"));
        print_widget->findChild<QLabel*>("lb_ct_32")->setText("FrameOfReferenceUID: "+m_qmap.value("FrameOfReferenceUID"));
        print_widget->findChild<QLabel*>("lb_ct_41")->setText("SeriesInstanceUID: "+m_qmap.value("SeriesInstanceUID"));
        print_widget->findChild<QLabel*>("lb_ct_42")->setText("Manufacturer: "+m_qmap.value("Manufacturer"));
        print_widget->findChild<QLabel*>("lb_ct_51")->setText("Modality: "+m_qmap.value("Modality"));
        print_widget->findChild<QLabel*>("lb_ct_52")->setText("SeriesTime: "+m_qmap.value("SeriesTime"));
        print_widget->findChild<QLabel*>("lb_ct_61")->setText("Model: "+m_qmap.value("Model"));

        print_widget->findChild<QLabel*>("label_study11")->setText("StudyDate: "+m_qmap.value("StudyDate"));
        print_widget->findChild<QLabel*>("label_study12")->setText("StudyDescription: "+m_qmap.value("StudyDescription"));
        print_widget->findChild<QLabel*>("label_study21")->setText("StudyID: "+m_qmap.value("StudyID"));
        print_widget->findChild<QLabel*>("label_study22")->setText("SeriesTime: "+m_qmap.value("SeriesTime"));
        print_widget->findChild<QLabel*>("label_study31")->setText("StudyInstanceUID: "+m_qmap.value("StudyInstanceUID"));
        print_widget->findChild<QLabel*>("label_study32")->setText("StudyTime: "+m_qmap.value("StudyTime"));

        int scale_size_h = this_printer->height() * 0.8;
        print_widget->findChild<QLabel*>("label_img")->setPixmap(QPixmap::fromImage(imageList[0].scaled(scale_size_h, scale_size_h)));
        print_widget->render(&painter);
        this_printer->newPage();

        // other Pages
        print_widget->findChild<QWidget*>("widget_1")->show();
        print_widget->findChild<QWidget*>("widget_22")->hide();
        print_widget->findChild<QLabel*>("lb_2_patient_name")->setText("PatientName: " + m_qmap.value("PatientName"));
        print_widget->findChild<QLabel*>("lb_2_patient_id")->setText("PatientID: " + m_qmap.value("PatientID"));
        print_widget->findChild<QLabel*>("lb_2_patient_age")->setText("PatientBirthDate: " + m_qmap.value("PatientBirthDate"));
        print_widget->findChild<QLabel*>("lb_2_patient_sex")->setText("PatientSex: " + m_qmap.value("PatientSex"));
        print_widget->findChild<QLabel*>("lb_2_gray_level")->setText("Depth: "+ QString::number(imageList[0].depth()));
        print_widget->findChild<QLabel*>("lb_2_gray_width")->setText("Width: "+ QString::number(imageList[0].width()));

        int page_num = 1;
        for (auto& i : imageList) {
            print_widget->findChild<QLabel*>("page_number")->setText(QString("Page %1 of %2").arg(++page_num).arg(imageList.size()));
            print_widget->findChild<QLabel*>("label_img")->setPixmap(QPixmap::fromImage(i.scaled(scale_size_h, scale_size_h)));
            print_widget->render(&painter);
            this_printer->newPage();
        }
        painter.end();
    });
    previewDialog.exec();
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onSaveTagsCheckBoxToggled(bool on)
{
  Q_D(qSlicerDICOMExportDialog);

  if (on)
  {
    // Commit changes to exported series item(s) and their study and patient parents
    d->DICOMTagEditorWidget->commitChangesToItems();
  }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::onExportToFolderCheckBoxToggled(bool on)
{
  // Write checkbox state into application settings so that it is remembered across sessions
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("DICOM/ExportToFolder", on);
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportDialog::showUpdatedDICOMBrowser()
{
  if (!qSlicerApplication::application()->dicomDatabase())
  {
    qCritical() << Q_FUNC_INFO << ": No DICOM database is set";
    return;
  }

  // Show DICOM browser and update DICOM database
  // (no direct function for it, so re-set the folder)
  PythonQt::init();
  PythonQtObjectPtr openBrowserContext = PythonQt::self()->getMainModule();
  openBrowserContext.evalScript(QString(
    "slicer.util.selectModule('DICOM')\n"
    "slicer.modules.DICOMInstance.browserWidget.dicomBrowser.dicomTableManager().updateTableViews()\n"
    ));
}

//-----------------------------------------------------------------------------
bool qSlicerDICOMExportDialog::exportSeries(const QDir& outputFolder)
{
  Q_D(qSlicerDICOMExportDialog);

  // Commit changes to exported series item(s) and their study and patient
  // parents after successful export if user requested it
  if (d->SaveTagsCheckBox->isChecked())
  {
    d->DICOMTagEditorWidget->commitChangesToItems();
  }

  if (d->DICOMTagEditorWidget->exportables().isEmpty())
  {
    return false;
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
  exportContext.evalScript(QString(
    "plugin = slicer.modules.dicomPlugins[%1]()\n"
    "try:\n"
    "  errorMessage = plugin.export(exportables)\n"
    "except Exception as e:\n"
    "  errorMessage = 'DICOM export failed. See application log for details';\n"
    "  logging.error(f'Failed to export using plugin {type(plugin).__name__}')\n"
    "  import traceback\n"
    "  traceback.print_exc()\n")
    .arg(qSlicerCorePythonManager::toPythonStringLiteral(d->DICOMTagEditorWidget->exportables()[0]->pluginClass())));
  QApplication::restoreOverrideCursor();

  // Extract error message from python
  QString errorMessage = exportContext.getVariable("errorMessage").toString();
  if (errorMessage.isNull())
  {
    // Invalid return value from DICOM exporter (it never returned)
    d->ErrorLabel->setText("Error occurred in exporter");
    return false;
  }
  else if (!errorMessage.isEmpty())
  {
    // Exporter encountered error
    d->ErrorLabel->setText(errorMessage);
    return false;
  }

  return true;
}
