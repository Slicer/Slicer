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

// STL includes
#include <stdlib.h>
#include <iostream>

//-----------------------------------------------------------------------------
QCTK_DECLARE_TEST(qCTKAddRemoveComboBoxTest1)
{
  qCTKAddRemoveComboBox qctkObject;
  
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
    
  QCTK_EXIT_TEST(EXIT_SUCCESS);
}

QCTK_RUN_TEST(qCTKAddRemoveComboBoxTest1);
