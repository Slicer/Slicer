#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationTextDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationTextDisplayNode> node1;

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

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

  TEST_SET_GET_UNSIGNED_INT_RANGE(node1, MaxCharactersPerLine, 1, 100);

  // test the line wrapping

  node1->UseLineWrapOn();

  node1->SetMaxCharactersPerLine(3);
  std::string in1 = std::string("1 2 3 4 5 6");
  std::string exp1 = std::string("1 2\n3 4\n5 6");
  std::cout << "\nTesting input string '" << in1.c_str() << "'" << std::endl;
  std::string out1 = node1->GetLineWrappedText(in1);
  if (out1.compare(exp1) != 0)
    {
    std::cerr << "Line wrapping failed! With max " << node1->GetMaxCharactersPerLine() << " characters per line:\n\tinput =\n'" << in1.c_str() << "'\n\toutput =\n'" << out1.c_str() << "'\n\texpected =\n'" << exp1.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Success: wrapped string:\n'" << out1.c_str() << "'" << std::endl;

  std::string in2 = std::string("1234567");
  std::string exp2 = std::string("123\n456\n7");
  std::cout << "\nTesting input string '" << in2.c_str() << "'" << std::endl;
  std::string out2 = node1->GetLineWrappedText(in2);
  if (out2.compare(exp2) != 0)
    {
    std::cerr << "Line wrapping failed! With max " << node1->GetMaxCharactersPerLine() << " characters per line:\n\tinput =\n'" << in2.c_str() << "'\n\toutput =\n'" << out2.c_str() << "'\n\texpected =\n'" << exp2.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Success: wrapped string:\n'" << out2.c_str() << "'" << std::endl;


  node1->SetMaxCharactersPerLine(10);
  std::string in3 = std::string("This is a test string with a better text example verylongword last");
  std::string exp3 = std::string("This is a\ntest\nstring\nwith a\nbetter\ntext\nexample\nverylongwo\nrd last");
//  std::string exp3 = std::string("This is a\ntest\nstring\nwith a\nbetter\ntext\nexample\nverylongword\nlast");
  std::cout << "\nTesting input string '" << in3.c_str() << "'" << std::endl;
  std::string out3 = node1->GetLineWrappedText(in3);
  if (out3.compare(exp3) != 0)
    {
    std::cerr << "Line wrapping failed! With max " << node1->GetMaxCharactersPerLine() << " characters per line:\n\tinput =\n'" << in3.c_str() << "'\n\toutput =\n'" << out3.c_str() << "'\n\texpected =\n'" << exp3.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Success: wrapped string:\n'" << out3.c_str() << "'" << std::endl;

  return EXIT_SUCCESS;
}
