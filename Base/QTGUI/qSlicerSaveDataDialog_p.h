#ifndef __qSlicerSaveDataDialog_p_h
#define __qSlicerSaveDataDialog_p_h

// Qt includes
#include <QDialog>
#include <QDir>
#include <QFileInfo>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerSaveDataDialog.h"
#include "ui_qSlicerSaveDataDialog.h"
#include "qSlicerBaseQTGUIExport.h"

class vtkMRMLStorableNode;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSaveDataDialogPrivate
  : public QDialog
  , public ctkPrivate<qSlicerSaveDataDialog>
  , public Ui_qSlicerSaveDataDialog
{
  Q_OBJECT
  CTK_DECLARE_PUBLIC(qSlicerSaveDataDialog);
public:
  explicit qSlicerSaveDataDialogPrivate(QWidget* _parent=0);
  virtual ~qSlicerSaveDataDialogPrivate();

  void populateItems();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;

public slots:
  void setDirectory(const QString& newDirectory);
  void selectModifiedSceneData();
  void selectModifiedData();
  bool save();
  ///
  /// Reimplemented from QDialog::accept(), only accept the dialog if
  /// save() is successful.
  virtual void accept();

protected slots:
  void formatChanged();
  bool saveScene();
  bool saveNodes();
  QFileInfo sceneFile()const ;

protected:
  enum ColumnType
  {
    NodeNameColumn = 0,
    NodeTypeColumn = 1,
    NodeStatusColumn = 2,
    FileFormatColumn = 3,
    FileNameColumn = 4,
    FileDirectoryColumn = 5
  };

  bool              prepareForSaving();
  void              populateScene();
  void              populateNode(vtkMRMLStorableNode* node);
  QFileInfo         nodeFileInfo(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeNameItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeTypeItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QWidget*          createFileFormatsWidget(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QTableWidgetItem* createFileNameItem(const QFileInfo& fileInfo);
  QWidget*          createFileDirectoryWidget(const QFileInfo& fileInfo);

  vtkMRMLScene* MRMLScene;
};

#endif
