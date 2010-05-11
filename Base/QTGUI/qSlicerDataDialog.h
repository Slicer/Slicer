#ifndef __qSlicerDataDialog_h
#define __qSlicerDataDialog_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

/// Forward declarations
class qSlicerDataDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerDataDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerDataDialog(QObject* parent =0);
  virtual ~qSlicerDataDialog();
  
  virtual qSlicerIO::IOFileType fileType()const;
  virtual qSlicerFileDialog::IOAction action()const;

  ///
  /// run the dialog to select the file/files/directory
  virtual bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties());

private:
  CTK_DECLARE_PRIVATE(qSlicerDataDialog);
};

#endif
