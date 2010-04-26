#include "vtkObjectFactory.h"
#include "vtkPETCTFusionPlots.h"

#include "vtkDoubleArray.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkIdList.h"
#include "vtkSortDataArray.h"

vtkStandardNewMacro(vtkPETCTFusionPlots);
vtkCxxRevisionMacro(vtkPETCTFusionPlots, "$Revision: $");



//---------------------------------------------------------------------------
vtkPETCTFusionPlots::vtkPETCTFusionPlots()
{
  this->MRMLScene  = NULL;
  this->SUVPlotData.clear();
}




//---------------------------------------------------------------------------
vtkPETCTFusionPlots::~vtkPETCTFusionPlots()
{

  this->ClearPlots();
  if (this->MRMLScene)
    {
    this->MRMLScene->Delete();
    this->MRMLScene = NULL;
    }
}




//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::PrintSelf(ostream& os, vtkIndent indent)
{
}




//---------------------------------------------------------------------------
vtkIntArray* vtkPETCTFusionPlots::GetLabelList()
{
  vtkIntArray* array = vtkIntArray::New();

  VOItoPlotDataMapType::iterator iter;
  for (iter = this->SUVPlotData.begin(); iter != this->SUVPlotData.end(); iter ++)
    {
    int label = iter->first;
    array->InsertNextValue(label);
    }

  return array;
}



//---------------------------------------------------------------------------
int vtkPETCTFusionPlots::OutputAllDataInCSV ( ostream& os )
{
  Update();

  int success = 1;
  int label;
  VOItoPlotDataMapType::iterator iter;

  //--- SUVmax
  for (iter = this->SUVPlotData.begin(); iter != this->SUVPlotData.end(); iter ++)
    {
    label = iter->first;
    vtkMRMLDoubleArrayNode* anode = this->GetPlotData(label);
    if (anode)
      {
      vtkDoubleArray* data = anode->GetArray();
      if (data)
        {
        int nData = anode->GetSize();
        for (int i = 0; i < nData; i ++)
          {
          double* xy = data->GetTuple(i);
          // Write the data
          // label,   studydate,    suvmax,   suvmean
          // label,   studydate,    suvmax,   suvmean
          // ...
          // label,   studydate,    suvmax,   suvmean          
          //   -----------------------------------------
          os << label << ", " << xy[0] << ", " << xy[1]  << ", " << xy[2]  << std::endl;
          }
        }
      }
    else
      {
      success = 0;
      }
    }

  return (success) ;
}



//---------------------------------------------------------------------------
int vtkPETCTFusionPlots::OutputDataInCSV(ostream& os, int label)
{

  Update();

  vtkMRMLDoubleArrayNode* anode = this->GetPlotData(label);
  if (anode)
    {
    vtkDoubleArray* data = anode->GetArray();
    if (data)
      {
      int nData = anode->GetSize();
      for (int i = 0; i < nData; i ++)
        {
        double* xy = data->GetTuple(i);
          // Write the data
          // label,   studydate,    suvmax,   suvmean          
          // label,   studydate,    suvmax,   suvmean          
          // ...
          // label,   studydate,    suvmax,   suvmean          
          //   -----------------------------------------
        os << label << ", " << xy[0] << ", " << xy[1]  << ", " << xy[2]  << std::endl;
        }
      }
    return 1;
    }
  else
    {
    return 0;
    }

}


//---------------------------------------------------------------------------
int vtkPETCTFusionPlots::Update()
{

  this->GeneratePlot();
//  this->GenerateXLabels();
//  this->GenerateYLabels();
  this->Modified();
  return 1;

}


//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::GenerateXLabels( int label )
{
/*
  //--- create x-axis labels out of the dates
  vtkMRMLDoubleArrayNode::LabelsVectorType labels;
  vtkDoubleArray *a = this->SUVPlotData[label]->GetArray();
  int numTuples = a->GetNumberOfTuples();
  std::stringstream ss;
  std::string day;
  std::string month;
  std::string year;
  std::string date;
  double val;
  int intval;

  for ( int i=0; i < numTuples; i ++ )
    {
    // pull out the date
    a->GetTupleValue (i,val);
    // cast to an int
    intval = static_cast<int>(val);
    // reformat 20081002 --> 02/10/2008
    ss.clear();
    ss << intval;
    day.clear();
    month.clear();
    year.clear();
    date.clear();
    year = ss.string().range(0, 4);
    month = ss.string().range(4, 2);
    day = ss.string().range(6, 2);
    ss << day << "/" << month << "/" << year << " ";
    date = ss.string();
    labels.push_back ( date.c_str() );
    }
*/
}

//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::GenerateYLabels( int label )
{
  //--- create y-axis labels out of the SUVs
}


//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::GeneratePlot()
{
  //--- get number of labels in the mask
 
  vtkIdList *keys = vtkIdList::New();
  vtkSortDataArray *sorter = vtkSortDataArray::New();

  vtkIntArray *labels = this->GetLabelList();
  int label;
  int numVOIs = this->SUVPlotData.size();  
  if ( numVOIs > 0 && labels != NULL )
    {
    for ( int voi = 0; voi < numVOIs; voi ++ )
      {
      //---
      //--- sort the each label's SUV array by study date
      //--- To accomplish this, copy the study dates
      //--- from the plot array into their own separate
      //--- array that can be used as keys for sorting.
      label = labels->GetValue(voi);
      vtkDoubleArray *a = this->SUVPlotData[label]->GetArray();

      //--- get number of timepoints included in the array
      //--- strip off the date at each timepoint and add
      //--- that (cast as an int) to the array to be sorted.
      int numTuples = a->GetNumberOfTuples();
      keys->SetNumberOfIds ( numTuples );

      double d[3];
      for ( int i=0; i < numTuples; i++ )
        {
        a->GetTupleValue (i,d);
        if ( d != NULL )
          {
          //--- dump the date into the keys list
          //--- cast it to an int.
          keys->InsertNextId ( static_cast<int>(d[0]) );
          }
        }

      //--- Now sort
      if ( keys->GetNumberOfIds() == numTuples )
        {
        sorter->Sort ( keys, this->SUVPlotData[label]->GetArray() );
        }

      //--- get rid of keys for this label,
      //--- to set up for date-sorting next label.
      keys->Reset();
      }
    }

  //---Now all should be sorted and ready to displayplot.
  
  // clean up afterward.
  keys->Delete();
  sorter->Delete();
}



//---------------------------------------------------------------------------
int vtkPETCTFusionPlots::GetNumberOfLabels()
{
  vtkIntArray *array = this->GetLabelList();
  int numLabels = 0;
  if ( array )
    {
    numLabels = array->GetNumberOfTuples();
    array->Delete();
    array = NULL;
    }
  return ( numLabels );
}

//---------------------------------------------------------------------------
int vtkPETCTFusionPlots::GetNumberOfTimepoints ( int label )
{
  vtkMRMLDoubleArrayNode *node = GetPlotData (label );
  int numTimepoints = 0;
  if ( node )
    {
    numTimepoints = node->GetSize();
    }
  return (numTimepoints);
}


//---------------------------------------------------------------------------
vtkMRMLDoubleArrayNode* vtkPETCTFusionPlots::GetPlotData(int label)
{
  VOItoPlotDataMapType::iterator iter;

  iter = this->SUVPlotData.find(label);
  if (iter != this->SUVPlotData.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }
}




//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::AddSUV ( int label, int year, int month, int day, double suvmax, double suvmean )

{

  // SUVPlotData looks like the following
  // ----------------------------------------------------------------------------------
  //  int (voi label)  |   vtkMRMLDoubleArrayNode
  // ----------------------------------------------------------------------------------
  //     yellow label |  Node->GetArray()= {[date, suvmax, suvmean],....,[date, suvmax, suvmean]}
  //     green label  |  Node->GetArray()= {[date, suvmax, suvmean],....,[date, suvmax, suvmean]}
  //     ...               |   ...
  //     blue label    |  Node->GetArray()= {[date, suvmax, suvmean],....,[date, suvmax, suvmean]}  
  // ----------------------------------------------------------------------------------
  //--- samples are in the order added, not necessarily a date-sorted order yet.
  
  //--- Create the plotdata if it doesn't yet exist.
  if ( this->GetPlotData(label) == NULL )
    {
    vtkDoubleArray* array;
    array  = vtkDoubleArray::New();
    array->SetNumberOfComponents( static_cast<vtkIdType>(3) );

    vtkMRMLDoubleArrayNode* anode = vtkMRMLDoubleArrayNode::New();
    this->GetMRMLScene()->AddNode(anode);
    anode->SetArray(array);
    this->SUVPlotData[label] = anode;

    array->Delete();
    anode->Delete();
    }


  //--- add values to it.
  double sample[3];
  //--- encode year, month, day into time
  //--- probably better way, but good enough for test.
  // e.g. 2008, 10, 02 --> 20081002.0
  double time = (year * 10000.0) + (month * 100.0) + (day * 1.0);
  sample[0] = time;
  sample[1] = suvmax;
  sample[2] = suvmean;
  this->SUVPlotData[label]->GetArray()->InsertNextTuple(sample);
}




//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::ResetPlots ( )
{

  VOItoPlotDataMapType::iterator iter;
  int label;
  for (iter = this->SUVPlotData.begin(); iter != this->SUVPlotData.end(); iter ++)
    {
    label = iter->first;
    if ( ( this->GetPlotData ( label ) != NULL) &&
         ( this->SUVPlotData[label]->GetArray() != NULL ) )
      {
      //--- empty array, don't free memory.
      this->SUVPlotData[label]->GetArray()->Reset();
      }
     //this->SUVPlotData.erase (iter);
    }
}


//---------------------------------------------------------------------------
void vtkPETCTFusionPlots::ClearPlots ( )
{

  VOItoPlotDataMapType::iterator iter;
  int label;
  for (iter = this->SUVPlotData.begin(); iter != this->SUVPlotData.end(); iter ++)
    {
    label = iter->first;
    if ( ( this->GetPlotData ( label ) != NULL) &&
         ( this->SUVPlotData[label]->GetArray() != NULL ) )
      {
      //--- empty array, free memory.
      this->SUVPlotData[label]->GetArray()->Reset();
      this->MRMLScene->RemoveNode(this->SUVPlotData[label]);
      //--- will this leak datastructures?
      this->SUVPlotData[label] = NULL;
      }
    }

  VOItoPlotDataMapType::size_type size = this->SUVPlotData.size();
  for( VOItoPlotDataMapType::size_type i=0; i < size; i++ )
    {
    iter = this->SUVPlotData.begin();
    this->SUVPlotData.erase( iter );
    }
}
