// Qt includes

// SlicerQt includes
#include "qSlicerAboutDialog.h"
#include "qSlicerApplication.h"
#include "ui_qSlicerAboutDialog.h"

//-----------------------------------------------------------------------------
class qSlicerAboutDialogPrivate: public ctkPrivate<qSlicerAboutDialog>,
                                 public Ui_qSlicerAboutDialog
{
public:
  CTK_DECLARE_PUBLIC(qSlicerAboutDialog);
};

//-----------------------------------------------------------------------------
// qSlicerAboutDialogPrivate methods


//-----------------------------------------------------------------------------
// qSlicerAboutDialog methods
qSlicerAboutDialog::qSlicerAboutDialog(QWidget* parentWidget)
 :QDialog(parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerAboutDialog);
  CTK_D(qSlicerAboutDialog);
  d->setupUi(this);

  qSlicerApplication* slicer = qSlicerApplication::application();
  d->CreditsTextEdit->append(slicer->copyrights());
  d->CreditsTextEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
}
