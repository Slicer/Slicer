/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorTableStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML include
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTableStorageNode.h"

// VTK include
#include <vtkDelimitedTextReader.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// STD include
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLColorTableStorageNode);

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::vtkMRMLColorTableStorageNode()
{
  // When a color table file contains very large numbers then most likely
  // it is not a valid file (probably it is some other text file and not
  // a color table). The highest acceptable color ID is specified in MaximumColorID.
  this->MaximumColorID = 1000000;
  this->DefaultWriteFileExtension = "csv";
}

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::~vtkMRMLColorTableStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLColorTableStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLColorTableNode");
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "MRML Color Table") + " (.ctbl)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "MRML Color Table") + " (.txt)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "Comma-separated values") + " (.csv)");
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "MRML Color Table") + " (.ctbl)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "MRML Color Table") + " (.txt)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLColorTableStorageNode", "Comma-separated values") + " (.csv)");
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  // cast the input node
  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(refNode);
  if (colorNode == nullptr)
  {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a known color table node");
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadDataInternal",
      "Filename is not specified (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadDataInternal",
      "Color table file '" << fullName.c_str() << "' is not found while trying to read node (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadDataInternal",
      "Color table file '" << fullName.c_str() << "' has no file extension while trying to read node (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  vtkDebugMacro("ReadDataInternal (" << (this->ID ? this->ID : "(unknown)") << "): extension = " << extension.c_str());

  if (extension == std::string(".ctbl") || extension == std::string(".txt"))
  {
    return this->ReadCtblFile(fullName, colorNode);
  }
  else if (extension == std::string(".csv") || extension == std::string(".tsv"))
  {
    return this->ReadCsvFile(fullName, colorNode);
  }
  else
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadDataInternal",
      "Color table file '" << fullName.c_str() << "' has unknown file extension while trying to read node (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorTableStorageNode::GetFieldDelimiterCharacters(std::string filename)
{
  std::string lowercaseFileExt = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(filename);
  std::string fieldDelimiterCharacters;
  if (lowercaseFileExt == std::string(".tsv") || lowercaseFileExt == std::string(".txt"))
  {
    fieldDelimiterCharacters = "\t";
  }
  else if (lowercaseFileExt == std::string(".csv"))
  {
    fieldDelimiterCharacters = ",";
  }
  else
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::GetFieldDelimiterCharacters",
      "Cannot determine field delimiter character from file extension: '" << lowercaseFileExt << "'.");
  }
  return fieldDelimiterCharacters;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::ReadCsvFile(std::string fullFileName, vtkMRMLColorTableNode* colorNode)
{
  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(fullFileName.c_str());
  reader->SetHaveHeaders(true);
  reader->SetFieldDelimiterCharacters(this->GetFieldDelimiterCharacters(fullFileName).c_str());
  // Make sure string delimiter characters are removed (somebody may have written a tsv with string delimiters)
  reader->SetUseStringDelimiter(true);
  // File contents is preserved better if we don't try to detect numeric columns
  reader->DetectNumericColumnsOff();

  // Read table
  vtkTable* table = nullptr;
  try
  {
    reader->Update();
    table = reader->GetOutput();
  }
  catch (...)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Failed to read color table file: '" << fullFileName << "'.");
    return 0;
  }

  if (!table)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Failed to read color table file: '" << fullFileName << "'.");
    return 0;
  }

  vtkStringArray* labelValueColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("LabelValue"));
  if (!labelValueColumn)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Failed to read color table file: '" << fullFileName << "' due to missing 'LabelValue' column.");
    return 0;
  }

  vtkStringArray* nameColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Name"));
  if (!nameColumn)
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Missing 'Name' column in color table file: '" << fullFileName << "'.");
  }

  vtkStringArray* colorRColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Color_R"));
  if (!colorRColumn)
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Missing 'Color_R' column in color table file: '" << fullFileName << "'.");
  }
  vtkStringArray* colorGColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Color_G"));
  if (!colorGColumn)
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Missing 'Color_G' column in color table file: '" << fullFileName << "'.");
  }
  vtkStringArray* colorBColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Color_B"));
  if (!colorBColumn)
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Missing 'Color_B' column in color table file: '" << fullFileName << "'.");
  }
  vtkStringArray* colorAColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Color_A"));
  if (!colorAColumn)
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
      "Missing 'Color_A' column in color table file: '" << fullFileName << "'.");
  }

  vtkStringArray* categoryCSColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Category.CodingSchemeDesignator"));
  vtkStringArray* categoryCVColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Category.CodeValue"));
  vtkStringArray* categoryCMColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Category.CodeMeaning"));

  vtkStringArray* typeCSColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Type.CodingSchemeDesignator"));
  vtkStringArray* typeCVColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Type.CodeValue"));
  vtkStringArray* typeCMColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("Type.CodeMeaning"));
  vtkStringArray* typeModifierCSColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("TypeModifier.CodingSchemeDesignator"));
  vtkStringArray* typeModifierCVColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("TypeModifier.CodeValue"));
  vtkStringArray* typeModifierCMColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("TypeModifier.CodeMeaning"));

  vtkStringArray* anatomicRegionCSColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegion.CodingSchemeDesignator"));
  vtkStringArray* anatomicRegionCVColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegion.CodeValue"));
  vtkStringArray* anatomicRegionCMColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegion.CodeMeaning"));
  vtkStringArray* anatomicRegionModifierCSColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegionModifier.CodingSchemeDesignator"));
  vtkStringArray* anatomicRegionModifierCVColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegionModifier.CodeValue"));
  vtkStringArray* anatomicRegionModifierCMColumn = vtkStringArray::SafeDownCast(table->GetColumnByName("AnatomicRegionModifier.CodeMeaning"));

  // clear out the table
  MRMLNodeModifyBlocker blocker(colorNode);

  // Set type to "File" by default if it has not been set yet.
  // It is important to only change type if it has not been set already
  // because otherwise "User" color node types would be always reverted to
  // read-only "File" type when the scene is saved and reloaded.
  if (colorNode->GetType()<colorNode->GetFirstType()
    || colorNode->GetType()>colorNode->GetLastType())
  {
    // no valid type has been set, set it to File
    colorNode->SetTypeToFile();
  }

  int numberOfTuples = labelValueColumn->GetNumberOfTuples();
  bool valid = false;
  int maxLabelValue = -1;
  std::vector<vtkIdType> validLabelValues(numberOfTuples, this->MaximumColorID + 1);
  for (vtkIdType row = 0; row < numberOfTuples; ++row)
  {
    if (labelValueColumn->GetValue(row).empty())
    {
      vtkWarningToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        "labelValue is not specified in line " << row << " - skipping this line.");
      continue;
    }
    int labelValue = labelValueColumn->GetVariantValue(row).ToInt(&valid);
    if (!valid)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        "labelValue '" << labelValueColumn->GetValue(row) << "' is invalid in line " << row << " - skipping this line.");
      continue;
    }
    if (labelValue < 0)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        "labelValue '" << labelValueColumn->GetValue(row) << "' is invalid, the value must be positive, in line " << row << " - skipping this line.");
      continue;
    }
    validLabelValues[row] = labelValue;
    if (labelValue > maxLabelValue)
    {
      maxLabelValue = labelValue;
    }
    if (maxLabelValue > this->MaximumColorID)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLColorTableStorageNode",
          "Cannot read '%1' file as a volume of type '%2'. Label values must not go above %3."),
          fullFileName.c_str(),
          colorNode ? colorNode->GetNodeTagName() : "",
          std::to_string(this->MaximumColorID).c_str()));
      colorNode->SetNumberOfColors(0);
      return 0;
    }
    if (maxLabelValue >= colorNode->GetNumberOfColors())
    {
      // we add 1 to the maximum label value because first color's ID is 1 (ID=0 is reserved for background)
      colorNode->SetNumberOfColors(maxLabelValue + 1);
    }
  }

  if (colorNode->GetLookupTable())
  {
    colorNode->GetLookupTable()->SetTableRange(0, maxLabelValue);
  }
  // init the table to black/opacity 0 with no name, just in case we're missing values
  colorNode->SetColors(0, maxLabelValue, colorNode->GetNoName(), 0.0, 0.0, 0.0, 0.0);

  // do a little sanity check, if never get an rgb bigger than 1.0, report
  // it as a possibly miswritten file
  for (vtkIdType row = 0; row < numberOfTuples; ++row)
  {
    if (validLabelValues[row] == this->MaximumColorID + 1)
    {
      // No valid label value in this row
      continue;
    }

    bool allValid = true;
    double r = colorRColumn->GetVariantValue(row).ToInt(&valid); allValid &= valid;
    double g = colorGColumn->GetVariantValue(row).ToInt(&valid); allValid &= valid;
    double b = colorBColumn->GetVariantValue(row).ToInt(&valid); allValid &= valid;
    double a = colorAColumn->GetVariantValue(row).ToInt(&valid); allValid &= valid;
    if (!allValid)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLColorTableStorageNode",
          "Failed to parse color values ('%1', '%2', '%3', '%4') in line %5"), std::to_string(r).c_str(),
          std::to_string(g).c_str(), std::to_string(b).c_str(), std::to_string(a).c_str(), std::to_string(row).c_str()));
      continue;
    }

    r /= 255.0; g /= 255.0; b /= 255.0; a /= 255.0;
    if (colorNode->SetColor(validLabelValues[row], nameColumn->GetValue(row).c_str(), r, g, b, a) == 0)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::ReadCsvFile",
        vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLColorTableStorageNode",
          "Unable to set color '%1' with name '%2', breaking the loop over '%3' lines in the file %4."),
          std::to_string(validLabelValues[row]).c_str(), nameColumn->GetValue(row).c_str(),
          std::to_string(numberOfTuples).c_str(), fullFileName.c_str()));
      return 0;
    }
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::ReadCtblFile(std::string fullFileName, vtkMRMLColorTableNode* colorNode)
{
  // open the file for reading input
  fstream fstr;
  fstr.open(fullFileName.c_str(), fstream::in);

  if (!fstr.is_open())
  {
    vtkErrorMacro("ERROR opening color file " << fullFileName << endl);
    return 0;
  }

  // clear out the table
  MRMLNodeModifyBlocker blocker(colorNode);

  // Set type to "File" by default if it has not been set yet.
  // It is important to only change type if it has not been set already
  // because otherwise "User" color node types would be always reverted to
  // read-only "File" type when the scene is saved and reloaded.
  if (colorNode->GetType() < colorNode->GetFirstType()
    || colorNode->GetType() > colorNode->GetLastType())
  {
    // no valid type has been set, set it to File
    colorNode->SetTypeToFile();
  }

  std::string line;
  // save the valid lines in a vector, parse them once know the max id
  std::vector<std::string>lines;
  int maxID = 0;
  while (fstr.good())
  {
    std::getline(fstr, line);

    // does it start with a #?
    if (line[0] == '#')
    {
      vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
      // sanity check: does the procedural header match?
      if (line.compare(0, 23, "# Color procedural file") == 0)
      {
        vtkErrorMacro("ReadDataInternal:\nfound a comment that this file "
          << " is a procedural color file, returning:\n"
          << line);
        return 0;
      }
    }
    else
    {
      // is it empty?
      if (line[0] == '\0')
      {
        vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
      }
      else
      {
        vtkDebugMacro("got a line: \n\"" << line << "\"");
        lines.emplace_back(line);
        std::stringstream ss;
        ss << line;
        int id;
        ss >> id;
        if (id > maxID)
        {
          maxID = id;
        }
      }
    }
  }
  fstr.close();
  // now parse out the valid lines and set up the color lookup table
  vtkDebugMacro("The largest id is " << maxID);
  if (maxID > this->MaximumColorID)
  {
    vtkErrorMacro("ReadData: maximum color id " << maxID << " is > "
      << this->MaximumColorID << ", invalid color file: " << fullFileName);
    colorNode->SetNumberOfColors(0);
    return 0;
  }
  // extra one for zero, also resizes the names array
  colorNode->SetNumberOfColors(maxID + 1);
  if (colorNode->GetLookupTable())
  {
    colorNode->GetLookupTable()->SetTableRange(0, maxID);
  }
  // init the table to black/opacity 0 with no name, just in case we're missing values
  colorNode->SetColors(0, maxID, colorNode->GetNoName(), 0.0, 0.0, 0.0, 0.0);

  // do a little sanity check, if never get an rgb bigger than 1.0, report
  // it as a possibly miswritten file
  bool biggerThanOne = false;
  for (unsigned int i = 0; i < lines.size(); i++)
  {
    std::stringstream ss;
    ss << lines[i];
    int id = 0;
    std::string name;
    double r = 0.0, g = 0.0, b = 0.0, a = 0.0;
    ss >> id;
    ss >> name;
    ss >> r;
    ss >> g;
    ss >> b;
    // check that there's still something left in the stream
    if (ss.str().length() == 0)
    {
      std::cout << "Error in parsing line " << i << ", no alpha information!" << std::endl;
    }
    else
    {
      ss >> a;
    }
    if (!biggerThanOne &&
      (r > 1.0 || g > 1.0 || b > 1.0))
    {
      biggerThanOne = true;
    }
    // the file values are 0-255, color look up table needs 0-1
    // clamp the colors just in case
    r = r > 255.0 ? 255.0 : r;
    r = r < 0.0 ? 0.0 : r;
    g = g > 255.0 ? 255.0 : g;
    g = g < 0.0 ? 0.0 : g;
    b = b > 255.0 ? 255.0 : b;
    b = b < 0.0 ? 0.0 : b;
    a = a > 255.0 ? 255.0 : a;
    a = a < 0.0 ? 0.0 : a;
    // now shift to 0-1
    r = r / 255.0;
    g = g / 255.0;
    b = b / 255.0;
    a = a / 255.0;
    // if the name has ticks around it, from copying from a mrml file, trim
    // them off the string
    if (name.find("'") != std::string::npos)
    {
      size_t firstnottick = name.find_first_not_of("'");
      size_t lastnottick = name.find_last_not_of("'");
      std::string withoutTicks = name.substr(firstnottick, (lastnottick - firstnottick) + 1);
      vtkDebugMacro("ReadDataInternal: Found ticks around name \"" << name << "\", using name without ticks instead:  \"" << withoutTicks << "\"");
      name = withoutTicks;
    }
    if (i < 10)
    {
      vtkDebugMacro("(first ten) Adding color at id " << id << ", name = " << name.c_str() << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a);
    }
    if (colorNode->SetColor(id, name.c_str(), r, g, b, a) == 0)
    {
      vtkWarningMacro("ReadData: unable to set color " << id << " with name " << name.c_str()
        << ", breaking the loop over " << lines.size() << " lines in the file " << this->FileName);
      return 0;
    }
    colorNode->SetColorNameWithSpaces(id, name.c_str(), "_");
  }
  if (lines.size() > 0 && !biggerThanOne)
  {
    vtkWarningMacro("ReadDataInternal: possibly malformed color table file:\n" << this->FileName
      << ".\n\tNo RGB values are greater than 1. Valid values are 0-255");
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
  }

  // cast the input node
  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(refNode);
  if (colorNode == nullptr || colorNode->GetLookupTable() == nullptr)
  {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known color table node");
    return 0;
  }

  std::string lowercaseFileExt = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(this->GetFileName());
  if (lowercaseFileExt == ".ctbl" || lowercaseFileExt == ".txt")
  {
    return this->WriteCtblFile(this->GetFileName(), colorNode);
  }
  else if (lowercaseFileExt == ".csv")
  {
    return this->WriteCsvFile(this->GetFileName(), colorNode);
  }
  else
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLColorTableStorageNode::WriteDataInternal",
      "Color table file '" << fullName.c_str() << "' has unknown file extension while trying to write node (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteCsvFile(std::string fullFileName, vtkMRMLColorTableNode* colorNode)
{
  unsigned int numberOfColors = colorNode->GetLookupTable()->GetNumberOfTableValues();

  // Construct table for color node
  vtkNew<vtkTable> colorTable;

  vtkNew<vtkIntArray> labelValueArray;
  labelValueArray->SetName("LabelValue");
  labelValueArray->SetNumberOfTuples(numberOfColors);
  colorTable->AddColumn(labelValueArray);
  vtkNew<vtkStringArray> nameArray;
  nameArray->SetName("Name");
  nameArray->SetNumberOfTuples(numberOfColors);
  colorTable->AddColumn(nameArray);
  vtkNew<vtkIntArray> colorArray;
  colorArray->SetName("Color");
  colorArray->SetNumberOfComponents(4); // RGBA
  colorArray->SetNumberOfTuples(numberOfColors);
  colorArray->SetComponentName(0, "R"); //Needed?
  colorArray->SetComponentName(1, "G");
  colorArray->SetComponentName(2, "B");
  colorArray->SetComponentName(3, "A");
  colorTable->AddColumn(colorArray);
  std::map<vtkIdType, std::vector<std::string>> componentNamesMap = {
    {colorTable->GetColumnIndex("Color"), {"R", "G", "B", "A"}} };
  unsigned int rowIndex = 0;
  for (int i = 0; i < numberOfColors; i++)
  {
    // Skip unnamed color
    if (colorNode->GetNoName() && colorNode->GetColorName(i) &&
        strcmp(colorNode->GetNoName(), colorNode->GetColorName(i)) == 0)
    {
      continue;
    }

    double rgba[4] = {0.0};
    colorNode->GetLookupTable()->GetTableValue(i, rgba);
    double rgba255[4] = { rgba[0] * 255.0, rgba[1] * 255.0, rgba[2] * 255.0, rgba[3] * 255.0 };
    labelValueArray->SetValue(rowIndex, i);
    nameArray->SetValue(rowIndex, colorNode->GetColorName(i));
    colorArray->SetTuple(rowIndex, rgba255);
    ++rowIndex;
  }

  return vtkMRMLTableStorageNode::WriteTable(fullFileName, colorTable,
    this->GetFieldDelimiterCharacters(fullFileName), componentNamesMap);
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteCtblFile(std::string fullFileName, vtkMRMLColorTableNode* colorNode)
{
  // open the file for writing
  fstream of;
  of.open(fullFileName.c_str(), fstream::out);

  if (!of.is_open())
  {
    vtkErrorMacro("WriteData: unable to open file " << fullFileName.c_str() << " for writing");
    return 0;
  }

  // put down a header
  of << "# Color table file " << (!fullFileName.empty() ? fullFileName : "null") << endl;
  if (colorNode->GetLookupTable() != nullptr)
  {
    unsigned int numberOfColors = colorNode->GetLookupTable()->GetNumberOfTableValues();
    of << "# " << numberOfColors << " values" << endl;
    for (int i = 0; i < numberOfColors; i++)
    {
      // Skip unnamed color
      if (colorNode->GetNoName() && colorNode->GetColorName(i) &&
          strcmp(colorNode->GetNoName(), colorNode->GetColorName(i)) == 0)
      {
        continue;
      }

      of << i << " ";
      of << colorNode->GetColorNameWithoutSpaces(i, "_") << " ";
      // the color look up table uses 0-1, file values are 0-255,
      double* rgba = colorNode->GetLookupTable()->GetTableValue(i);
      of << rgba[0] * 255.0 << " ";
      of << rgba[1] * 255.0 << " ";
      of << rgba[2] * 255.0 << " ";
      of << rgba[3] * 255.0 << endl;
    }
  }

  of.close();
  return 1;
}
