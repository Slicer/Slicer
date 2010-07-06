#ifndef __qSlicerIOManager_h
#define __qSlicerIOManager_h

// Qt includes
#include <QList>
#include <QString>
#include <QUrl>

// CTK includes
#include <ctkPimpl.h>

// SlicerQ includes
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileDialog.h"

#include "qSlicerBaseQTGUIExport.h"

/// QT declarations
class QWidget;

class qSlicerIOManagerPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerIOManager : public qSlicerCoreIOManager
{
  Q_OBJECT;
public:
  typedef qSlicerCoreIOManager Superclass;
  qSlicerIOManager(QObject* parent = 0);
  virtual ~qSlicerIOManager();

  bool openLoadSceneDialog();
  bool openAddSceneDialog();
  inline bool openAddVolumesDialog();
  inline bool openAddModelsDialog();
  inline bool openAddDataDialog();
  inline bool openAddScalarOverlayDialog();
  inline bool openSaveDataDialog();

  bool openDialog(qSlicerIO::IOFileType fileType,
                  qSlicerFileDialog::IOAction action,
                  const qSlicerIO::IOProperties& ioProperties
                    = qSlicerIO::IOProperties());

  void addHistory(const QString& path);
  const QStringList& history()const;

  void addFavorite(const QUrl& urls);
  const QList<QUrl>& favorites()const;

  ///
  /// Takes ownership. Any previously set dialog corresponding to the same
  /// fileType (only 1 dialog per filetype) is overriden.
  void registerDialog(qSlicerFileDialog* dialog);

protected:
  friend class qSlicerFileDialog;
  using qSlicerCoreIOManager::ios;
private:
  CTK_DECLARE_PRIVATE(qSlicerIOManager);
};

//------------------------------------------------------------------------------
bool qSlicerIOManager::openAddDataDialog()
{
  return this->openDialog(qSlicerIO::NoFile, qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddVolumesDialog()
{
  return this->openDialog(qSlicerIO::VolumeFile, qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddModelsDialog()
{
  return this->openDialog(qSlicerIO::ModelFile, qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddScalarOverlayDialog()
{
  return this->openDialog(qSlicerIO::ScalarOverlayFile, qSlicerFileDialog::Read);
}

//------------------------------------------------------------------------------
bool qSlicerIOManager::openSaveDataDialog()
{
  return this->openDialog(qSlicerIO::NoFile, qSlicerFileDialog::Write);
}

#endif
