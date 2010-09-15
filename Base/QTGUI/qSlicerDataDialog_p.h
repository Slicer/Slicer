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

protected slots:
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
  void addFile(const QFileInfo& file);
};


#endif
