#ifndef __qSlicerFileDialog_h
#define __qSlicerFileDialog_h

// Qt includes
#include <QObject>
#include <QStringList>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTGUIExport.h"

/// Forward declarations
class qSlicerIOManager;
//class qSlicerFileDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerFileDialog : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerFileDialog(QObject* parent =0);
  virtual ~qSlicerFileDialog();
  
  virtual qSlicerIO::IOFileType fileType()const = 0;
  enum IOAction
  {
    Read,
    Write
  };
  virtual qSlicerFileDialog::IOAction action()const = 0;
  ///
  /// run the dialog to select the file/files/directory
  virtual bool exec(const qSlicerIO::IOProperties& ioProperties =
                    qSlicerIO::IOProperties()) = 0;

  ///
  /// TBD: move in qSlicerCoreIOManager or qSlicerIOManager ?
  /// Return the namefilters of all the readers in IOManager corresponding to 
  /// fileType
  static QStringList nameFilters(qSlicerIO::IOFileType fileType);

//private:
//  CTK_DECLARE_PRIVATE(qSlicerFileDialog);
};

class qSlicerStandardFileDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStandardFileDialog : public qSlicerFileDialog
{
public:
  qSlicerStandardFileDialog(QObject* parent=0);

  void setFileType(qSlicerIO::IOFileType fileType);
  virtual qSlicerIO::IOFileType fileType()const;

  void setAction(qSlicerFileDialog::IOAction dialogAction);
  virtual qSlicerFileDialog::IOAction action()const;

  virtual bool exec(const qSlicerIO::IOProperties& ioProperties =
                    qSlicerIO::IOProperties());
private:
  CTK_DECLARE_PRIVATE(qSlicerStandardFileDialog);
};

#endif
