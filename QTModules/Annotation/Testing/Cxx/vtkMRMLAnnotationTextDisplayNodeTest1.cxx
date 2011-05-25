#include "vtkMRMLAnnotationTextDisplayNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLAnnotationTextDisplayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLAnnotationTextDisplayNode > node1 = vtkSmartPointer< vtkMRMLAnnotationTextDisplayNode >::New();

  EXERCISE_BASIC_DISPLAY_MRML_METHODS(vtkMRMLAnnotationTextDisplayNode, node1);
 
  TEST_SET_GET_BOOLEAN(node1, UseLineWrap);
  TEST_SET_GET_BOOLEAN(node1, ShowBorder);
  TEST_SET_GET_BOOLEAN(node1, ShowLeader);
  TEST_SET_GET_BOOLEAN(node1, UseThreeDimensionalLeader);
  TEST_SET_GET_BOOLEAN(node1, ShowArrowHead);

  // LeaderGlyphSize, MaximumLeaderGlyphSize, Padding are clamped, so test manually

  TEST_SET_GET_DOUBLE(node1, LeaderGlyphSize, 0.0);
  TEST_SET_GET_DOUBLE(node1, LeaderGlyphSize, 0.1);
  TEST_SET_GET_DOUBLE(node1, LeaderGlyphSize, 0.05);


  TEST_SET_GET_INT_RANGE(node1, MaximumLeaderGlyphSize, 2, 999);
  TEST_SET_GET_INT_RANGE(node1, Padding, 1, 49);

  TEST_SET_GET_BOOLEAN(node1, AttachEdgeOnly);

  TEST_SET_GET_INT_RANGE(node1, MaxCharactersPerLine, 1, 100);

  return EXIT_SUCCESS;
}
