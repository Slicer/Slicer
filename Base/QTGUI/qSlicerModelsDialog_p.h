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

#ifndef __qSlicerModelsDialog_p_h
#define __qSlicerModelsDialog_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QDialog>
#include <QStringList>

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerModelsDialog.h"
#include "ui_qSlicerModelsDialog.h"

//-----------------------------------------------------------------------------
class qSlicerModelsDialogPrivate
  : public QDialog
  , public Ui_qSlicerModelsDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerModelsDialog);
protected:
  qSlicerModelsDialog* const q_ptr;
public:
  explicit qSlicerModelsDialogPrivate(qSlicerModelsDialog& object, QWidget* parentWidget = nullptr);
  ~qSlicerModelsDialogPrivate() override;

  void init();

public slots:
  void openAddModelFileDialog();
  void openAddModelDirectoryDialog();
protected:
  QStringList SelectedFiles;
  QStringList LoadedNodeIDs;
};

#endif
