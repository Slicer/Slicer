/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qMRMLTextWidget_h
#define __qMRMLTextWidget_h

// Slicer includes
#include "qSlicerWidget.h"

// Text widgets includes
#include "qSlicerTextsModuleWidgetsExport.h"
#include "ui_qMRMLTextWidget.h"

class vtkMRMLNode;
class vtkMRMLTextNode;
class qMRMLTextWidgetPrivate;
class QTextEdit;

/// \ingroup Slicer_QtModules_Texts
class Q_SLICER_MODULE_TEXTS_WIDGETS_EXPORT qMRMLTextWidget : public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qMRMLTextWidget(QWidget *parent=nullptr);
  ~qMRMLTextWidget() override;

  Q_PROPERTY(bool autoSave READ isAutoSave WRITE setAutoSave);
  Q_PROPERTY(bool editing READ isEditing);
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly);
  Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap);

  /// Get the text node
  Q_INVOKABLE vtkMRMLTextNode* mrmlTextNode() const;
  Q_INVOKABLE vtkMRMLNode* mrmlNode() const;

  /// Returns true if the text editor is read only
  /// If read only is enabled, only the text edit will be shown, and the user will be unable to type in the text edit.
  /// \sa setReadOnly()
  bool isReadOnly();

  /// Returns true if text changes made in the widget must be immediately saved into the text node.
  /// If true, the text editor will propagate text changes to the vtkMRMLTextNode after each keypress.
  /// If false, the text editor will only update the node when "Save" is clicked, and changes from the
  /// text node will not be shown in the widget while the text is being edited.
  /// Save and Cancel buttons are only shown if auto-save is disabled.
  /// \sa setAutoSave()
  bool isAutoSave();

  /// Returns true if the text box is in edit mode
  bool isEditing();

  /// Returns the word wrap mode used in the text editor
  /// \sa setWordWrap()
  bool wordWrap();

  /// Returns the internal text editor widget to allow low-level access and customization.
  Q_INVOKABLE QTextEdit* textEditWidget();

public slots:
  /// Reimplemented from qSlicerWidget
  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Set the read only property of the text editor.
  /// If read only is enabled, only the text edit will be shown, and the user will be unable to edit the text.
  /// \sa isReadOnly()
  void setReadOnly(bool readOnly);

  /// Set the continuous update property of the text editor
  /// If true, the text editor will propogate the text to the vtkMRMLTextNode as it is modified, and vice versa.
  /// If false, the text editor will only update the node when "Save" is clicked, and changes from the vtkMRMLTextNode will not be propagated
  /// if the text is being edited.
  /// When auto update is enabled, only the text edit will be shown.
  /// \sa isAutoSave()
  void setAutoSave(bool autoSave);

  /// Set the word wrap mode to be used by the text editor
  /// \sa wordWrap()
  void setWordWrap(bool wordWrap);

  /// Set the currently observed text node
  /// \sa mrmlTextNode()
  void setMRMLTextNode(vtkMRMLTextNode* textNode);

  /// Utility function to simply connect signals/slots with Qt Designer
  /// \sa mrmlNode()
  void setMRMLNode(vtkMRMLNode* textNode);

public slots:
  /// Start editing mode
  void startEdits();

  /// Finish editing, discarding all changes.
  void cancelEdits();

  /// Finish editing, saving edited contents to the text node.
  void saveEdits();

signals:
  /// This signal is emitted if updates to the widget from the MRML node have been requested.
  /// \sa updateWidgetFromMRMLFinished()
  void updateWidgetFromMRMLRequested();

  /// This signal is emitted if updates to the MRML node from the widget have been requested.
  /// \sa updateMRMLFromWidgetFinished()
  void updateMRMLFromWidgetRequested();

  /// This signal is emitted if updates to the widget from the MRML node have finished.
  /// \sa updateWidgetFromMRMLRequested()
  void updateWidgetFromMRMLFinished();

  /// This signal is emitted if updates to the MRML node from the widget have finished.
  /// \sa updateMRMLFromWidgetRequested()
  void updateMRMLFromWidgetFinished();

  /// This signal is emitted if the node changes
  /// \sa setMRMLNode(), setMRMLTextNode(), mrmlNode(), mrmlTextNode()
  void mrmlNodeChanged(vtkMRMLNode*);

  /// This signal is emitted if the read only property is changed
  void readOnlyChanged(bool);

  /// This signal is emitted if the autoSave property is changed
  bool autoSaveChanged(bool);

  /// This signal is emitted when the user starts/stops the widget edit mode
  void editingChanged(bool);

protected slots:
  /// Update the GUI to reflect the currently selected text node.
  void updateWidgetFromMRML();

  /// Update the MRML node to reflect the currently state of the GUI.
  void updateMRMLFromWidget();

  /// Method invoked when the contents of the text node is modified
  void onTextNodeContentsModified();

  /// Method invoked when the contents of the text edit is changed
  void onTextEditChanged();

protected:
  QScopedPointer<qMRMLTextWidgetPrivate> d_ptr;

  // Setup the UI and establish connections
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qMRMLTextWidget);
  Q_DISABLE_COPY(qMRMLTextWidget);

protected:
  bool AutoSave{false};
  bool ReadOnly{false};

};

#endif
