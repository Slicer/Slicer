/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLNodePropertyMacros_h
#define __vtkMRMLNodePropertyMacros_h

/// @file

//----------------------------------------------------------------------------
/// @defgroup vtkMRMLWriteXMLMacros Helper macros for writing MRML node properties to XML attributes.
/// They are To be used in WriteXML(ostream& of, int nIndent) method.
/// Arguments of value writing macros:
/// - xmlAttributeName: XML attribute name (without quotes), typically the same as the property name but starts with lowercase
/// - propertyName: property name (without quotes); value is retrieved using Get(propertyName) method.
///
/// @{

/// This macro must be placed before the first value writing macro.
/// \param of is the output stream where node values will be written to
#define vtkMRMLWriteXMLBeginMacro(of) \
  { \
  ostream& xmlWriteOutputStream = of;

/// This macro must be placed after the last value writing macro.
#define vtkMRMLWriteXMLEndMacro() \
  }

/// Macro for writing bool node property to XML.
#define vtkMRMLWriteXMLBooleanMacro(xmlAttributeName, propertyName) \
  xmlWriteOutputStream << " " #xmlAttributeName "=\"" << (Get##propertyName() ? "true" : "false") << "\"";

/// Macro for writing char* node property to XML.
/// If pointer is NULL then the attribute will not be written to XML.
#define vtkMRMLWriteXMLStringMacro(xmlAttributeName, propertyName) \
  if (Get##propertyName() != NULL) \
    { \
    xmlWriteOutputStream << " " #xmlAttributeName "=\""; \
    xmlWriteOutputStream << vtkMRMLNode::XMLAttributeEncodeString(Get##propertyName()); \
    xmlWriteOutputStream << "\""; \
    }

/// Macro for writing std::string node property to XML.
#define vtkMRMLWriteXMLStdStringMacro(xmlAttributeName, propertyName) \
  xmlWriteOutputStream << " " #xmlAttributeName "=\"" << vtkMRMLNode::XMLAttributeEncodeString(Get##propertyName().c_str()) << "\""; \

/// Macro for writing enum node property to XML.
/// Requires Get(propertyName)AsString method to convert from numeric value to code string.
#define vtkMRMLWriteXMLEnumMacro(xmlAttributeName, propertyName) \
  xmlWriteOutputStream << " " #xmlAttributeName "=\""; \
  if (Get##propertyName##AsString(Get##propertyName()) != NULL) \
    { \
    xmlWriteOutputStream << vtkMRMLNode::XMLAttributeEncodeString(Get##propertyName##AsString(Get##propertyName())); \
    } \
  xmlWriteOutputStream << "\"";

/// Macro for writing int node property to XML.
#define vtkMRMLWriteXMLIntMacro(xmlAttributeName, propertyName) \
  xmlWriteOutputStream << " " #xmlAttributeName "=\"" << Get##propertyName() << "\"";

/// Macro for writing floating-point (double or float) node property to XML.
#define vtkMRMLWriteXMLFloatMacro(xmlAttributeName, propertyName) \
  xmlWriteOutputStream << " " #xmlAttributeName "=\"" << Get##propertyName() << "\"";

/// Macro for writing vector (of numbers) node property to XML.
#define vtkMRMLWriteXMLVectorMacro(xmlAttributeName, propertyName, vectorType, vectorSize) \
  { \
    xmlWriteOutputStream << " " #xmlAttributeName "=\""; \
    vectorType* vectorPtr = Get##propertyName(); \
    if (vectorPtr != NULL) \
      { \
      for (int i=0; i<vectorSize; i++) \
        { \
        if (i > 0) \
          { \
          xmlWriteOutputStream << " "; \
          } \
        xmlWriteOutputStream << vectorPtr[i]; \
        } \
      } \
    xmlWriteOutputStream << "\""; \
  }

/// @}

//----------------------------------------------------------------------------
/// @defgroup vtkMRMLReadXMLMacros Helper macros for reading MRML node properties from XML attributes.
/// They are To be used in ReadXMLAttributes(const char** atts) method.
/// Arguments of value writing macros:
/// - xmlAttributeName: XML attribute name (without quotes), typically the same as the property name but starts with lowercase
/// - propertyName: property name (without quotes); value is set using Set(propertyName) method.
///
/// @{

/// This macro must be placed before the first value reading macro.
/// \param atts is the C array of of attribute name/value pointer pairs
#define vtkMRMLReadXMLBeginMacro(atts) \
  { \
  const char* xmlReadAttName; \
  const char* xmlReadAttValue; \
  const char** xmlReadAtts = atts; \
  while (*xmlReadAtts != NULL) \
    { \
    xmlReadAttName = *(xmlReadAtts++); \
    xmlReadAttValue = *(xmlReadAtts++); \
    if (xmlReadAttValue == NULL) \
      { \
      break; \
      }

/// This macro must be placed after the last value reading macro.
#define vtkMRMLReadXMLEndMacro() \
  }};

/// Macro for reading bool node property from XML.
#define vtkMRMLReadXMLBooleanMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    this->Set##propertyName(strcmp(xmlReadAttValue,"true") ? false : true); \
    }

/// Macro for reading char* node property from XML.
#define vtkMRMLReadXMLStringMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    this->Set##propertyName(vtkMRMLNode::XMLAttributeDecodeString(xmlReadAttValue).c_str()); \
    }

/// Macro for reading std::string node property from XML.
#define vtkMRMLReadXMLStdStringMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    this->Set##propertyName(vtkMRMLNode::XMLAttributeDecodeString(xmlReadAttValue)); \
    }

/// Macro for reading enum node property from XML.
/// Requires Get(propertyName)FromString method to convert from string to numeric value.
#define vtkMRMLReadXMLEnumMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    int propertyValue = this->Get##propertyName##FromString(vtkMRMLNode::XMLAttributeDecodeString(xmlReadAttValue).c_str()); \
    if (propertyValue >= 0) \
      { \
      this->Set##propertyName(propertyValue); \
      } \
    else \
      { \
      vtkErrorMacro("Failed to read #xmlAttributeName attribute value from string '" << xmlReadAttValue << "'"); \
      } \
    }

/// Macro for reading int node property from XML.
#define vtkMRMLReadXMLIntMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    vtkVariant variantValue(xmlReadAttValue); \
    bool valid = false; \
    int intValue =  variantValue.ToInt(&valid); \
    if (valid) \
      { \
      this->Set##propertyName(intValue); \
      } \
    else \
      { \
      vtkErrorMacro("Failed to read #xmlAttributeName attribute value from string '" << xmlReadAttValue << "': integer expected"); \
      } \
    }

/// Macro for reading floating-point (float or double) node property from XML.
#define vtkMRMLReadXMLFloatMacro(xmlAttributeName, propertyName) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    vtkVariant variantValue(xmlReadAttValue); \
    bool valid = false; \
    double scalarValue =  variantValue.ToDouble(&valid); \
    if (valid) \
      { \
      this->Set##propertyName(scalarValue); \
      } \
    else \
      { \
      vtkErrorMacro("Failed to read #xmlAttributeName attribute value from string '" << xmlReadAttValue << "': float expected"); \
      } \
    }

/// Macro for reading floating-point (float or double) vector node property from XML.
#define vtkMRMLReadXMLVectorMacro(xmlAttributeName, propertyName, vectorType, vectorSize) \
  if (!strcmp(xmlReadAttName, #xmlAttributeName)) \
    { \
    vectorType vectorValue[vectorSize] = {0}; \
    std::stringstream ss; \
    ss << xmlReadAttValue; \
    for (int i=0; i<vectorSize; i++) \
      { \
      vectorType val; \
      ss >> val; \
      vectorValue[i] = val; \
      } \
    this->Set##propertyName(vectorValue); \
    }

/// @}

//----------------------------------------------------------------------------
/// @defgroup vtkMRMLCopyMacros Helper macros for copying node properties from a source node.
/// They are To be used in Copy(vtkMRMLNode *anode) method.
/// Arguments of value copying macros:
/// - propertyName: property name (without quotes); value is get/set using Get/Set(propertyName) methods.
///
/// @{

/// This macro must be placed before the first value copying macro.
/// \param sourceNode pointer to the node where property values will be copied from
#define vtkMRMLCopyBeginMacro(sourceNode) \
  { \
  vtkMRMLNode* copySourceNode = this->SafeDownCast(sourceNode); \
  if (copySourceNode != NULL) \
    {

/// This macro must be placed after the last value copying macro.
#define vtkMRMLCopyEndMacro() \
    } \
  else \
    { \
    vtkErrorMacro("Copy failed: invalid source node"); \
    } \
  }

/// Macro for copying bool node property value.
#define vtkMRMLCopyBooleanMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying char* node property value.
#define vtkMRMLCopyStringMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying std::string node property value.
#define vtkMRMLCopyStdStringMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying int node property value.
#define vtkMRMLCopyIntMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying enum node property value.
#define vtkMRMLCopyEnumMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying floating-point (float or double) node property value.
#define vtkMRMLCopyFloatMacro(propertyName) \
  this->Set##propertyName(this->SafeDownCast(copySourceNode)->Get##propertyName());

/// Macro for copying floating-point (float or double) vector node property value.
#define vtkMRMLCopyVectorMacro(propertyName, vectorType, vectorSize) \
    { \
    /* Currently, vectorType and vectorSize is not essential, but in the future */ \
    /* this information may be used more. */ \
    vectorType* sourceVector = this->SafeDownCast(copySourceNode)->Get##propertyName(); \
    if (sourceVector != NULL) \
      { \
      this->Set##propertyName(sourceVector); \
      } \
    else \
      { \
      vtkErrorMacro("Failed to copy #xmlAttributeName attribute value: source node returned NULL"); \
      } \
    }

/// @}

//----------------------------------------------------------------------------
/// @defgroup vtkMRMLPrintMacros Helper macros for copying node properties from a source node.
/// They are To be used in PrintSelf(ostream& os, vtkIndent indent) method.
/// Arguments of value printing macros:
/// - propertyName: property name (without quotes); value is get using Get(propertyName) method.
///
/// @{

/// This macro must be placed before the first value printing macro.
/// - os: output stream
/// - indent: current indentation level
#define vtkMRMLPrintBeginMacro(os, indent) \
  { \
  ostream& printOutputStream = os; \
  vtkIndent printOutputIndent = indent;

/// This macro must be placed after the last value printing macro.
#define vtkMRMLPrintEndMacro() \
  }

/// Macro for printing bool node property value.
#define vtkMRMLPrintBooleanMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << (this->Get##propertyName() ? "true" : "false")  << "\n";

/// Macro for printing char* node property value.
#define vtkMRMLPrintStringMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << (this->Get##propertyName() != NULL ? this->Get##propertyName() : "(none)")  << "\n";

/// Macro for printing std::string node property value.
#define vtkMRMLPrintStdStringMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << this->Get##propertyName() << "\n";

/// Macro for printing enum node property value.
#define vtkMRMLPrintEnumMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << (Get##propertyName##AsString(Get##propertyName()))  << "\n";

/// Macro for printing int node property value.
#define vtkMRMLPrintIntMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << this->Get##propertyName() << "\n";

/// Macro for printing floating-point (float or double) node property value.
#define vtkMRMLPrintFloatMacro(propertyName) \
  printOutputStream << printOutputIndent << #propertyName ": " << this->Get##propertyName() << "\n";

/// Macro for printing floating-point (float or double) vector node property value.
#define vtkMRMLPrintVectorMacro(propertyName, vectorType, vectorSize) \
  { \
  printOutputStream << printOutputIndent << #propertyName " : ["; \
  vectorType* vectorValue = this->Get##propertyName(); \
  if (vectorValue) \
    { \
    for (int i=0; i<vectorSize; i++) \
      { \
      if (i > 0) \
        { \
        printOutputStream << ", "; \
        } \
      printOutputStream << vectorValue[i]; \
      } \
    printOutputStream << "]\n"; \
    } \
  }

/// @}

#endif // __vtkMRMLNodePropertyMacros_h
