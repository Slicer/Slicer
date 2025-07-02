/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkDataFileFormatHelper.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

namespace
{
enum TestReadReferenceType
{
  NullptrAsReference,
  TransformNodeAsReference,
  ModelNodeAsReference
};
}

//---------------------------------------------------------------------------
class vtkMRMLStorageNodeTestHelper1 : public vtkMRMLStorageNode
{
public:
  // Provide a concrete New.
  static vtkMRMLStorageNodeTestHelper1* New();

  vtkTypeMacro(vtkMRMLStorageNodeTestHelper1, vtkMRMLStorageNode);

  vtkMRMLNode* CreateNodeInstance() override { return vtkMRMLStorageNodeTestHelper1::New(); }
  const char* GetNodeTagName() override { return "vtkMRMLStorageNodeTestHelper1"; }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }

  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override { return refNode->IsA(this->SupportedClass); }
  int ReadDataInternal(vtkMRMLNode* vtkNotUsed(refNode)) override { return this->ReadDataReturnValue; }

  const char* SupportedClass{ nullptr };
  int ReadDataReturnValue{ 0 };

protected:
  vtkMRMLStorageNodeTestHelper1() = default;
};
vtkStandardNewMacro(vtkMRMLStorageNodeTestHelper1);

//---------------------------------------------------------------------------
int TestBasics();
int TestReadData();
int TestWriteData();
int TestExtensionFormatHelper();

//---------------------------------------------------------------------------
int vtkMRMLStorageNodeTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestBasics());
  CHECK_EXIT_SUCCESS(TestReadData());
  CHECK_EXIT_SUCCESS(TestWriteData());
  CHECK_EXIT_SUCCESS(TestExtensionFormatHelper());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestBasics()
{
  vtkNew<vtkMRMLStorageNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadData(TestReadReferenceType referenceNodeType,
                 const char* supportedClass,
                 int readDataReturn,
                 int expectedRes)
{
  vtkNew<vtkMRMLStorageNodeTestHelper1> storageNode;
  storageNode->SupportedClass = supportedClass;
  storageNode->ReadDataReturnValue = readDataReturn;
  storageNode->SetFileName("file.ext");
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  vtkNew<vtkMRMLModelNode> modelNode;
  vtkMRMLNode* referenceNode = nullptr;
  switch (referenceNodeType)
  {
    case NullptrAsReference:
      referenceNode = nullptr;
      break;
    case TransformNodeAsReference:
      referenceNode = transformNode;
      break;
    case ModelNodeAsReference:
      referenceNode = modelNode;
      break;
  }
  int res = storageNode->ReadData(referenceNode);
  std::cout << "StoredTime: " << storageNode->GetStoredTime() << std::endl;
  CHECK_INT(res, expectedRes);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadData()
{
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(NullptrAsReference, "invalid", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(NullptrAsReference, "invalid", /*readDataResult=*/1, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(NullptrAsReference, "vtkMRMLModelNode", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(NullptrAsReference, "vtkMRMLModelNode", /*readDataResult=*/1, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(TransformNodeAsReference, "invalid", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(TransformNodeAsReference, "invalid", /*readDataResult=*/1, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(TransformNodeAsReference, "vtkMRMLModelNode", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(TransformNodeAsReference, "vtkMRMLModelNode", /*readDataResult=*/1, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(ModelNodeAsReference, "invalid", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(ModelNodeAsReference, "invalid", /*readDataResult=*/1, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_EXIT_SUCCESS(TestReadData(ModelNodeAsReference, "vtkMRMLModelNode", /*readDataResult=*/0, /*success=*/0));
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_EXIT_SUCCESS(TestReadData(ModelNodeAsReference, "vtkMRMLModelNode", /*readDataResult=*/1, /*success=*/1));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestWriteData()
{
  // TODO
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestExtensionFormatHelper()
{
  vtkNew<vtkDataFileFormatHelper> helper;

  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("VTK File (.vtk)"), ".vtk");
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("Segmentation (.seg.nrrd)"), ".seg.nrrd");
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("This is a NRRD (.nrrd)"), ".nrrd");
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("Nifti-file (.nii.gz)"), ".nii.gz");
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("Any file (.*)"), ".*");
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString("foo"), "");

  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  CHECK_STD_STRING(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(".vtk"), ".vtk");
  TESTING_OUTPUT_ASSERT_WARNINGS(1);
  TESTING_OUTPUT_ASSERT_WARNINGS_END();

  return EXIT_SUCCESS;
}
