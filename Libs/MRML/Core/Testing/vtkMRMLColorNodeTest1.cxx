/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorNode.h"
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

/// \brief Concrete implementation of vtkMRMLColorNode
class vtkMRMLColorNodeTestHelper1 : public vtkMRMLColorNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLColorNodeTestHelper1,vtkMRMLColorNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLColorNodeTestHelper1::New();
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLColorNodeTestHelper1";
    }

  virtual int ReadFile()
    {
    std::cout << "vtkMRMLColorNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }
  virtual int GetNumberOfColors(){return 1;}
  virtual bool GetColor(int vtkNotUsed(ind), double color[4])
    {
    color[0] = 10;
    color[1] = 100;
    color[2] = 200;
    return true;
    }
};
vtkStandardNewMacro(vtkMRMLColorNodeTestHelper1);

//---------------------------------------------------------------------------
bool TestGetColorNameAsFileName();
bool TestGetColorNameAsFileName(const char* colorName,
                                const char* expectedColorFileName,
                                const char* substr = "_");

//---------------------------------------------------------------------------
int vtkMRMLColorNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLColorNodeTestHelper1> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( vtkMRMLColorNodeTestHelper1, node1.GetPointer());

  bool res = true;
  res = TestGetColorNameAsFileName() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool TestGetColorNameAsFileName()
{
  bool res = true;
  res = TestGetColorNameAsFileName("validName", "validName") && res;
  res = TestGetColorNameAsFileName("name with space", "name_with_space") && res;
  res = TestGetColorNameAsFileName("n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars", "n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars") && res;
  res = TestGetColorNameAsFileName("n`a@m&e*w+i=t[h]i;n:v\\a|l\"i<D>c,h/a?rs", "n_a_m_e_w_i_t_h_i_n_v_a_l_i_D_c_h_a_rs") && res;
  res = TestGetColorNameAsFileName("name with \nreturn", "name_with__return") && res;
  res = TestGetColorNameAsFileName("ÑÂme wïth àçÇénts", "____me_w__th_________nts") && res;
  res = TestGetColorNameAsFileName(
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very long name",
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_v") && res;
  res = TestGetColorNameAsFileName("name with space", "name with space", " ") && res;
  res = TestGetColorNameAsFileName("name with space", "name__with__space", "__") && res;
  return res;
}

//---------------------------------------------------------------------------
bool TestGetColorNameAsFileName(const char* colorName, const char * expectedColorFileName, const char* substr)
{
  vtkNew<vtkMRMLColorNodeTestHelper1> colorNode;
  colorNode->SetNamesFromColors();
  colorNode->SetColorName(0, colorName);
  std::string fileName = colorNode->GetColorNameAsFileName(0, substr);
  bool res = (fileName == std::string(expectedColorFileName));
  if (!res)
    {
    std::cout << "Input: " << colorName
              << "\nOutput: " << fileName.c_str()
              << "\n Expected: " << expectedColorFileName <<std::endl;
    }
  return res;
}
