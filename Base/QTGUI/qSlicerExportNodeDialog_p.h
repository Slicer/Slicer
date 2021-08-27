#ifndef __qSlicerExportNodeDialog_p_h
#define __qSlicerExportNodeDialog_p_h

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
#include <QFileInfo>
#include <QHash>
#include <QList>

// Slicer includes
#include "qSlicerExportNodeDialog.h"
#include "ui_qSlicerExportNodeDialog.h"

class vtkMRMLNode;
class qSlicerFileWriterOptionsWidget;

//-----------------------------------------------------------------------------
class qSlicerExportNodeDialogPrivate
  : public QDialog
  , public Ui_qSlicerExportNodeDialog
{
  Q_OBJECT
public:
  typedef qSlicerExportNodeDialogPrivate Self;
  explicit qSlicerExportNodeDialogPrivate(QWidget* _parent=nullptr);
  ~qSlicerExportNodeDialogPrivate() override;

  /* Set the node being exported and fill out the
     dialog widgets with a reasonable initial state */
  bool setup(vtkMRMLScene* scene, vtkMRMLNode* node);

  /* Save the states of some widgets
  so that they can be restored next time the dialog
  is activated. Note that the options widget state
  is not involved here-- instead of saving options widget state
  we save the options widgets themselves in updateOptionsWidget.
  This prevents things from breaking as people make changes to
  options widgets. */
  void saveWidgetStates();

public slots:
  bool exportNode();
  void accept() override; // overrides QDialog::accept()

protected slots:
  void formatChangedSlot();
  void onFilenameEditingFinished();

protected:

  void formatChanged(bool updateFilename);
  void updateOptionsWidget();
  QFileInfo file() const;

  void setOptionsWidget(qSlicerFileWriterOptionsWidget*);
  qSlicerFileWriterOptionsWidget* getOptionsWidget() const;

  // return the actual text of the currently selected option
  // in the format dropdown. e.g. "NRRD (.nrrd)"
  QString formatText() const;

  // return extension corresponding to format selected in dropdown
  // e.g. ".nrrd"
  QString extension() const;

  // return the recommended filename based on current filename and the
  // format selected in the dropdown
  QString recommendedFilename() const;


  vtkMRMLScene* MRMLScene;
  vtkMRMLNode* MRMLNode;

  QString lastUsedDirectory;
  QList<QString> lastUsedFormats;
  bool lastUsedHardenTransform;

  // mapping from formats to options widgets, to save them as they get created
  QHash<QString,qSlicerFileWriterOptionsWidget*> formatToOptionsWidget;
};


#endif
