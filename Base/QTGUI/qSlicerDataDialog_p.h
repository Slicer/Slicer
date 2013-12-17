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

// SlicerQt includes
#include "qSlicerDataDialog.h"
#include "ui_qSlicerDataDialog.h"

class QDragEnterEvent;
class QDropEvent;

//-----------------------------------------------------------------------------
class qSlicerDataDialogPrivate
  : public QDialog
  , public Ui_qSlicerDataDialog
{
  Q_OBJECT
public:
  explicit qSlicerDataDialogPrivate(QWidget* _parent=0);
  virtual ~qSlicerDataDialogPrivate();

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
  void addFile(const QFileInfo& file);
  void setFileOptions(int row, const QString& filePath, const QString& fileDescription);
  /// Return the row the last signal comes from.
  int senderRow()const;
  bool propagateChange(int changedRow)const;
  /// Return true if the 2 items have the same filetype options.
  /// I.e. same items int the TypeColumn combobox.
  bool haveSameTypeOption(int row1, int row2)const;
private:
  friend class qSlicerDataDialog;
};


#endif
