#ifndef __qSlicerModelsDialog_h
#define __qSlicerModelsDialog_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModelsDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModelsDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerModelsDialog(QObject* parent =0);
  virtual ~qSlicerModelsDialog();

  virtual qSlicerIO::IOFileType fileType()const;
  virtual qSlicerFileDialog::IOAction action()const;

  ///
  /// run the dialog to select the file/files/directory
  virtual bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties());

private:
  CTK_DECLARE_PRIVATE(qSlicerModelsDialog);
};

#endif
