/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLDoubleArrayStorageNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLDoubleArrayStorageNode* vtkMRMLDoubleArrayStorageNode::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDoubleArrayStorageNode");
    if(ret)
    {
        return (vtkMRMLDoubleArrayStorageNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLDoubleArrayStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDoubleArrayStorageNode::CreateNodeInstance()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDoubleArrayStorageNode");
    if(ret)
    {
        return (vtkMRMLDoubleArrayStorageNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLDoubleArrayStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayStorageNode::vtkMRMLDoubleArrayStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayStorageNode::~vtkMRMLDoubleArrayStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::ReadXMLAttributes(const char** atts)
{

    Superclass::ReadXMLAttributes(atts);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLDoubleArrayStorageNode::Copy(vtkMRMLNode *anode)
{
    Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
    vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
    this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayStorageNode::ReadData(vtkMRMLNode *refNode)
{
    // do not read if if we are not in the scene (for example inside snapshot)
    if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
        return 1;
    }

    if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 1;
    }

    vtkDebugMacro("Reading measurement data");
    // test whether refNode is a valid node to hold a measurement list
    if ( !( refNode->IsA("vtkMRMLDoubleArrayNode"))
        ) 
    {
        vtkErrorMacro("Reference node is not a proper vtkMRMLDoubleArrayNode");
        return 0;         
    }

    if (this->GetFileName() == NULL && this->GetURI() == NULL) 
    {
        vtkErrorMacro("ReadData: file name and uri not set");
        return 0;
    }

    Superclass::StageReadData(refNode);
    if ( this->GetReadState() != this->TransferDone )
    {
        // remote file download hasn't finished
        vtkWarningMacro("ReadData: Read state is pending, returning.");
        return 0;
    }

    std::string fullName = this->GetFullNameFromFileName(); 

    if (fullName == std::string("")) 
    {
        vtkErrorMacro("vtkMRMLDoubleArrayStorageNode: File name not specified");
        return 0;
    }

    // cast the input node
    vtkMRMLDoubleArrayNode *doubleArrayNode = NULL;
    if ( refNode->IsA("vtkMRMLDoubleArrayNode") )
    {
        doubleArrayNode = dynamic_cast <vtkMRMLDoubleArrayNode *> (refNode);
    }

    if (doubleArrayNode == NULL)
    {
        vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a double array (measurement) node");
        return 0;
    }

    // open the file for reading input
    fstream fstr;

    fstr.open(fullName.c_str(), fstream::in);

    if (fstr.is_open())
    {
        //turn off modified events
        int modFlag = doubleArrayNode->GetDisableModifiedEvent();   //
        doubleArrayNode->DisableModifiedEventOn();
        char line[1024];
        // default column ordering for measurement info
        // independent_variable dependent_veariable err
        int xColumn = 0;
        int yColumn = 1;
        int zColumn = 2;

        int numColumns = 3;
        std::vector<std::string> labels;
        // save the valid lines in a vector, parse them once know the max id
        std::vector<std::string>lines;
        bool firstLine = 1;

        while (fstr.good())
        {
            fstr.getline(line, 1024);


            // does it start with a #?
            if (line[0] == '#')
            {
                vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
                
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
                    char *ptr;
                    bool reTokenise = false;
                    if (strncmp(line, ",", 1) == 0)
                    {
                        ptr = NULL;
                        reTokenise = true;
                    }
                    else
                    {
                        ptr = strtok(line, ",");
                    }
                    
                    double x = 0.0, y = 0.0, yerr = 0.0;
                    int columnNumber = 0;

                    while (columnNumber < numColumns)
                    {
                        if (ptr != NULL)
                        {
                            if (columnNumber == xColumn)
                            {
                                if (firstLine)
                                {
                                    labels.push_back(ptr);
                                }
                                else
                                {
                                    x = atof(ptr);
                                }
                            }
                            else if (columnNumber == yColumn)
                            {
                                if (firstLine)
                                {
                                    labels.push_back(ptr);
                                }
                                else
                                {
                                    y = atof(ptr);
                                }
                            }
                            else if (columnNumber == zColumn)
                            {
                                if (firstLine)
                                {
                                    labels.push_back(ptr);
                                }
                                else
                                {
                                    yerr = atof(ptr);
                                }
                            }
                        }
                        if (reTokenise == false)
                        {
                            ptr = strtok(NULL, ",");
                        }
                        else
                        {
                            ptr = strtok(line, ",");
                            // turn it off
                            reTokenise = false;
                        }
                        columnNumber++;
                    } // end while over columns
                    
                    int   fidIndex;
                    if (firstLine)
                    {
                        doubleArrayNode->vtkMRMLDoubleArrayNode::SetLabels(labels);
                    }
                    else
                    {
                        fidIndex = doubleArrayNode->vtkMRMLDoubleArrayNode::AddXYValue(x, y, yerr);
                        if (fidIndex == 0)
                        {
                            vtkErrorMacro("Error adding a measurement to the list");
                        }
                    }
                    firstLine = 0;

                } // point line
            }
        }
        doubleArrayNode->SetDisableModifiedEvent(modFlag);
        doubleArrayNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, doubleArrayNode);//vtkMRMLFiducialListNode::DisplayModifiedEvent);
        fstr.close();
    }

    else
    {
        vtkErrorMacro("ERROR opening measurement file " << this->FileName << endl);
        return 0;
    }

    this->SetReadStateIdle();

    // make sure that the list node points to this storage node
    //-------------------------> doubleArrayNode->SetAndObserveStorageNodeID(this->GetID());


    // mark it unmodified since read
    doubleArrayNode->ModifiedSinceReadOff();

    return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayStorageNode::WriteData(vtkMRMLNode *refNode)
{

    // test whether refNode is a valid node to hold a volume
    if ( !( refNode->IsA("vtkMRMLDoubleArrayNode") ) )
    {
        vtkErrorMacro("Reference node is not a proper vtkMRMLDoubleArrayNode");
        return 0;         
    }

    if (this->GetFileName() == NULL) 
    {
        vtkErrorMacro("WriteData: file name is not set");
        return 0;
    }

    std::string fullName = this->GetFullNameFromFileName();
    if (fullName == std::string("")) 
    {
        vtkErrorMacro("vtkMRMLDoubleArrayStorageNode: File name not specified");
        return 0;
    }

    // cast the input node
    vtkMRMLDoubleArrayNode *doubleArrayNode = NULL;
    if ( refNode->IsA("vtkMRMLDoubleArrayNode") )
    {
        doubleArrayNode = dynamic_cast <vtkMRMLDoubleArrayNode *> (refNode);
    }

    if (doubleArrayNode == NULL)
    {
        vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known double array node");
        return 0;
    }

    // open the file for writing
    fstream of;

    of.open(fullName.c_str(), fstream::out);

    if (!of.is_open())
    {
        vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
        return 0;
    }

    // put down a header
    of << "# measurement file " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;

    // if change the ones being included, make sure to update the parsing in ReadData
    of << "# columns = x,y,yerr" << endl;
    std::vector< std::string > labels = doubleArrayNode->GetLabels();
    if (labels.size())
    {
        for(unsigned int l = 0; l<labels.size()-1; l++)
        {
            of << labels.at(l) << ",";
        }
        of << labels.at(labels.size()-1) << endl;
    }



    for (unsigned int i = 0; i < doubleArrayNode->GetSize(); i++)
    {
        double x,y,yerr;
        if (doubleArrayNode->GetXYValue(i, &x, &y, &yerr))
        {

            of << x << "," << y << "," << yerr;
            of << endl;   
        }
        else if (doubleArrayNode->GetXYValue(i, &x, &y))
        {

            of << x << "," << y ;
            of << endl;
        }
    }
    of.close();

    Superclass::StageWriteData(refNode);

    return 1;

}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayStorageNode::SupportedFileType(const char *fileName)
{
    // check to see which file name we need to check
    std::string name;
    if (fileName)
    {
        name = std::string(fileName);
    }
    else if (this->FileName != NULL)
    {
        name = std::string(this->FileName);
    }
    else if (this->URI != NULL)
    {
        name = std::string(this->URI);
    }
    else
    {
        vtkWarningMacro("SupportedFileType: no file name to check");
        return 0;
    }

    std::string::size_type loc = name.find_last_of(".");
    if( loc == std::string::npos ) 
    {
        vtkErrorMacro("SupportedFileType: no file extension specified");
        return 0;
    }
    std::string extension = name.substr(loc);

    vtkDebugMacro("SupportedFileType: extension = " << extension.c_str());
    if (extension.compare(".mcsv") == 0 ||
       extension.compare(".txt") == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::InitializeSupportedWriteFileTypes()
{
    this->SupportedWriteFileTypes->InsertNextValue("Measurement CSV (.mcsv)");
    this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}
