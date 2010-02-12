#ifndef __qCTKAddRemoveComboBox_p_h
#define __qCTKAddRemoveComboBox_p_h

#include "qCTKAddRemoveComboBox.h"
#include "ui_qCTKAddRemoveComboBox.h"

/// qCTK includes 
#include "qCTKPimpl.h"


//-----------------------------------------------------------------------------
class qCTKAddRemoveComboBoxPrivate : public QObject,
                                     public qCTKPrivate<qCTKAddRemoveComboBox>,
                                     public Ui_qCTKAddRemoveComboBox
{
  Q_OBJECT
public:
  qCTKAddRemoveComboBoxPrivate();

  /// 
  /// Insert 'Add', 'Remove' and 'Edit' item in the combobox
  /// Note also that no signals are emitted while the items are inserted
  void insertActionItems();

  ///
  /// Remove 'Add', 'Remove' and 'Edit' item from the combox
  /// Note also that no signals are emitted while the items are removed
  void removeActionItems();

  /// 
  /// Insert 'None' item
  /// Note: Also make sure that no signal is emitted while the item is inserted
  /// That function doesn't prevent from inserting multiple time the 'None' item
  void insertEmptyItem();

  void connectComboBox(QComboBox* combobox);

public slots:
  void currentIndexChanged(int index);
  void activated(int index);

public:
  /// Empty item
  QString EmptyText;

  /// Number of action items (including separator)
  unsigned int ActionItemsCount;
  
  /// Actions text
  QString AddText;
  QString RemoveText;
  QString EditText;

  /// Set to true when inserting either action items or the 'None' item.
  /// Will prevent the itemAdded signal from being sent
  bool    AddingEmptyItem; 
  bool    AddingActionItems;

  /// Set to true when removing either action items or the 'None' item.
  /// Will prevent the itemRemoved signal from being sent
  bool    RemovingEmptyItem;
  bool    RemovingActionItems;

  /// Actions state
  bool    AddEnabled;
  bool    RemoveEnabled;
  bool    EditEnabled;

  /// If true, it means there is no item beside of the 'None' one
  bool    HasEmptyItem;

  /// If greater than 0, it means the actions item have been added
  int ActionItemsIndex; 
  
  /// If True, it means the PushButtons (add, remove, edit) are enabled and visible,
  /// Otherwise, Actions (add, remove, edit) are available in the combobox.
  bool    PushButtonEnabled;

  /// Store the index of the current selected item
  /// After the user selected an action item, if needed, it allows to restore
  /// the current item.
  int     CurrentItemIndex;

  /// Set to true when restoring the item selected before the user click an one action item
  bool    RestoringSelectedItem; 
};

#endif
