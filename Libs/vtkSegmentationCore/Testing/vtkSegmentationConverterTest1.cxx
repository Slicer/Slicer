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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// SegmentationCore includes
#include <vtkSegmentationConverter.h>
#include <vtkSegmentationConverterFactory.h>
#include <vtkSegmentationConverterRule.h>

//----------------------------------------------------------------------------
// Test macros
#define VERIFY_EQUAL(description, actual, expected) \
{ \
  if (expected != actual) \
    { \
    std::cerr << "Test failure: Mismatch in " << description << ". Expected " << expected << ", actual value is " << actual << std::endl << std::endl; \
    exit(EXIT_FAILURE); \
    } \
  else \
    { \
    std::cout << "Test case success: " << description << " matches expected value " << actual << std::endl; \
    } \
}

//----------------------------------------------------------------------------
// Conversion graph

// Convenience macro for defining a converter rule class with a single line
#define RULE(from, to, weight) \
class vtkRep##from##ToRep##to##Rule: public vtkSegmentationConverterRule \
{ \
public: \
  static vtkRep##from##ToRep##to##Rule* New(); \
  vtkTypeMacro(vtkRep##from##ToRep##to##Rule, vtkSegmentationConverterRule); \
  virtual vtkSegmentationConverterRule* CreateRuleInstance() override; \
  virtual vtkDataObject* ConstructRepresentationObjectByRepresentation( \
    std::string vtkNotUsed(representationName))  override { return nullptr; }; \
  virtual vtkDataObject* ConstructRepresentationObjectByClass( \
    std::string vtkNotUsed(className)) override { return nullptr; }; \
  virtual bool Convert( \
    vtkSegment* vtkNotUsed(segment)) override { return true; } \
  virtual unsigned int GetConversionCost( \
    vtkDataObject* sourceRepresentation=nullptr, \
    vtkDataObject* targetRepresentation=nullptr)  override \
  { \
    (void)sourceRepresentation; \
    (void)targetRepresentation; \
    return weight; \
  }; \
  virtual const char* GetName() override { return "Rep " #from " to Rep " #to; } \
  virtual const char* GetSourceRepresentationName() override { return "Rep" #from ; }  \
  virtual const char* GetTargetRepresentationName()  override { return "Rep" #to ; } \
}; \
vtkSegmentationConverterRuleNewMacro(vtkRep##from##ToRep##to##Rule);

/*
Test conversion graph:
  A, B, ... = vertex = data representation type
  --4>--    = edge (with weight of 4) = converter rule

  2>--B-1>--C
 /   /     / \
A--<3   -<2   4>--E
 \     /         / \
  6>--D---2>-----   \
      \              \
       -------<1------

*/
// Converter rules corresponding to the test conversion graph above
RULE(A, B, 2);
RULE(A, D, 6);
RULE(B, A, 3);
RULE(B, C, 1);
RULE(C, D, 2);
RULE(C, E, 4);
RULE(D, E, 2);
RULE(E, D, 1);

void PrintPath(const vtkSegmentationConverter::ConversionPathType& path)
{
  for (vtkSegmentationConverter::ConversionPathType::const_iterator ruleIt = path.begin(); ruleIt != path.end(); ++ruleIt)
    {
    std::cout << "      " << (*ruleIt)->GetName() << "(" << (*ruleIt)->GetConversionCost() << ")" << std::endl;
    }
}

//----------------------------------------------------------------------------
void TestRegisterUnregister()
{
  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();

  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepBToRepARule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepBToRepCRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepCToRepERule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepCToRepERule>::New());
  VERIFY_EQUAL("number of rules after register", converterFactory->GetConverterRules().size(), 4);

  // Remove one
  converterFactory->UnregisterConverterRule(converterFactory->GetConverterRules()[3]);
  VERIFY_EQUAL("number of rules after unregister", converterFactory->GetConverterRules().size(), 3);

  // Remove all
  while (converterFactory->GetConverterRules().size()>0)
    {
    converterFactory->UnregisterConverterRule(converterFactory->GetConverterRules()[0]);
    }
  VERIFY_EQUAL("number of rules after unregister", converterFactory->GetConverterRules().size(), 0);
}

//----------------------------------------------------------------------------
int vtkSegmentationConverterTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  TestRegisterUnregister();

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepAToRepBRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepAToRepDRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepBToRepARule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepBToRepCRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepCToRepDRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepCToRepERule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepDToRepERule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkRepEToRepDRule>::New());

  vtkSmartPointer<vtkSegmentationConverter> converter = vtkSmartPointer<vtkSegmentationConverter>::New();

  vtkSegmentationConverter::ConversionPathAndCostListType pathsCosts;
  vtkSegmentationConverter::ConversionPathType shortestPath;

  // A->E paths: ABCE, ABCDE, ADE
  std::cout << "Conversion from RepA to RepE" << std::endl;
  std::cout << "  All paths:" << std::endl;
  converter->GetPossibleConversions("RepA", "RepE", pathsCosts);
  for (vtkSegmentationConverter::ConversionPathAndCostListType::iterator pathsCostsIt = pathsCosts.begin(); pathsCostsIt != pathsCosts.end(); ++pathsCostsIt)
    {
    std::cout << "    Path: (total cost = " << pathsCostsIt->second << ")" << std::endl;
    PrintPath(pathsCostsIt->first);
    }
  VERIFY_EQUAL("number of paths from representation A to E", pathsCosts.size(), 3);
  std::cout << "  Cheapest path:" << std::endl;
  shortestPath = vtkSegmentationConverter::GetCheapestPath(pathsCosts);
  PrintPath(shortestPath);
  VERIFY_EQUAL("number of paths from representation A to E", shortestPath.size(), 3);

  // E->A paths: none
  std::cout << "Conversion from RepE to RepA" << std::endl;
  converter->GetPossibleConversions("RepE", "RepA", pathsCosts);
  VERIFY_EQUAL("number of paths from representation E to A", pathsCosts.size(), 0);

  // B->D paths: BAD, BCD, BCED
  std::cout << "Conversion from RepB to RepD" << std::endl;
  std::cout << "  All paths:" << std::endl;
  converter->GetPossibleConversions("RepB", "RepD", pathsCosts);
  for (vtkSegmentationConverter::ConversionPathAndCostListType::iterator pathsCostsIt = pathsCosts.begin(); pathsCostsIt != pathsCosts.end(); ++pathsCostsIt)
    {
    std::cout << "    Path: (total cost = " << pathsCostsIt->second << ")" << std::endl;
    PrintPath(pathsCostsIt->first);
    }
  VERIFY_EQUAL("number of paths from representation B to D", pathsCosts.size(), 3);
  std::cout << "  Cheapest path:" << std::endl;
  shortestPath = vtkSegmentationConverter::GetCheapestPath(pathsCosts);
  PrintPath(shortestPath);
  VERIFY_EQUAL("number of paths from representation B to D", shortestPath.size(), 2);

  // C->D paths: CD, CED
  std::cout << "Conversion from RepC to RepD" << std::endl;
  std::cout << "  All paths:" << std::endl;
  converter->GetPossibleConversions("RepC", "RepD", pathsCosts);
  for (vtkSegmentationConverter::ConversionPathAndCostListType::iterator pathsCostsIt = pathsCosts.begin(); pathsCostsIt != pathsCosts.end(); ++pathsCostsIt)
    {
    std::cout << "    Path: (total cost = " << pathsCostsIt->second << ")" << std::endl;
    PrintPath(pathsCostsIt->first);
    }
  VERIFY_EQUAL("number of paths from representation C to D", pathsCosts.size(), 2);
  std::cout << "  Cheapest path:" << std::endl;
  shortestPath = vtkSegmentationConverter::GetCheapestPath(pathsCosts);
  PrintPath(shortestPath);
  VERIFY_EQUAL("number of paths from representation C to D", shortestPath.size(), 1);

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
