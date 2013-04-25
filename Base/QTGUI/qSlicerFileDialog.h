/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerFileDialog_h
#define __qSlicerFileDialog_h

// Qt includes
#include <QObject>
#include <QStringList>
class QDragEnterEvent;
class QDropEvent;

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
  Q_ENUMS(IOAction)
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
  /// run the dialog to select the file/files/directory
  /// Properties availables with IOPorperties: fileMode, multipleFiles, fileType.
  virtual bool exec(const qSlicerIO::IOProperties& ioProperties =
                    qSlicerIO::IOProperties()) = 0;

  /// TBD: move in qSlicerCoreIOManager or qSlicerIOManager ?
  /// Return the namefilters of all the readers in IOManager corresponding to
  /// fileType
  static QStringList nameFilters(qSlicerIO::IOFileType fileType =
                                 QString("NoFile"));

  /// Accept or ignore drops.
  /// Can be reimplemented in subclass to support drag&drop.
  /// Do nothing by default.
  /// \sa dropEvent()
  virtual void dragEnterEvent(QDragEnterEvent *event);

  /// Handle drop events: populate the dialog with the droped mime data.
  /// Can be reimplemented in subclass to support drag&drop.
  /// Do nothing by default.
  /// If it does something, acceptProposedAction() or accept() must be called.
  /// \sa dragEnterEvent()
  virtual void dropEvent(QDropEvent *event);

private:
//  Q_DECLARE_PRIVATE(qSlicerFileDialog);
  Q_DISABLE_COPY(qSlicerFileDialog);
};

class qSlicerStandardFileDialogPrivate;
class ctkFileDialog;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStandardFileDialog : public qSlicerFileDialog
{
  Q_OBJECT
public:
  qSlicerStandardFileDialog(QObject* parent=0);
  virtual ~qSlicerStandardFileDialog();

  void setFileType(qSlicerIO::IOFileType fileType);
  virtual qSlicerIO::IOFileType fileType()const;

  void setAction(qSlicerFileDialog::IOAction dialogAction);
  virtual qSlicerFileDialog::IOAction action()const;

  virtual bool exec(const qSlicerIO::IOProperties& ioProperties =
                    qSlicerIO::IOProperties());

  /// Properties availables with IOPorperties: fileMode, multipleFiles, fileType.
  static QStringList getOpenFileName(qSlicerIO::IOProperties ioProperties =
                                     qSlicerIO::IOProperties());
  static QString getExistingDirectory(qSlicerIO::IOProperties ioProperties =
                                      qSlicerIO::IOProperties());

protected:
  static ctkFileDialog* createFileDialog(const qSlicerIO::IOProperties& ioProperties =
                                         qSlicerIO::IOProperties());

protected:
  QScopedPointer<qSlicerStandardFileDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerStandardFileDialog);
  Q_DISABLE_COPY(qSlicerStandardFileDialog);
};

Q_DECLARE_METATYPE(qSlicerFileDialog::IOAction)

#endif
