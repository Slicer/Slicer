#ifndef __qSlicerDataDialog_p_h
#define __qSlicerDataDialog_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QDialog>
#include <QDir>
#include <QFileInfo>

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerDataDialog.h"
#include "ui_qSlicerDataDialog.h"

class QDragEnterEvent;
class QDropEvent;
class QTemporaryDir;

//-----------------------------------------------------------------------------
class qSlicerDataDialogPrivate
  : public QDialog
  , public Ui_qSlicerDataDialog
{
  Q_OBJECT
public:
  explicit qSlicerDataDialogPrivate(QWidget* _parent=nullptr);
  ~qSlicerDataDialogPrivate() override;

  QList<qSlicerIO::IOProperties> selectedFiles()const;
public slots:
  void addDirectory();
  void addFiles();
  void reset();
  void showOptions(bool);

protected slots:
  void onFileTypeChanged(const QString&);
  void onFileTypeActivated(const QString&);

//  void updateCheckBoxes(Qt::Orientation orientation, int first, int last);
//  void updateCheckBoxHeader(int row, int column);

protected:
  enum ColumnType
  {
    FileColumn = 0,
    TypeColumn = 1,
    OptionsColumn = 2
  };

  void addDirectory(const QDir& directory);
  // addFile doesn't resize the columns to contents (as it might be a bit too
  // time consuming if you do it for every file added).
  // If a readerDescription is specified then only that reader will be offered. If ioProperties is specified as well then
  // it will be used to initialize the reader properties widget.
  void addFile(const QFileInfo& file, const QString& readerDescription=QString(), qSlicerIO::IOProperties* ioProperties=nullptr);
  void setFileOptions(int row, const QString& filePath, const QString& fileDescription);
  /// Return the row the last signal comes from.
  int senderRow()const;
  bool propagateChange(int changedRow)const;
  /// Return true if the 2 items have the same filetype options.
  /// I.e. same items int the TypeColumn combobox.
  bool haveSameTypeOption(int row1, int row2)const;
  /// Check if file is an archive, and if so, give the user
  /// the option to unpack it and load the contents.
  /// Currently only zip files with the extension .zip are handled.
  bool checkAndHandleArchive(const QFileInfo &file);
  /// A holder for the temporary directory so that it doesn't go out of scope before loading.
  QScopedPointer<QTemporaryDir> temporaryArchiveDirectory;
private:
  friend class qSlicerDataDialog;
};


#endif
