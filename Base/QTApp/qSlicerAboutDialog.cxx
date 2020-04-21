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

// SlicerApp includes
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
  d->CreditsTextBrowser->setFontPointSize(25);
  d->CreditsTextBrowser->append(slicer->applicationName());
  d->CreditsTextBrowser->setFontPointSize(11);
  d->CreditsTextBrowser->append("");
  if (!slicer->isCustomMainApplication())
    {
    d->CreditsTextBrowser->append(slicer->applicationVersion() + " " + "r" + slicer->revision()
      + " / " + slicer->repositoryRevision());
    d->CreditsTextBrowser->append("");
    d->CreditsTextBrowser->append("");
    d->CreditsTextBrowser->insertHtml("<a href=\"http://download.slicer.org/\">Download</a> a newer version<br />");
    d->CreditsTextBrowser->append("");
    }
  else
    {
    d->CreditsTextBrowser->append(slicer->applicationVersion() + " (" + slicer->mainApplicationRepositoryRevision() + ")");
    d->CreditsTextBrowser->append("");
    }
  d->CreditsTextBrowser->insertHtml(slicer->acknowledgment());
  d->CreditsTextBrowser->insertHtml(slicer->libraries());
  d->SlicerLinksTextBrowser->insertHtml(slicer->copyrights());
  d->CreditsTextBrowser->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

  connect(d->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

//-----------------------------------------------------------------------------
void qSlicerAboutDialog::setLogo(const QPixmap& newLogo)
{
  Q_D(qSlicerAboutDialog);
  d->SlicerLabel->setPixmap(newLogo);
}

//-----------------------------------------------------------------------------
qSlicerAboutDialog::~qSlicerAboutDialog() = default;
