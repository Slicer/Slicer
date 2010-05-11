/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// qCTK includes
#include "qCTKAddRemoveComboBox.h"
#include "qCTKTestApplication.h"

// QT includes
#include <QSignalSpy>

// STD includes
#include <stdlib.h>
#include <iostream>

namespace
{
class qCTKAddRemoveComboBoxWrapper : public qCTKAddRemoveComboBox
{
public:
  qCTKAddRemoveComboBoxWrapper(QWidget* _parent = 0):qCTKAddRemoveComboBox(_parent)
    {
    this->resetTestFlags();
    }
  void resetTestFlags()
    {
    this->OnAddFunctionCallCount = 0;
    this->OnRemoveFunctionCallCount = 0;
    this->OnEditFunctionCallCount = 0; 
    }
  virtual void onAdd()
    {
    this->OnAddFunctionCallCount++; 
    }
  virtual void onRemove()
    {
    this->OnRemoveFunctionCallCount++; 
    }
  virtual void onEdit()
    {
    this->OnEditFunctionCallCount++; 
    }
  int OnAddFunctionCallCount;
  int OnRemoveFunctionCallCount;
  int OnEditFunctionCallCount;
};
}

//-----------------------------------------------------------------------------
QCTK_DECLARE_TEST(qCTKAddRemoveComboBoxTest1)
{
  qCTKAddRemoveComboBoxWrapper qctkObject;

  // Enable pushButtons
  qctkObject.setPushButtonsEnabled(true);
  
  //
  // Test case when PushButtonEnabled = True
  //

  if (!qctkObject.pushButtonsEnabled())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in pushButtonsEnabled() - Expected to be True" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
  
  int currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  // Add items
  qctkObject.addItem("Item1");
  qctkObject.addItem("Item2");
  qctkObject.addItem("Item3");

  currentCount = qctkObject.count();
  if (currentCount != 3)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 3, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "Item1" ||
      qctkObject.itemText(1) != "Item2" ||
      qctkObject.itemText(2) != "Item3")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [Item1, Item2, Item3]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << ", "
                                    << qPrintable(qctkObject.itemText(2)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.removeItem(1);

  currentCount = qctkObject.count();
  if (currentCount != 2)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 2, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "Item1" ||
      qctkObject.itemText(1) != "Item3")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [Item1, Item3]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.addItem("Item4");

  currentCount = qctkObject.count();
  if (currentCount != 3)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 3, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "Item1" ||
      qctkObject.itemText(1) != "Item3" ||
      qctkObject.itemText(2) != "Item4")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [Item1, Item3, Item4]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << ", "
                                    << qPrintable(qctkObject.itemText(2)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.clear();

  currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.addItem("Item5");

  currentCount = qctkObject.count();
  if (currentCount != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 1, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "Item5")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [Item5]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.clear();

  currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
  
  //
  // Test case when PushButtonEnabled = False
  //
  
  qctkObject.setPushButtonsEnabled(false);

  if (qctkObject.pushButtonsEnabled())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in pushButtonsEnabled() - Expected to be False" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
  
  // count should return 0
  currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  // Let's check if the combobox contains the expected element
  QString currentEmptyItemText = qctkObject.itemText(0); // EmptyItem
  QString currentSeparatorText = qctkObject.itemText(1); // Separator
  QString currentAddItemText = qctkObject.itemText(2); // Add item
  QString currentRemoveItemText = qctkObject.itemText(3); // Remove Item
  QString currentEditItemText = qctkObject.itemText(4); // Edit Item

  if (currentEmptyItemText != "None")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in clear()" << std::endl
                          << " Expected item[0] should be empty"<< std::endl
                          << " Current item[0] => " << qPrintable(currentEmptyItemText) << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (!currentSeparatorText.isEmpty())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in clear()" << std::endl
                          << " Expected item[1] should be empty"<< std::endl
                          << " Current item[1] => " << qPrintable(currentSeparatorText) << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (currentAddItemText != qctkObject.addText())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in clear()" << std::endl
                          << " Expected item[2] => " << qPrintable(qctkObject.addText()) << std::endl
                          << " Current item[2] => " << qPrintable(currentAddItemText) << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (currentRemoveItemText != qctkObject.removeText())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in clear()" << std::endl
                          << " Expected item[3] => " << qPrintable(qctkObject.removeText()) << std::endl
                          << " Current item[3] => " << qPrintable(currentRemoveItemText) << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (currentEditItemText != qctkObject.editText())
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in clear()" << std::endl
                          << " Expected item[4] => " << qPrintable(qctkObject.editText()) << std::endl
                          << " Current item[4] => " << qPrintable(currentEditItemText) << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  // Add items
  qctkObject.addItem("ItemA");
  qctkObject.addItem("ItemB");
  qctkObject.addItem("ItemC");

  currentCount = qctkObject.count();
  if (currentCount != 3)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 3, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "ItemA" ||
      qctkObject.itemText(1) != "ItemB" ||
      qctkObject.itemText(2) != "ItemC")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [ItemA, ItemB, ItemC]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << ", "
                                    << qPrintable(qctkObject.itemText(2)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.removeItem(1);

  currentCount = qctkObject.count();
  if (currentCount != 2)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 2, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "ItemA" ||
      qctkObject.itemText(1) != "ItemC")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [ItemA, ItemC]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.addItem("ItemD");

  currentCount = qctkObject.count();
  if (currentCount != 3)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 3, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "ItemA" ||
      qctkObject.itemText(1) != "ItemC" ||
      qctkObject.itemText(2) != "ItemD")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [ItemA, ItemC, ItemD]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << ", "
                                    << qPrintable(qctkObject.itemText(1)) << ", "
                                    << qPrintable(qctkObject.itemText(2)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.clear();

  currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.addItem("ItemE");

  currentCount = qctkObject.count();
  if (currentCount != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 1, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.itemText(0) != "ItemE")
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in itemText()" << std::endl
              << " Expected items [ItemE]" << std::endl
              << " Current items [" << qPrintable(qctkObject.itemText(0)) << "]" << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  qctkObject.clear();

  currentCount = qctkObject.count();
  if (currentCount != 0)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  QCTK_EXIT_TEST(EXIT_SUCCESS);

  //
  // Test case when PushButtonEnabled = False, let's now make sure the appropriate signals are sent
  // when either an action item or a regular item is selected.
  //

  // Add few items
  qctkObject.addItem("Testing");
  qctkObject.addItem("is");
  qctkObject.addItem("good");

  QSignalSpy spy(&qctkObject, SIGNAL(currentIndexChanged(int)));
  qctkObject.setCurrentIndex(2);
  qctkObject.setCurrentIndex(0);
  qctkObject.setCurrentIndex(1);
  qctkObject.setCurrentIndex(3); // Select separator ?
  qctkObject.setCurrentIndex(4); // Select Add
  qctkObject.setCurrentIndex(5); // Select Remove
  qctkObject.setCurrentIndex(6); // Select Edit
  if (spy.count() != 3)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error with signal currentIndexChanged(int)" << std::endl
              << " Expected: 3, current:" << spy.count() << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
  if (qctkObject.OnAddFunctionCallCount != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error with onAdd() function" << std::endl
              << " Expected: 1, current:" << qctkObject.OnAddFunctionCallCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (qctkObject.OnRemoveFunctionCallCount != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error with onRemove() function" << std::endl
              << " Expected: 1, current:" << qctkObject.OnRemoveFunctionCallCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
    
  if (qctkObject.OnEditFunctionCallCount != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error with onEdit() function" << std::endl
              << " Expected: 1, current:" << qctkObject.OnEditFunctionCallCount << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }

  if (qctkObject.currentIndex() != 1)
    {
    qctkObject.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error with currentIndex()" << std::endl
              << " Expected: 1, current:" << qctkObject.currentIndex() << std::endl;
    QCTK_EXIT_TEST(EXIT_FAILURE);
    }
}

QCTK_RUN_TEST(qCTKAddRemoveComboBoxTest1);
