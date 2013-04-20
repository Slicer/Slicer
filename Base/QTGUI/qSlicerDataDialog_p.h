#ifndef __qSlicerDataDialog_p_h
#define __qSlicerDataDialog_p_h

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

private:
  friend class qSlicerDataDialog;
};


#endif
