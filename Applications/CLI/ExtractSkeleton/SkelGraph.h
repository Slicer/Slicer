#ifndef _SKEL_GRAPH_H_
#define _SKEL_GRAPH_H_

#include <list>
#include "coordTypes.h"

typedef struct skel_branch_struct
{
  int branchID;     // == position in graph
  double length;

  double acc_length;  // for temporary use when searching maximal path
  list<int> * acc_path;
  
  double max_length;
  list<int> * max_path;         // maximal path 
   
  point * end_1_point;
  point * end_2_point;
  list<int> * end_1_neighbors; // id's == one can use advance for random access
  list<int> * end_2_neighbors;
} skel_branch;


class SkelGraph  {
 private:

  // Extracted Graph
  list<skel_branch> * graph;
  
  // To_Do list, only of temporary use for extract graph
  list<skel_branch> * to_do;

  // endpoint list, only of temporary use
  list<point> * endpoints;
  
  // Image to extract from
  unsigned char *image;
  int dim[3];
  // Label image, only of temporary use
  int  *label_image;

  skel_branch * max_node;   // for storage of start of maximal path
  double        max_length;

  
  // private routines

  void Add_new_elem_to_todo(skel_branch * &newElem);
  // adds a new element with default values to To_do list
  
  void find_endpoints();
  // find all endpoints in image

  int endpoint_Test(int x, int y, int z);
  // tests whether (x,y,z) is an endpoint
  
  void get_valid_neighbors(point *point1, std::list<point> * &neighbors);
  // returns a list of valid neighbors at act_point
  
  void ResetGraph();
  
 public:

  SkelGraph();
  SkelGraph(SkelGraph * graph); // not fully implemented
  ~SkelGraph();
  
  void PrintGraph();
  
  void Extract_skel_graph(unsigned char *orig_image, int orig_dim[3]);
  // Extract skeletal graph

  void Extract_max_axis_in_graph();
  // extract maximal path between 2 points in the graph
  
  void Sample_along_axis(int n_dim, list<point> * axis_points);
  // sample along the medial axis and perpendicular to it

};


#endif
