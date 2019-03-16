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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerExtensionsManagerDialog_h
#define __qSlicerExtensionsManagerDialog_h

// Qt includes
#include <QDialog>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsManagerDialogPrivate;
class qSlicerExtensionsManagerModel;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsManagerDialog
  : public QDialog
{
  Q_OBJECT
  Q_PROPERTY(bool restartRequested READ restartRequested WRITE setRestartRequested)
public:
  /// Superclass typedef
  typedef QDialog Superclass;

  /// Constructor
  explicit qSlicerExtensionsManagerDialog(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsManagerDialog() override;

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

  /// Return True if the application is expected to be restarted.
  bool restartRequested()const;

  /// \sa restartRequested()
  void setRestartRequested(bool value);

protected slots:
  void onModelUpdated();

protected:
  QScopedPointer<qSlicerExtensionsManagerDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManagerDialog);
  Q_DISABLE_COPY(qSlicerExtensionsManagerDialog);
};

#endif
