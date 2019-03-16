//***************************************************************************
// This Fibonacci heap implementation is Copyright (c) 1996 by John Boyer.
// See the header file for free usage information.
//***************************************************************************

//***************************************************************************
// The classes in this package are designed to allow the package user
// to quickly and easily develop applications that require a heap data
// structure.  Using amortized analysis, the asymptotically fastest heap
// data structure is the Fibonacci heap.  The constants are a little
// high so the real speed gain will not be seen until larger data sets
// are required, but in most cases, if the data set is small, then the
// run-time will be negligible anyway.
//
// To use this heap class you need do only two things.  First, subclass
// the FibHeapNode class to create the class of objects that you'd
// like to store in a heap.  Second, create an instance of the FibHeap
// class, which can then be used to Insert(), ExtractMin(), etc.,
// instances of your FibHeapNode subclass.  Notice that you don't need
// to create a subclass of the FibHeap class.
//
// The application-specific data object that you'd like to store in a heap
// will have a key value.  In the class that you derive from FibHeapNode,
// you will need to define the key structure then provide assignment (=),
// equality (==) and less-than operators and a destructor.  These functions
// are declared virtual so that the code in the FibHeap class can compare,
// assign and destroy your objects by calling on your code.
//
// The overloaded operators in your defined class MUST call functions in
// the Fibonacci heap node class first.  For assignment, the function
// FHN_Assign() should be called before code that deals with the copy of
// the key value.  For comparison operators, the function FHN_Cmp() should
// appear first.  If it returns 0, then keys can be compared as normal.
// The following indicates what the three most common operators must do
// based on the return value of FHN_Cmp()
//
// For ==, if zero returned, then compare keys
//     if non-zero X returned, then return 0
// For <,  if zero returned, then compare keys
//         if non-zero X returned, then return X<0?1:0
// For >,  if zero returned, then compare keys
//         if non-zero X returned, then return X>0?1:0
//***************************************************************************


#include <cstdlib>
#include <iostream>
#include <cstdio>
#include "FibHeap.h"
using namespace std;

//***************************************************************************
//=========================================================
// FibHeapNode Constructor
//=========================================================
//***************************************************************************

//FibHeapNode::FibHeapNode() {
//  Left = Right = Parent = Child = nullptr;
//  Degree = Mark = NegInfinityFlag = 0;
//}

//=========================================================
// FibHeapNode Destructor
//
// Body is empty, but declaration is required in order to
// force virtual.  This will ensure that FibHeap class
// calls derived class destructors.
//=========================================================

FibHeapNode::~FibHeapNode()  = default;

//=========================================================
// FHN_Assign()
//
// To be used as first step of an assignment operator in a
// derived class.  The derived class will handle assignment
// of key value, and this function handles copy of the
// NegInfinityFlag (which overrides the key value if it is
// set).
//=========================================================

//void FibHeapNode::FHN_Assign(FibHeapNode& RHS) {
//  NegInfinityFlag = RHS.NegInfinityFlag;
//}

//=========================================================
// FHN_Cmp()
//
// To be used as the first step of ALL comparators in a
// derived class.
//
// Compares the relative state of the two neg. infinity
// flags.  Note that 'this' is the left hand side.  If
// LHS neg. infinity is set, then it will be less than (-1)
// the RHS unless RHS neg. infinity flag is also set.
// Only if function returns 0 should the key comparison
// defined in the derived class be performed, e.g.
//
// For ==, if zero returned, then compare keys
//     if non-zero X returned, then return 0
// For <,  if zero returned, then compare keys
//         if non-zero X returned, then return X<0?1:0
// For >,  if zero returned, then compare keys
//         if non-zero X returned, then return X>0?1:0
//=========================================================

//int  FibHeapNode::FHN_Cmp(FibHeapNode& RHS) {
//  if (NegInfinityFlag)
//    return RHS.NegInfinityFlag ? 0 : -1;
//  return RHS.NegInfinityFlag ? 1 : 0;
//}

//========================================================================
// We do, on occasion, compare and assign objects of type FibHeapNode, but
// only when the NegInfinityFlag is set.  See for example FibHeap::Delete().
//
// Also, these functions exemplify what a derived class should do.
//========================================================================

void FibHeapNode::operator =(FibHeapNode& RHS) {
  FHN_Assign(RHS);
  // Key assignment goes here in derived classes
}

int  FibHeapNode::operator ==(FibHeapNode& RHS) {
  if (FHN_Cmp(RHS)) return 0;
  // Key compare goes here in derived classes
  return 1;
}

int  FibHeapNode::operator <(FibHeapNode& RHS) {
  int X;
  if ((X=FHN_Cmp(RHS)) != 0)
    return X < 0 ? 1 : 0;
  // Key compare goes here in derived classes
  return 0;
}

//=========================================================
// Print()
//=========================================================

void FibHeapNode::Print() {
  if (NegInfinityFlag)
    cout << "-inf.";
}

//***************************************************************************
//===========================================================================
// FibHeap Constructor
//===========================================================================
//***************************************************************************

FibHeap::FibHeap() {
  MinRoot = nullptr;
  NumNodes = NumTrees = NumMarkedNodes = 0;
  ClearHeapOwnership();
}

//===========================================================================
// FibHeap Destructor
//===========================================================================

FibHeap::~FibHeap() {
  FibHeapNode *Temp;

  if (GetHeapOwnership()) {
    while (MinRoot != nullptr) {
      Temp = ExtractMin();
      delete Temp;
    }
  }
}

//===========================================================================
// Insert() - O(1) actual; O(2) amortized
//
// I am using O(2) here to indicate that although Insert() is
// constant time, its amortized rating is more costly because some
// of the work of inserting is done by other operations such as
// ExtractMin(), which is where tree-balancing occurs.
//
// The child pointer is deliberately not set to nullptr because Insert()
// is also used internally to help put whole trees onto the root list.
//===========================================================================

void FibHeap::Insert(FibHeapNode *NewNode) {
  if (NewNode == nullptr) return;

  // If the heap is currently empty, then new node becomes singleton
  // circular root list

  if (MinRoot == nullptr)
    MinRoot = NewNode->Left = NewNode->Right = NewNode;

  else {
    // Pointers from NewNode set to insert between MinRoot and MinRoot->Right

    NewNode->Right = MinRoot->Right;
    NewNode->Left = MinRoot;

    // Set Pointers to NewNode

    NewNode->Left->Right = NewNode;
    NewNode->Right->Left = NewNode;

    // The new node becomes new MinRoot if it is less than current MinRoot

    if (*NewNode < *MinRoot)
      MinRoot = NewNode;
  }

  // We have one more node in the heap, and it is a tree on the root list

  NumNodes++;

  NumTrees++;
  NewNode->Parent = nullptr;
}

//===========================================================================
// Union() - O(1) actual; O(1) amortized
//===========================================================================

void FibHeap::Union(FibHeap *OtherHeap) {
  FibHeapNode *Min1, *Min2, *Next1, *Next2;

  if (OtherHeap == nullptr || OtherHeap->MinRoot == nullptr) return;

  // We join the two circular lists by cutting each list between its
  // min node and the node after the min.  This code just pulls those
  // nodes into temporary variables so we don't get lost as changes
  // are made.

  Min1 = MinRoot;
  Min2 = OtherHeap->MinRoot;
  Next1 = Min1->Right;
  Next2 = Min2->Right;

  // To join the two circles, we join the minimum nodes to the next
  // nodes on the opposite chains.  Conceptually, it looks like the way
  // two bubbles join to form one larger bubble.  They meet at one point
  // of contact, then expand out to make the bigger circle.

  Min1->Right = Next2;
  Next2->Left = Min1;
  Min2->Right = Next1;
  Next1->Left = Min2;

  // Choose the new minimum for the heap

  if (*Min2 < *Min1)
    MinRoot = Min2;

  // Set the amortized analysis statistics and size of the new heap

  NumNodes += OtherHeap->NumNodes;
  NumMarkedNodes += OtherHeap->NumMarkedNodes;
  NumTrees += OtherHeap->NumTrees;

  // Complete the union by setting the other heap to emptiness
  // then destroying it

  OtherHeap->MinRoot  = nullptr;
  OtherHeap->NumNodes =
    OtherHeap->NumTrees =
    OtherHeap->NumMarkedNodes = 0;

  delete OtherHeap;
}

//===========================================================================
// Minimum - O(1) actual; O(1) amortized
//===========================================================================

//FibHeapNode *FibHeap::Minimum() {
//  return MinRoot;
//}

//===========================================================================
// ExtractMin() - O(n) worst-case actual; O(lg n) amortized
//===========================================================================

FibHeapNode *FibHeap::ExtractMin() {
  FibHeapNode *Result;
  FibHeap *ChildHeap = nullptr;

  // Remove minimum node and set MinRoot to next node

  if ((Result = Minimum()) == nullptr)
    return nullptr;

  MinRoot = Result->Right;
  Result->Right->Left = Result->Left;
  Result->Left->Right = Result->Right;
  Result->Left = Result->Right = nullptr;

  NumNodes --;
  if (Result->Mark) {
    NumMarkedNodes --;
    Result->Mark = 0;
  }
  Result->Degree = 0;

  // Attach child list of Minimum node to the root list of the heap
  // If there is no child list, then do no work

  if (Result->Child == nullptr) {
    if (MinRoot == Result)
      MinRoot = nullptr;
  }

  // If MinRoot==Result then there was only one root tree, so the
  // root list is simply the child list of that node (which is
  // nullptr if this is the last node in the list)

  else if (MinRoot == Result)
    MinRoot = Result->Child;

  // If MinRoot is different, then the child list is pushed into a
  // new temporary heap, which is then merged by Union() onto the
  // root list of this heap.

  else {
    ChildHeap = new FibHeap();
    ChildHeap->MinRoot = Result->Child;
  }

  // Complete the disassociation of the Result node from the heap

  if (Result->Child != nullptr)
    Result->Child->Parent = nullptr;
  Result->Child = Result->Parent = nullptr;

  // If there was a child list, then we now merge it with the
  //  rest of the root list

  if (ChildHeap)
    Union(ChildHeap);

  // Consolidate heap to find new minimum and do reorganize work

  if (MinRoot != nullptr)
    _Consolidate();

  // Return the minimum node, which is now disassociated with the heap
  // It has Left, Right, Parent, Child, Mark and Degree cleared.

  return Result;
}

//===========================================================================
// DecreaseKey() - O(lg n) actual; O(1) amortized
//
// The O(lg n) actual cost stems from the fact that the depth, and
// therefore the number of ancestor parents, is bounded by O(lg n).
//===========================================================================

int  FibHeap::DecreaseKey(FibHeapNode *theNode, FibHeapNode& NewKey) {
  FibHeapNode *theParent;

  if (theNode==nullptr || *theNode < NewKey)
    return NOTOK;

  *theNode = NewKey;

  theParent = theNode->Parent;
  if (theParent != nullptr && *theNode < *theParent) {
    _Cut(theNode, theParent);
    _CascadingCut(theParent);
  }

  if (*theNode < *MinRoot)
    MinRoot = theNode;

  return OK;
}

//===========================================================================
// Delete() - O(lg n) amortized; ExtractMin() dominates
//
// Notice that if we don't own the heap nodes, then we clear the
// NegInfinityFlag on the deleted node.  Presumably, the programmer
// will be reusing the node.
//===========================================================================

int  FibHeap::Delete(FibHeapNode *theNode) {
  FibHeapNode Temp;
  int Result;

  if (theNode == nullptr) return NOTOK;

  Temp.NegInfinityFlag = 1;
  Result = DecreaseKey(theNode, Temp);

  if (Result == OK)
    if (ExtractMin() == nullptr)
      Result = NOTOK;

  if (Result == OK) {
    if (GetHeapOwnership())
      delete theNode;
    else
      theNode->NegInfinityFlag = 0;
  }

  return Result;
}

//========================================================================
// Print()
//
// Used internally for debugging purposes.  The function prints the key
// value for each node along the root list, then it calls itself on each
// child list.
//========================================================================

void FibHeap::Print(FibHeapNode *Tree, FibHeapNode *theParent) {
  FibHeapNode* Temp = nullptr;

  if (Tree == nullptr) Tree = MinRoot;

  Temp = Tree;
  do {
    if (Temp->Left == nullptr)
      cout << "(Left is nullptr)";
    Temp->Print();
    if (Temp->Parent != theParent)
      cout << "(Parent is incorrect)";
    if (Temp->Right == nullptr)
      cout << "(Right is nullptr)";
    else if (Temp->Right->Left != Temp)
      cout << "(Error in left link left) ->";
    else cout << " <-> ";

    Temp = Temp->Right;
    /*
    if (kbhit() && getch() == 27)
    {
    cout << "Hit a key to resume or ESC to break\n";
    if (getch() == 27)
    break;
    }
    */
  } while (Temp != nullptr && Temp != Tree);
  cout << '\n';

  Temp = Tree;
  do {
    cout << "Children of ";
    Temp->Print();
    cout << ": ";
    if (Temp->Child == nullptr)
      cout << "NONE\n";
    else Print(Temp->Child, Temp);
    Temp = Temp->Right;
  } while (Temp!=nullptr && Temp != Tree);

  if (theParent == nullptr) {
    char ch;

    cout << "Done Printing.  Hit a key.\n";
    cin >> ch;
  }
}

//===========================================================================
//===========================================================================

//void FibHeap::_Exchange(FibHeapNode*& N1, FibHeapNode*& N2) {
//  FibHeapNode *Temp;
//  Temp = N1;
//  N1 = N2;
//  N2 = Temp;
//}

//===========================================================================
// Consolidate()
//
// Internal function that reorganizes heap as part of an ExtractMin().
// We must find new minimum in heap, which could be anywhere along the
// root list.  The search could be O(n) since all nodes could be on
// the root list.  So, we reorganize the tree into more of a binomial forest
// structure, and then find the new minimum on the consolidated O(lg n) sized
// root list, and in the process set each Parent pointer to nullptr, and count
// the number of resulting subtrees.
//
// Note that after a list of n inserts, there will be n nodes on the root
// list, so the first ExtractMin() will be O(n) regardless of whether or
// not we consolidate.  However, the consolidation causes subsequent
// ExtractMin() operations to be O(lg n).  Furthermore, the extra cost of
// the first ExtractMin() is covered by the higher amortized cost of
// Insert(), which is the real governing factor in how costly the first
// ExtractMin() will be.
//===========================================================================

void FibHeap::_Consolidate() {
  FibHeapNode *x, *y, *w;
  FibHeapNode *A[1+8*sizeof(long)]; // 1+lg(n)
  int  I=0, Dn = 1+8*sizeof(long);
  short d;

  // Initialize the consolidation detection array

  for (I=0; I < Dn; I++)
    A[I] = nullptr;

  // We need to loop through all elements on root list.
  // When a collision of degree is found, the two trees
  // are consolidated in favor of the one with the lesser
  // element key value.  We first need to break the circle
  // so that we can have a stopping condition (we can't go
  // around until we reach the tree we started with
  // because all root trees are subject to becoming a
  // child during the consolidation).

  MinRoot->Left->Right = nullptr;
  MinRoot->Left = nullptr;
  w = MinRoot;

  do {
    //cout << "Top of Consolidate's loop\n";
    //Print(w);

    x = w;
    d = x->Degree;
    w = w->Right;

    // We need another loop here because the consolidated result
    // may collide with another large tree on the root list.

    while (A[d] != nullptr) {
      y = A[d];
      if (*y < *x)
        _Exchange(x, y);
      if (w == y) w = y->Right;
      _Link(y, x);
      A[d] = nullptr;
      d++;
      //cout << "After a round of Linking\n";
      //Print(x);
    }
    A[d] = x;

  } while (w != nullptr);

  // Now we rebuild the root list, find the new minimum,
  // set all root list nodes' parent pointers to nullptr and
  // count the number of subtrees.

  MinRoot = nullptr;
  NumTrees = 0;
  for (I = 0; I < Dn; I++)
    if (A[I] != nullptr)
      _AddToRootList(A[I]);
}

//===========================================================================
// The node y is removed from the root list and becomes a subtree of node x.
//===========================================================================

void FibHeap::_Link(FibHeapNode *y, FibHeapNode *x) {
  // Remove node y from root list

  if (y->Right != nullptr)
    y->Right->Left = y->Left;
  if (y->Left != nullptr)
    y->Left->Right = y->Right;
  NumTrees--;

  // Make node y a singleton circular list with a parent of x

  y->Left = y->Right = y;
  y->Parent = x;

  // If node x has no children, then list y is its new child list

  if (x->Child == nullptr)
    x->Child = y;

  // Otherwise, node y must be added to node x's child list

  else {
    y->Left = x->Child;
    y->Right = x->Child->Right;
    x->Child->Right = y;
    y->Right->Left = y;
  }

  // Increase the degree of node x because it's now a bigger tree

  x->Degree ++;

  // Node y has just been made a child, so clear its mark

  if (y->Mark) NumMarkedNodes--;
  y->Mark = 0;
}

//===========================================================================
//===========================================================================

void FibHeap::_AddToRootList(FibHeapNode *x) {
  if (x->Mark) NumMarkedNodes --;
  x->Mark = 0;

  NumNodes--;
  Insert(x);
}

//===========================================================================
// Remove node x from the child list of its parent node y
//===========================================================================

void FibHeap::_Cut(FibHeapNode *x, FibHeapNode *y) {
  if (y->Child == x)
    y->Child = x->Right;
  if (y->Child == x)
    y->Child = nullptr;

  y->Degree --;

  x->Left->Right = x->Right;
  x->Right->Left = x->Left;

  _AddToRootList(x);
}

//===========================================================================
// Cuts each node in parent list, putting successive ancestor nodes on the
// root list until we either arrive at the root list or until we find an
// ancestor that is unmarked.  When a mark is set (which only happens during
// a cascading cut), it means that one child subtree has been lost; if a
// second subtree is lost later during another cascading cut, then we move
// the node to the root list so that it can be re-balanced on the next
// consolidate.
//===========================================================================

void FibHeap::_CascadingCut(FibHeapNode *y) {
  FibHeapNode *z = y->Parent;

  while (z != nullptr) {
    if (y->Mark == 0) {
      y->Mark = 1;
      NumMarkedNodes++;
      z = nullptr;
    } else {
      _Cut(y, z);
      y = z;
      z = y->Parent;
    }
  }
}
