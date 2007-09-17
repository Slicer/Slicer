//  ===============================================
//  SkelGraph.cc
//  Graphs a Thinning-skeleton 
//  + Processing utilities of the graph
//  ===============================================

#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <iterator>

#include "SkelGraph.h"


static char msg[200];
static char tmp_filename[1024];

/*
===============================================
Constructors, Destructor
===============================================
*/

SkelGraph::SkelGraph() 
  {
  graph  = NULL;
  to_do = NULL;
  endpoints = NULL;
  image = NULL;
  label_image = NULL;
  max_node = NULL;
  }

SkelGraph::SkelGraph(SkelGraph * skelgraph)
  {  // not fully implemented
  if (skelgraph == NULL) {
    SkelGraph();
    return;
    }

  // Extracted Graph
  if (skelgraph->graph)
    graph = new list<skel_branch> (*(skelgraph->graph));
  else
    graph = NULL;

  if (skelgraph->to_do)
    to_do = new list<skel_branch> (*(skelgraph->to_do));
  else
    to_do = NULL;

  if (skelgraph->endpoints)
    endpoints = new list<point> (*(skelgraph->endpoints));
  else
    endpoints = NULL;

  image = skelgraph->image;
  label_image = skelgraph->label_image; // only for temporary use, WATCH OUT
  for (int i = 0; i < 3; i++) dim[i] = skelgraph->dim[i];

  max_length = skelgraph->max_length;
  max_node = NULL;                   // for storage of start of maximal path
  // find right max_node
  list<skel_branch>::iterator elem, elem_new;
  elem_new = graph->begin();
  elem = skelgraph->graph->begin();
  while (elem_new != graph->end()) {
    if (skelgraph->max_node == &(*elem))
      max_node = &(*elem_new);
    ++ elem_new;
    ++ elem;
    }

  }

SkelGraph::~SkelGraph() 
  {
  if (image) delete [] image;
  if (label_image) delete [] label_image;
  ResetGraph();
  }

void SkelGraph::ResetGraph() 
  {
  if (graph && !(graph->empty())) {
    list<skel_branch>::iterator elem;
    elem = graph->begin();
    while (elem != graph->end()) {
      // delete content
      if (elem->end_1_neighbors) delete elem->end_1_neighbors;
      if (elem->end_2_neighbors) delete elem->end_2_neighbors;
      if (elem->end_1_point) delete elem->end_1_point;
      if (elem->end_2_point) delete elem->end_2_point;
      if (elem->acc_path) delete elem->acc_path;
      if (elem->max_path) delete elem->max_path;
      elem++;
      }
    delete graph; graph = NULL;
    }
  if (to_do && !(to_do->empty())) {
    list<skel_branch>::iterator elem;
    elem = to_do->begin();
    while (elem != to_do->end()) {
      // delete content
      if (elem->end_1_neighbors) delete elem->end_1_neighbors;
      if (elem->end_2_neighbors) delete elem->end_2_neighbors;
      if (elem->end_1_point) delete elem->end_1_point;
      if (elem->end_2_point) delete elem->end_2_point;
      if (elem->acc_path) delete elem->acc_path;
      if (elem->max_path) delete elem->max_path;
      elem++;
      }
    delete to_do; to_do = NULL;
    }
  if (endpoints) 
    delete endpoints; 
  endpoints = NULL;
  }

// -------------------------------------------------------------------------
// Public Methods
// -------------------------------------------------------------------------


void SkelGraph::Extract_skel_graph(unsigned char *orig_image, int orig_dim[3])
// Graph compilation of a thinning skeleton
  {
  if (graph) ResetGraph();
  graph = new list<skel_branch>();
  if (label_image) delete [] label_image;

  image = orig_image;
  for (int i = 0; i < 3; i++) dim[i] = orig_dim[i];
  int size_image = dim[0]*dim[1]*dim[2];

  label_image = new int[size_image];
  for (int i = 0; i < size_image; i++) label_image[i] = 0;

  // determine endpoints = points that have exactly 1 neighbor 
  find_endpoints();
  // if (DEBUG_VSKEL)
  //  cout << endpoints->size() << " Endpoints found" << endl;

  // if  (while) untreated endpoint -> put it into to_do list
  // if  (while) to_do list non-empty -> follow branches 
  //    following of branch has 1 neighbor -> label, cont
  //    branch ends                        -> stop, next
  //    multiple branches emerge           -> stop, put in to_do list 
  list<point>::iterator act_endpoint;
  act_endpoint = endpoints->begin();
  // if  (while) untreated endpoint -> put it into to_do list
  while (act_endpoint != endpoints->end()) {
    if (act_endpoint != endpoints->end()) {
      // add to to_do list (which should be empty at this point)
      to_do = new list<skel_branch>;
      skel_branch *branch_elem;
      Add_new_elem_to_todo(branch_elem);
      branch_elem->end_1_point->x = branch_elem->end_2_point->x = act_endpoint->x;
      branch_elem->end_1_point->y = branch_elem->end_2_point->y = act_endpoint->y;
      branch_elem->end_1_point->z = branch_elem->end_2_point->z = act_endpoint->z;

      list<skel_branch>::iterator act_branch;
      act_branch = to_do->begin();
      while (!to_do->empty()) {
        // act_branch != to_do->end() && 
        // if  (while) to_do list non-empty -> follow branches
        int branch_done = 0;
        point * act_point = act_branch->end_2_point;
        int branchID = act_branch->branchID;
        // label endpoint
        label_image[act_point->x + 
          dim[0] * (act_point->y + dim[1] * act_point->z)] = branchID;
        while (!branch_done) {
          list<point> * neighbors = new list<point>();
          get_valid_neighbors(act_point, neighbors);
          int num_nb = neighbors->size();
          if (num_nb == 0) {
            //    branch ends                        -> stop, next
            branch_done = 1;
            } if (num_nb == 1) {
              //    following of branch has 1 neighbor -> label, cont
              point * pt = &(*(neighbors->begin()));
              // update length 
              // since act_point->x - pt->x is either [-1,0,1] -> abs == ^2 
              act_branch->length += sqrt((float) abs(act_point->x - pt->x) +
                abs(act_point->y - pt->y) +
                abs(act_point->z - pt->z));
              act_point->x = pt->x; act_point->y = pt->y; act_point->z = pt->z;
              label_image[act_point->x + 
                dim[0] * (act_point->y + dim[1] * act_point->z)] = branchID;
            } else {
              //    multiple branches emerge           -> stop, put in to_do list
              branch_done = 1;
              skel_branch ** elems = new skel_branch * [num_nb];
              list<point>::iterator act_neighbor = neighbors->begin();
              int  i = 0;
              while (act_neighbor != neighbors->end()) {
                point * pt = &(*act_neighbor);
                Add_new_elem_to_todo(elems[i]);
                // label start point
                elems[i]->end_1_point->x = elems[i]->end_2_point->x = pt->x; 
                elems[i]->end_1_point->y = elems[i]->end_2_point->y = pt->y; 
                elems[i]->end_1_point->z = elems[i]->end_2_point->z = pt->z;
                label_image[pt->x + dim[0] * 
                  (pt->y + dim[1] * pt->z)] = elems[i]->branchID;
                // update ends with act_branch
                if (! elems[i]->end_1_neighbors)
                  elems[i]->end_1_neighbors = new list<int>();
                elems[i]->end_1_neighbors->push_back(act_branch->branchID);
                if (! act_branch->end_2_neighbors)
                  act_branch->end_2_neighbors = new list<int>();
                act_branch->end_2_neighbors->push_back(elems[i]->branchID);

                ++act_neighbor; i++;
                }
              // update ends of new branches with each other
              for (int i = 0; i < num_nb; i++) {
                for (int j = 0; j < num_nb; j++) {
                  if (i != j) {
                    if (! elems[i]->end_1_neighbors)
                      elems[i]->end_1_neighbors = new list<int>();
                    elems[i]->end_1_neighbors->push_back(elems[j]->branchID);
                    }
                  }
                }
              delete elems; elems = NULL;
              } // else
            delete neighbors; neighbors = NULL;
          } // while (! branch_done)
        // copy branch from to_do to graph
        graph->insert(graph->end(),*act_branch);
        // remove it from to_do
        to_do->pop_front(); 
        act_branch = to_do->begin();
        }
      delete to_do; to_do = NULL;
      }
    // find next valid endpoint -> an yet unlabeled endpoint
    while (act_endpoint != endpoints->end()  &&
      label_image[act_endpoint->x + 
      dim[0] * (act_endpoint->y + dim[1] * act_endpoint->z)]) {
        ++act_endpoint;
      }
    }

  // done
  delete endpoints; endpoints = NULL;
  delete [] label_image; label_image = NULL;
  image = NULL; // no delete, since image points to original

  // if (DEBUG_VSKEL)
  //   cout << graph->size() << " Branches found" << endl;
  // graph print
  //PrintGraph();
  }

void SkelGraph::PrintGraph()
// print actual graph
  {
  list<skel_branch>::iterator act_graph;
  list<int>::iterator act_nb;

  act_graph = graph->begin();
  cout << "Graph : " << endl;
  int cnt = 0;
  cout << "Number: Branch Br.ID | N 1 | N 2 | Length | End1 | End2 " << endl;
  while (act_graph != graph->end()) {
    cnt ++;
    cout << cnt << ": Br. " << act_graph->branchID;

    if (act_graph->end_1_neighbors) {
      act_nb = act_graph->end_1_neighbors->begin();
      cout << "| ";
      while (act_nb != act_graph->end_1_neighbors->end()) {
        cout << *act_nb << ", " ;
        act_nb ++;
        }
      } else {
        cout << "| None";
      }

    if (act_graph->end_2_neighbors) {
      act_nb = act_graph->end_2_neighbors->begin();
      cout << "| ";
      while (act_nb != act_graph->end_2_neighbors->end()) {
        cout << *act_nb << ", " ;
        act_nb ++;
        }
      } else {
        cout << "|  None";
      }
    cout  << "| " << act_graph->length << "| " << 
      act_graph->end_1_point->x << "," << act_graph->end_1_point->y << "," <<
      act_graph->end_1_point->z << " | " << 
      act_graph->end_2_point->x << "," << act_graph->end_2_point->y << "," <<
      act_graph->end_2_point->z << " | " << endl;
    act_graph++;
    }
  }


void SkelGraph::Extract_max_axis_in_graph()
// extract maximal path between 2 points in the graph
  {
  list<skel_branch>::iterator act_endbranch;

  act_endbranch = graph->begin();
  // initialize
  while (act_endbranch != graph->end()) {
    act_endbranch->max_length = 0.0;
    if (act_endbranch->max_path)
      delete act_endbranch->max_path;
    act_endbranch->max_path = NULL;
    ++act_endbranch;
    }

  act_endbranch = graph->begin();

  while (act_endbranch != graph->end()) {
    //  search for next entry that has neighbors but
    // end_1_neighbors == NULL OR act_endbranch->end_2_neighbors != NULL
    while (act_endbranch->end_1_neighbors && act_endbranch->end_2_neighbors &&
      (act_endbranch->end_1_neighbors || act_endbranch->end_2_neighbors)) 
      ++act_endbranch;  

    // initialize
    list<skel_branch>::iterator act_branch;
    act_branch = graph->begin();
    while (act_branch != graph->end()) {
      act_branch->acc_length = 0.0;
      if (act_branch->acc_path)
        delete act_branch->acc_path;
      act_branch->acc_path = NULL;
      ++act_branch;
      }

    // do cost traversal
    list<skel_branch *> * wait_list = new list<skel_branch *>();
    wait_list->push_back(&(*act_endbranch));
    while (!wait_list->empty()) {
      // get next entry in wait_list
      skel_branch * act_node = *(wait_list->begin());
      wait_list->pop_front();

      // add to path
      act_node->acc_length += act_node->length;
      if (!act_node->acc_path)
        act_node->acc_path = new list<int>();
      act_node->acc_path->push_back(act_node->branchID);
      int act_pos_id = act_node->branchID;
      list<skel_branch>::iterator act_pos_list = graph->begin();
      // since the graph_id's are the location of its member in the list, 
      // we can use advance for random access
      advance(act_pos_list, act_pos_id-1);
      //cout << "A " << act_pos_id  << endl;

      for (int i = 0; i < 2; i++) {
        list<int> * cont_end;
        point * cont_end_point;
        if (i == 0) {
          cont_end = act_node->end_2_neighbors;
          cont_end_point = act_node->end_2_point;
          } else if (i == 1) {
            cont_end = act_node->end_1_neighbors;
            cont_end_point = act_node->end_1_point;
          }
        if (cont_end) {
          // add all neighbors to wait_list that are not yet treated 
          list<int>::iterator neighbors = cont_end->begin();
          while (neighbors != cont_end->end()) {
            // get neighbours entry
            int distance = *neighbors - act_pos_id;
            list<skel_branch>::iterator act_pos_neigh = act_pos_list;
            advance(act_pos_neigh, distance);
            skel_branch * act_neighbor = &(*(act_pos_neigh));
            if (!act_neighbor->acc_path)  {
              // neighbour not yet treated
              wait_list->push_back(act_neighbor);
              // update entries of neighbour
              // since act_point->x - pt->x is either [-1,0,1] -> abs == ^2 
              // add distance between branches to length at preceding branch
              point * cont_neigh_point;
              act_neighbor->acc_length = act_node->acc_length;
              // determine connection costs -> since we do not know which one is the
              // corresponding endpoint of the neighbour, we have to try out and take
              // the one combination that yields the smallest costs
              double conn_costs = 100.0;
              for (int j = 0; j < 2; j++) {
                if (j == 0) {
                  cont_neigh_point = act_neighbor->end_1_point;
                  } else if (j == 1) {
                    cont_neigh_point = act_neighbor->end_2_point;
                  }
                double new_costs = sqrt((float)abs(cont_neigh_point->x - cont_end_point->x) + 
                  abs(cont_neigh_point->y - cont_end_point->y) +
                  abs(cont_neigh_point->z - cont_end_point->z));
                if (conn_costs > new_costs) conn_costs = new_costs;
                }
              act_neighbor->acc_length += conn_costs; 


              // copy path
              if (act_neighbor->acc_path)  {
                delete act_neighbor->acc_path;
                return;
                } 
              act_neighbor->acc_path = new list<int>(*(act_node->acc_path)); 
              // initiate with copy of path of preceding branch
              }
            ++neighbors;
            }
          }
        } 
      }
    delete wait_list; wait_list = NULL;

    // look for maximum

    list<skel_branch>::iterator act_graph;
    act_graph = graph->begin();
    skel_branch * act_max_node = &(*(act_graph));
    double act_max_val = 0.0;
    while (act_graph != graph->end()) {
      if (act_graph->acc_length > act_max_val) {
        act_max_val = act_graph->acc_length;
        act_max_node = &(*(act_graph));
        }
      ++act_graph;
      }

    // copy maximal path
    act_endbranch->max_length = act_max_val;
    if (act_endbranch->max_path)
      delete act_endbranch->max_path;
    act_endbranch->max_path = new list<int>(*(act_max_node->acc_path));

    //     if (DEBUG_VSKEL) {
    //       cout << act_endbranch->branchID 
    //      << " : max length = " << act_endbranch->max_length
    //      << " | path = ";
    //       //double len = 0.0;
    //       list<int>::iterator act_path = act_endbranch->max_path->begin();
    //       //list<skel_branch>::iterator old_pos = graph->end();
    //       while (act_path != act_endbranch->max_path->end()) {
    //   // list<skel_branch>::iterator act_pos = graph->begin();
    //   // advance(act_pos, *act_path - 1);
    //   //len += act_pos->length;
    //   //if (old_pos != graph->end()) {
    //   //double len1 = sqrt(abs(act_pos->end_1_point->x - old_pos->end_2_point->x) + 
    //   //         abs(act_pos->end_1_point->y - old_pos->end_2_point->y) +
    //   //         abs(act_pos->end_1_point->z - old_pos->end_2_point->z));
    //   //  double len2 = sqrt(abs(act_pos->end_2_point->x - old_pos->end_1_point->x) + 
    //   //         abs(act_pos->end_2_point->y - old_pos->end_1_point->y) +
    //   //         abs(act_pos->end_2_point->z - old_pos->end_1_point->z));
    //   //  if (len1 < len2) len += len1; else len += len2;
    //   //}
    //   //old_pos = act_pos;
    //   cout << *act_path << ",";  
    //   // cout << len << "-";
    //   ++act_path;
    //       }
    //       cout << endl;
    //     }
    ++act_endbranch;
    }

  // Get Maximum of all maximal paths (which is double contained, otherweise it would 
  // not be maximal )

  double act_max_val = 0.0;
  act_endbranch = graph->begin();
  skel_branch * act_max_node = &(*(act_endbranch));
  // initialize
  while (act_endbranch != graph->end()) {
    if (act_endbranch->max_length  >  act_max_val) {
      act_max_val = act_endbranch->max_length;
      act_max_node = &(*(act_endbranch));
      }
    ++act_endbranch;
    }
  max_node = act_max_node;
  }

void SkelGraph::Sample_along_axis(int n_dim, list<point> * axis_points)
// sample along the medial axis and perpendicular to it
  {

  if (axis_points == NULL)
    axis_points = new list<point>;  //  n_dim points

  list<int>::iterator act_path = max_node->max_path->begin();

  list<skel_branch>::iterator act_pos = graph->begin();
  advance(act_pos, *act_path - 1); 
  int from_end = 0; // holds the endpoint that we are propagating from 
  point *endpt_1, *endpt_2;
  if (act_pos->end_1_neighbors) {
    endpt_1 = act_pos->end_2_point;
    endpt_2 = act_pos->end_1_point;
    from_end = 2;
    } else {
      endpt_2 = act_pos->end_2_point;
      endpt_1 = act_pos->end_1_point;
      from_end = 1;
    }

  int cnt_samples = n_dim -1;
  double inc_dist = (max_node->max_length*0.95) / cnt_samples; 
  double samp_len = max_node->max_length*0.025; // take 5% off at ends

  double len = 0.0;

  point act_point;
  //list<point>::iterator act_point  = axis_points->begin();
  list<skel_branch>::iterator old_pos = graph->end(); 
  // sentinel, because cannot set NULL
  //cout << "ID from start_len end_len samp_len samp_coor" << endl;
  while (act_path != max_node->max_path->end()) {
    act_pos = graph->begin();
    advance(act_pos, *act_path - 1); 
    // add connection cost to prev_branch branch
    if (old_pos != graph->end()) {
      double len1 = sqrt((float)abs(act_pos->end_1_point->x - old_pos->end_2_point->x) + 
        abs(act_pos->end_1_point->y - old_pos->end_2_point->y) +
        abs(act_pos->end_1_point->z - old_pos->end_2_point->z));
      double len2 = sqrt((float)abs(act_pos->end_2_point->x - old_pos->end_1_point->x) + 
        abs(act_pos->end_2_point->y - old_pos->end_1_point->y) +
        abs(act_pos->end_2_point->z - old_pos->end_1_point->z));
      if (len1 < len2) {
        len += len1; 
        endpt_2 = act_pos->end_2_point;
        endpt_1 = act_pos->end_1_point;
        from_end = 1;
        } else { 
          len += len2; 
          endpt_1 = act_pos->end_2_point;
          endpt_2 = act_pos->end_1_point;
          from_end = 2;
        }
      }
    while (len + act_pos->length >= samp_len) { // next sample found
      //cout << act_pos->branchID << " ,"  ;
      cnt_samples --;
      // interpolate position
      double factor;
      if (act_pos->length)
        factor = (samp_len - len) / act_pos->length ; // interpolation factor
      else
        factor = 0.0 ; // interpolation factor

      //cout << "f = " << factor << ","; 
      act_point.x = (int) (endpt_1->x + factor*(endpt_2->x - endpt_1->x)); 
      act_point.y = (int) (endpt_1->y + factor*(endpt_2->y - endpt_1->y)); 
      act_point.z = (int) (endpt_1->z + factor*(endpt_2->z - endpt_1->z)); 
      //cout  << len + act_pos->length << "," << samp_len << "," << act_pos->length 
      //      << " - "
      //      << act_point->x << "," << act_point->y << "," << act_point->z 
      //            << endl;
      samp_len += inc_dist;
      axis_points->push_back(act_point);
      } 
    len += act_pos->length;
    old_pos = act_pos;
    ++act_path;
    }

  //   if (DEBUG_VSKEL) {
  //     act_point  = axis_points->begin();
  //     while (act_point != axis_points->end() ) {
  //       cout  << "(" << act_point->x << "," << act_point->y 
  //             << "," << act_point->z << "), ";
  //       ++act_point;
  //     }
  //     cout << endl;
  //   }
  }

// -------------------------------------------------------------------------
// Private Methods
// -------------------------------------------------------------------------

void SkelGraph::find_endpoints()
// find all endpoints in image
  {
  point elem;

  endpoints = new list<point>;

  //search image

  for (int x = 1; x < dim[0]-1; x++) {
    for (int y = 1; y < dim[1]-1; y++) {
      for (int z = 1; z < dim[2]-1; z++) {
        if (image[x + dim[0] * ( y + dim[1]*z )] && endpoint_Test(x,y,z)) {
          // x,y,z is an endpoint
          elem.x = x;
          elem.y = y;
          elem.z = z;
          endpoints->insert(endpoints->end(),elem);
          //appends copy of elem at end
          }
        }
      }
    }

  }

int SkelGraph::endpoint_Test(int x, int y, int z)
// tests whether (x,y,z) is an endpoint
  {
  int pz = z-1;

  int nb = 0;
  for (int i = 0; i < 3; i++) {
    int py = y-1;
    for (int j = 0; j < 3; j++) {
      int px = x-1;
      for (int k = 0; k < 3; k++) {
        if (image[px + dim[0] * ( py + dim[1] * pz )]) {
          nb++;
          }
        px++;
        } 
      py++;
      } 
    pz++;
    } 

  return (nb == 2); // == 1 neighbor 

  }

void SkelGraph::Add_new_elem_to_todo(skel_branch * &newElem)
// adds a new element with default values to To_do list
  {
  skel_branch new_elem;

  new_elem.branchID = to_do->size() + graph->size() + 1;
  new_elem.length = 0.0;
  new_elem.end_1_neighbors = NULL;
  new_elem.end_2_neighbors = NULL;
  new_elem.end_1_point = new point;
  new_elem.end_2_point = new point;
  new_elem.end_1_point->x = 0; new_elem.end_1_point->y = 0; 
  new_elem.end_1_point->z = 0;
  new_elem.end_2_point->x = 0; new_elem.end_2_point->y = 0; 
  new_elem.end_2_point->z = 0;

  new_elem.acc_length = new_elem.max_length = 0.0;
  new_elem.acc_path = new_elem.max_path = NULL;

  newElem = &(*(to_do->insert(to_do->end(),new_elem)));

  }

void SkelGraph::get_valid_neighbors(point *act_point, list<point> * &neighbors)
// returns a list of valid neighbors at act_point
// points that exist in skeleton, but are yet unlabeled
  {
  point pt_elem;

  int pz = act_point->z - 1;

  int nb = 0;
  for (int i = 0; i < 3; i++) {
    int py = act_point->y - 1;
    for (int j = 0; j < 3; j++) {
      int px = act_point->x - 1;
      for (int k = 0; k < 3; k++) {
        if (image[px + dim[0] * ( py + dim[1] * pz )] && 
          ! label_image[px + dim[0] * ( py + dim[1] * pz )] ) {
            // point exist in skeleton, but is yet unlabeled
            pt_elem.x = px; pt_elem.y = py; pt_elem.z = pz;
            neighbors->push_back(pt_elem);
          }
        px++;
        } 
      py++;
      } 
    pz++;
    } 

  //   if (DEBUG_VSKEL) { 
  //     list<point>::iterator act_neighbor = neighbors->begin();
  //     int num_nb = neighbors->size();
  //     int i = 0;
  //     while (act_neighbor != neighbors->end()) {
  //       i++;
  //       point * pt = &(*act_neighbor);
  //       cout << i << ".Neighbour: "<< pt->x << "," << pt->y << "," << pt->z << endl;
  //       ++act_neighbor;
  //     }
  //   }

  }
