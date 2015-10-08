import qt

#-----------------------------------------------------------------------------
def _makeAction(parent, text, icon=None, shortcut=None, slot=None):
  action = qt.QAction(text, parent)

  if icon is not None:
    action.setIcon(qt.QIcon.fromTheme(icon))

  if shortcut is not None:
    action.shortcut = qt.QKeySequence.fromString(shortcut)
    action.shortcutContext = qt.Qt.WidgetWithChildrenShortcut

  if slot is not None:
    action.connect('triggered(bool)', slot)

  parent.addAction(action)

  return action

#-----------------------------------------------------------------------------
def _newItemPlaceholderItem(parent):
  palette = parent.palette
  color = qt.QColor(palette.text().color())
  color.setAlphaF(0.5)

  item = qt.QTreeWidgetItem()
  item.setText(0, "(New item)")
  item.setForeground(0, qt.QBrush(color))

  return item

#=============================================================================
#
# EditableTreeWidget
#
#=============================================================================
class EditableTreeWidget(qt.QTreeWidget):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    qt.QTreeWidget.__init__(self, *args, **kwargs)

    # Create initial placeholder item
    self._items = []
    self.addItem(_newItemPlaceholderItem(self), placeholder=True)

    # Set up context menu
    self._shiftUpAction = _makeAction(self, text="Move &Up",
                                      icon="arrow-up",
                                      shortcut="ctrl+shift+up",
                                      slot=self.shiftSelectionUp)

    self._shiftDownAction = _makeAction(self, text="Move &Down",
                                        icon="arrow-down",
                                        shortcut="ctrl+shift+down",
                                        slot=self.shiftSelectionDown)

    self._deleteAction = _makeAction(self, text="&Delete", icon="edit-delete",
                                     shortcut="del", slot=self.deleteSelection)

    self.contextMenuPolicy = qt.Qt.ActionsContextMenu

    # Connect internal slots
    self.connect('itemChanged(QTreeWidgetItem*,int)', self.updateItemData)
    self.connect('itemSelectionChanged()', self.updateActions)

  #---------------------------------------------------------------------------
  def addItem(self, item, placeholder=False):
    item.setFlags(item.flags() | qt.Qt.ItemIsEditable)

    if placeholder:
      self._items.append(item)
      self.addTopLevelItem(item)
    else:
      pos = len(self._items) - 1
      self._items.insert(pos, item)
      self.insertTopLevelItem(pos, item)

  #---------------------------------------------------------------------------
  def clear(self):
    # Delete all but placeholder item
    while len(self._items) > 1:
      del self._items[0]

  #---------------------------------------------------------------------------
  @property
  def itemCount(self):
    return self.topLevelItemCount - 1

  #---------------------------------------------------------------------------
  def selectedRows(self):
    placeholder = self._items[-1]
    items = self.selectedItems()
    return [self.indexOfTopLevelItem(i) for i in items if i is not placeholder]

  #---------------------------------------------------------------------------
  def setSelectedRows(self, rows):
    sm = self.selectionModel()
    sm.clear()

    for item in (self.topLevelItem(row) for row in rows):
      item.setSelected(True)

  #---------------------------------------------------------------------------
  def updateActions(self):
    placeholder = self._items[-1]

    items = self.selectedItems()
    rows = self.selectedRows()

    last = self.topLevelItemCount - 2

    self._shiftUpAction.enabled = len(rows) and not 0 in rows
    self._shiftDownAction.enabled = len(rows) and not last in rows
    self._deleteAction.enabled = True if len(rows) else False

  #---------------------------------------------------------------------------
  def updateItemData(self, item, column):
    # Create new placeholder item if edited item is current placeholder
    if item is self._items[-1]:
      self.addItem(_newItemPlaceholderItem(self), placeholder=True)

      # Remove placeholder effect from new item
      item.setData(0, qt.Qt.ForegroundRole, None)
      if column != 0:
        item.setText(0, "Anonymous")

      # Update actions so new item can be moved/deleted
      self.updateActions()

  #---------------------------------------------------------------------------
  def shiftSelection(self, delta):
    current = self.currentItem()

    rows = sorted(self.selectedRows())
    for row in rows if delta < 0 else reversed(rows):
      item = self.takeTopLevelItem(row)
      self._items.pop(row)
      self._items.insert(row + delta, item)
      self.insertTopLevelItem(row + delta, item)

    self.setSelectedRows((row + delta for row in rows))
    self.setCurrentItem(current)

  #---------------------------------------------------------------------------
  def shiftSelectionUp(self):
    self.shiftSelection(-1)

  #---------------------------------------------------------------------------
  def shiftSelectionDown(self):
    self.shiftSelection(+1)

  #---------------------------------------------------------------------------
  def deleteSelection(self):
    rows = self.selectedRows()
    for row in reversed(sorted(rows)):
      del self._items[row]
