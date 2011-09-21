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

// Qt includes

// SlicerQt includes
#include "qSlicerAboutDialog.h"
#include "qSlicerApplication.h"
#include "ui_qSlicerAboutDialog.h"

//-----------------------------------------------------------------------------
class qSlicerAboutDialogPrivate: public Ui_qSlicerAboutDialog
{
public:
};

//-----------------------------------------------------------------------------
// qSlicerAboutDialogPrivate methods


//-----------------------------------------------------------------------------
// qSlicerAboutDialog methods
qSlicerAboutDialog::qSlicerAboutDialog(QWidget* parentWidget)
 :QDialog(parentWidget)
  , d_ptr(new qSlicerAboutDialogPrivate)
{
  Q_D(qSlicerAboutDialog);
  d->setupUi(this);

  qSlicerApplication* slicer = qSlicerApplication::application();
  d->CreditsTextEdit->append(
    slicer->applicationName() + " "
    + slicer->applicationVersion()+ " "
    + "r" + slicer->repositoryRevision());
  d->CreditsTextEdit->append("");
  d->CreditsTextEdit->append(slicer->copyrights());
  d->CreditsTextEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
}

//-----------------------------------------------------------------------------
qSlicerAboutDialog::~qSlicerAboutDialog()
{
}
