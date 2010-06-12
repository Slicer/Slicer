#ifndef __qSlicerSaveDataDialog_h
#define __qSlicerSaveDataDialog_h

// SlicerQt includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

/// Forward declarations
class qSlicerSaveDataDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSaveDataDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerSaveDataDialog(QObject* parent = 0);
  virtual ~qSlicerSaveDataDialog();

  virtual qSlicerIO::IOFileType fileType()const;
  virtual qSlicerFileDialog::IOAction action()const;

  ///
  /// Open the data dialog and save the nodes/scene
  virtual bool exec(const qSlicerIO::IOProperties& readerProperties =
                    qSlicerIO::IOProperties());

private:
  CTK_DECLARE_PRIVATE(qSlicerSaveDataDialog);
};

#endif
