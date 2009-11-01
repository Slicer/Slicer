#ifndef __qCTKAddRemoveComboBox_h
#define __qCTKAddRemoveComboBox_h

#include <QWidget>
#include <QVariant>

#include "qCTKWidgetsWin32Header.h"

class QComboBox; 

class QCTK_WIDGETS_EXPORT qCTKAddRemoveComboBox : public QWidget
{
  Q_OBJECT
  
public:
  // Superclass typedef
  typedef QWidget Superclass;
  
  // Constructors
  qCTKAddRemoveComboBox(QWidget* parent = 0);
  virtual ~qCTKAddRemoveComboBox();
  
  // Description:
  // Set content of description label. If set to Null or empty string, the label is hidden.
  void setDescription(const QString& text); 
  
  // Description:
  // Set text that should be displayed in the comboBox when it is empty
  void setEmptyText(const QString& text); 
  
  // Description:
  // Enable/Disable the edit button. 
  void setEditButtonEnabled(bool enable); 

  // Description:
  // Enable/Disable the add button. 
  void setAddButtonEnabled(bool enable); 
  
  // Description:
  // Add an item to the list. By default, notify is true, the signal 'itemAdded' is sent afterward.
  void addItem(const QString & text, const QVariant & userData = QVariant(), bool notify = true); 
  void addItemNoNotify(const QString & text, const QVariant & userData = QVariant()); 
  
  // Description:
  // Remove an item from the list. By default, notify is true, the signal 'itemRemoved' is sent afterward.
  void removeItem(const QString & text, bool notify = true); 
  void removeItemNoNotify(const QString & text);
  void removeItem(const QVariant & data, bool notify = true); 
  void removeItemNoNotify(const QVariant & data);  
  
  // Description:
  // Return the number of item
  int count()const;
  
  // Description:
  // Returns the index of the item containing the given text; otherwise returns -1.
  // The flags specify how the items in the combobox are searched.
  int findText(const QString& text, Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive ) const;

  // Description:
  // Return the current item
  QString selectedItemName()const; 
  QVariant selectedItemData()const;

signals:
  // Description:
  // This signal is sent when the user press the 'add' push button
  void addPushButtonPressed();
  
  // Description:
  // This signal is sent when the user press the 'add' push button
  void removePushButtonPressed(const QString & selectedItemName);
  void removePushButtonPressed(const QVariant & selectedItemData);
  
  // Description:
  // This signal is sent when the user select an item in the combo box
  void itemSelected(const QString & itemName); 
  void itemSelected(const QVariant & itemData); 
  
  // Description:
  // This signal is sent after the method 'addItem' has been called programmatically
  void itemAdded(const QString & itemName);
  void itemAdded(const QVariant & itemData);
  
  // Description:
  // This signal is sent after the method 'removeItem' or 'removeSelectedItem' 
  // has been called programmatically.
  // If the last item of the list is removed, isLastItem is True
  void itemRemoved(const QString & itemName, bool isLastItem);
  void itemRemoved(const QVariant & itemData, bool isLastItem);
  
  // Description:
  // This signal is sent after the last item has been removed
  bool lastItemRemoved(); 
  
  // Description:
  // This signal is sent after the user presses the 'edit' button
  void itemEditRequested(const QString & itemName); 
  void itemEditRequested(const QVariant & itemData); 

public slots:
  // Description:
  // Remove the item currently selected. See signal 'itemRemoved'
  void removeSelectedItem();
  
  // Description:
  // Update the item of the selected Item
  void updateSelectedItemName(const QString& newItemName); 
  
  // Description:
  // Update the name of the item matching itemData
  void updateItemName(const QVariant& itemData, const QString& newItemName); 
  
  // Description:
  // Update the name of the item matching itemName
  void updateItemName(const QString& itemName, const QString& newItemName); 

  // Description:
  // Select the current index
  void setCurrentIndex(int index);
  
protected slots:
  void onEditPushButtonPressed(); 
  void onRemovePushButtonPressed(); 

  // Description:
  // Remove an item from the list. By default, No signal are sent.
  void removeItem(int index, bool notify = false); 

private:
  struct qInternal; 
  qInternal * Internal;

};

#endif
