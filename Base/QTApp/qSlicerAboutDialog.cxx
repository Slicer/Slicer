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

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
# include "qSlicerApplicationUpdateManager.h"
#endif

//-----------------------------------------------------------------------------
class qSlicerAboutDialogPrivate : public Ui_qSlicerAboutDialog
{
public:
};

//-----------------------------------------------------------------------------
// qSlicerAboutDialogPrivate methods

//-----------------------------------------------------------------------------
// qSlicerAboutDialog methods
qSlicerAboutDialog::qSlicerAboutDialog(QWidget* parentWidget)
  : QDialog(parentWidget)
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
    d->CreditsTextBrowser->append(slicer->applicationVersion() + " " + "r" + slicer->revision() + " / "
                                  + slicer->repositoryRevision());
    d->CreditsTextBrowser->append("");
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
    if (qSlicerApplicationUpdateManager::isApplicationUpdateEnabled())
    {
      qSlicerApplicationUpdateManager* applicationUpdateManager = slicer->applicationUpdateManager();
      if (applicationUpdateManager && applicationUpdateManager->isUpdateAvailable())
      {
        QString appUpdateText =
          tr("New application version is available: %1").arg(applicationUpdateManager->latestReleaseVersion());
        d->CreditsTextBrowser->insertHtml(QString("<b><a href=\"%1\"><font color=\"orange\">%2</font></a></b>")
                                            .arg(applicationUpdateManager->applicationDownloadPageUrl().toString())
                                            .arg(appUpdateText));
        d->CreditsTextBrowser->append("");
      }
    }
#else
    QString downloadSiteLink = QString("<a href=\"https://download.slicer.org/\">%1</a>").arg(tr("download site"));
    d->CreditsTextBrowser->insertHtml(tr("Visit the %1 to check if a new version is available.").arg(downloadSiteLink));
    d->CreditsTextBrowser->append("");
#endif
    d->CreditsTextBrowser->append("");
  }
  else
  {
    d->CreditsTextBrowser->append(slicer->applicationVersion() + " (" + slicer->mainApplicationRepositoryRevision()
                                  + ")");
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
