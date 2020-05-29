#ifndef __qSlicerIOManager_h
#define __qSlicerIOManager_h

// Qt includes
#include <QList>
#include <QString>
#include <QUrl>

// CTK includes
#include <ctkVTKObject.h>

// SlicerQ includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileDialog.h"

#include "qSlicerBaseQTGUIExport.h"

/// Qt declarations
class QDragEnterEvent;
class QDropEvent;
class QWidget;

class qSlicerIOManagerPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerIOManager : public qSlicerCoreIOManager
{
  Q_OBJECT;
  QVTK_OBJECT;
public:
  typedef qSlicerCoreIOManager Superclass;
  qSlicerIOManager(QObject* parent = nullptr);
  ~qSlicerIOManager() override;

  /// Search for the most appropriate dialog based on the action and fileType,
  /// and open it. Once the user select the file(s), the action (read or write)
  /// is done. Note that to write a node, the "nodeID" property must be passed.
  /// If no dialog is registered for a given fileType (e.g.
  /// QString("SceneFile")), a default dialog (qSlicerStandardFileDialog) is
  /// used.
  /// If reading files (action == qSlicerFileDialog::Read) and if loadedNodes
  /// is not null, the loadedNodes collection is being populated with the
  /// loaded nodes.
  /// Returns true on success, false otherwise.
  Q_INVOKABLE bool openDialog(qSlicerIO::IOFileType fileType,
                              qSlicerFileDialog::IOAction action,
                              qSlicerIO::IOProperties ioProperties = qSlicerIO::IOProperties(),
                              vtkCollection* loadedNodes = nullptr);

  void addHistory(const QString& path);
  const QStringList& history()const;

  void setFavorites(const QList<QUrl>& urls);
  const QList<QUrl>& favorites()const;

  /// Takes ownership. Any previously set dialog corresponding to the same
  /// fileType (only 1 dialog per filetype) is overridden.
  void registerDialog(qSlicerFileDialog* dialog);

  /// Displays a progress dialog if it takes too long to load
  /// There is no way to know in advance how long the loading will take, so the
  /// progress dialog listens to the scene and increment the progress anytime
  /// a node is added.
  Q_INVOKABLE bool loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes = nullptr) override;
  /// If you have a list of nodes to load, it's best to use this function
  /// in order to have a unique progress dialog instead of multiple ones.
  /// It internally calls loadNodes() for each file.
  bool loadNodes(const QList<qSlicerIO::IOProperties>& files,
                         vtkCollection* loadedNodes = nullptr) override;

  /// dragEnterEvents can be forwarded to the IOManager, if a registered dialog
  /// supports it, the event is accepted, otherwise ignored.
  /// \sa dropEvent()
  void dragEnterEvent(QDragEnterEvent *event);

  /// Search, in the list of registered readers, the first dialog that
  /// handles the drop event. If the event is accepted by the dialog (
  /// usually the is also used to populate the dialog), the manager opens the dialog,
  /// otherwise the next dialog is tested. The order in which dialogs are
  /// being tested is the opposite of the dialogs are registered.
  /// \sa dragEnterEvent()
  void dropEvent(QDropEvent *event);

public slots:

  void openScreenshotDialog();
  void openSceneViewsDialog();
  bool openLoadSceneDialog();
  bool openAddSceneDialog();
  inline bool openAddDataDialog();
  inline bool openAddDataDialog(QString fileName);
  inline bool openAddVolumeDialog();
  inline bool openAddVolumesDialog();
  inline bool openAddModelDialog();
  inline bool openAddScalarOverlayDialog();
  inline bool openAddTransformDialog();
  inline bool openAddColorTableDialog();
  inline bool openAddFiducialDialog();
  inline bool openAddMarkupsDialog();
  inline bool openAddFiberBundleDialog();
  inline bool openSaveDataDialog();

protected slots:
  void updateProgressDialog();

protected:
  friend class qSlicerFileDialog;
  using qSlicerCoreIOManager::readers;
protected:
  QScopedPointer<qSlicerIOManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIOManager);
  Q_DISABLE_COPY(qSlicerIOManager);
};

//------------------------------------------------------------------------------
bool qSlicerIOManager::openAddDataDialog(QString fileName)
{
  qSlicerIO::IOProperties ioProperties;
  ioProperties["fileName"] = fileName;
  return this->openDialog(QString("NoFile"), qSlicerFileDialog::Read, ioProperties);
}

//------------------------------------------------------------------------------
bool qSlicerIOManager::openAddDataDialog()
{
  return this->openDialog(QString("NoFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddVolumeDialog()
{
  return this->openDialog(QString("VolumeFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddVolumesDialog()
{
  qSlicerIO::IOProperties ioProperties;
  ioProperties["multipleFiles"] = true;
  return this->openDialog(QString("VolumeFile"), qSlicerFileDialog::Read, ioProperties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddModelDialog()
{
  return this->openDialog(QString("ModelFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddScalarOverlayDialog()
{
  return this->openDialog(QString("ScalarOverlayFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddTransformDialog()
{
  return this->openDialog(QString("TransformFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddColorTableDialog()
{
  return this->openDialog(QString("ColorTableFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddFiducialDialog()
{
  return this->openDialog(QString("FiducialListFile"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddMarkupsDialog()
{
  return this->openDialog(QString("MarkupsFiducials"), qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddFiberBundleDialog()
{
  return this->openDialog(QString("FiberBundleFile"), qSlicerFileDialog::Read);
}

//------------------------------------------------------------------------------
bool qSlicerIOManager::openSaveDataDialog()
{
  return this->openDialog(QString("NoFile"), qSlicerFileDialog::Write);
}

#endif
