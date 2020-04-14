//***************************************************************************
// This Fibonacci heap implementation is Copyright (c) 1996 by John Boyer.
// See the header file for free usage information.
//***************************************************************************

#include "FibHeap.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

#define OK      0
#define NOTOK   -1

const NodeIndexType FibHeapNode::NullNodeIndex = 0xFFFFFFFF;
const NodeKeyValueType FibHeapNode::NegativeInfinity = -std::numeric_limits<NodeKeyValueType>::infinity();

//-----------------------------------------------------------------------------
FibHeap::FibHeap()
{
  m_HeapNodes = nullptr;
  m_MinRoot = nullptr;
  m_NumNodes = m_NumTrees = m_NumMarkedNodes = 0;
}

//-----------------------------------------------------------------------------
FibHeap::~FibHeap() = default;

//-----------------------------------------------------------------------------
void FibHeap::SetHeapNodes(FibHeapNode* heapNodes)
{
  m_HeapNodes = heapNodes;
}

//-----------------------------------------------------------------------------
void FibHeap::Insert(FibHeapNode* NewNode)
{
  if (NewNode == nullptr)
  {
    return;
  }

  if (m_MinRoot == nullptr)
  {
    // If the heap is currently empty, then new node becomes singleton
    // circular root list
    m_MinRoot = NewNode;
    NewNode->m_Left = NewNode->m_Right = NewNode->m_Index;
  }
  else
  {
    // Pointers from NewNode set to insert between m_MinRoot and m_MinRoot->m_Right
    NewNode->m_Right = m_MinRoot->m_Right;
    NewNode->m_Left = m_MinRoot->GetIndexValue();

    // Set Pointers to NewNode
    m_HeapNodes[NewNode->m_Left].m_Right = NewNode->m_Index;
    m_HeapNodes[NewNode->m_Right].m_Left = NewNode->m_Index;

    // The new node becomes new m_MinRoot if it is less than current m_MinRoot
    if (*NewNode < *m_MinRoot)
    {
      m_MinRoot = NewNode;
    }
  }

  // We have one more node in the heap, and it is a tree on the root list
  m_NumNodes++;
  m_NumTrees++;
  NewNode->m_Parent = FibHeapNode::NullNodeIndex;
}

//-----------------------------------------------------------------------------
void FibHeap::Union(FibHeap *OtherHeap)
{
  if (OtherHeap == nullptr || OtherHeap->m_MinRoot == nullptr)
  {
    return;
  }

  // We join the two circular lists by cutting each list between its
  // min node and the node after the min.  This code just pulls those
  // nodes into temporary variables so we don't get lost as changes
  // are made.
  FibHeapNode* Min1 = m_MinRoot;
  FibHeapNode* Min2 = OtherHeap->m_MinRoot;
  FibHeapNode* Next1 = HeapNodeFromIndex(Min1->m_Right);
  FibHeapNode* Next2 = HeapNodeFromIndex(Min2->m_Right);

  // To join the two circles, we join the minimum nodes to the next
  // nodes on the opposite chains.  Conceptually, it looks like the way
  // two bubbles join to form one larger bubble.  They meet at one point
  // of contact, then expand out to make the bigger circle.
  Min1->m_Right = Next2->m_Index;
  Next2->m_Left = Min1->m_Index;
  Min2->m_Right = Next1->m_Index;
  Next1->m_Left = Min2->m_Index;

  // Choose the new minimum for the heap
  if (*Min2 < *Min1)
  {
    m_MinRoot = Min2;
  }

  // Set the amortized analysis statistics and size of the new heap
  m_NumNodes += OtherHeap->m_NumNodes;
  m_NumMarkedNodes += OtherHeap->m_NumMarkedNodes;
  m_NumTrees += OtherHeap->m_NumTrees;

  // Complete the union by setting the other heap to emptiness
  // then destroying it
  OtherHeap->m_MinRoot  = nullptr;
  OtherHeap->m_NumNodes = 0;
  OtherHeap->m_NumTrees = 0;
  OtherHeap->m_NumMarkedNodes = 0;

  delete OtherHeap;
}

//-----------------------------------------------------------------------------
FibHeapNode *FibHeap::ExtractMin()
{
  FibHeapNode *Result;
  FibHeap *ChildHeap = nullptr;

  // Remove minimum node and set m_MinRoot to next node
  if ((Result = Minimum()) == nullptr)
  {
    return nullptr;
  }

  m_MinRoot = HeapNodeFromIndex(Result->m_Right);
  m_HeapNodes[Result->m_Right].m_Left = Result->m_Left;
  m_HeapNodes[Result->m_Left].m_Right = Result->m_Right;
  Result->m_Left = Result->m_Right = FibHeapNode::NullNodeIndex;

  m_NumNodes --;
  if (Result->m_Mark)
  {
    m_NumMarkedNodes --;
    Result->m_Mark = false;
  }
  Result->m_Degree = 0;

  // Attach child list of Minimum node to the root list of the heap
  if (Result->m_Child == FibHeapNode::NullNodeIndex)
  {
    // If there is no child list, then do no work
    if (m_MinRoot == Result)
    {
      m_MinRoot = nullptr;
    }
  }
  else if (m_MinRoot == Result)
  {
    // If m_MinRoot==Result then there was only one root tree, so the
    // root list is simply the child list of that node (which is
    // nullptr if this is the last node in the list)
    m_MinRoot = HeapNodeFromIndex(Result->m_Child);
  }
  else
  {
    // If m_MinRoot is different, then the child list is pushed into a
    // new temporary heap, which is then merged by Union() onto the
    // root list of this heap.
    ChildHeap = new FibHeap();
    ChildHeap->m_MinRoot = HeapNodeFromIndex(Result->m_Child);
  }

  // Complete the disassociation of the Result node from the heap
  if (Result->m_Child != FibHeapNode::NullNodeIndex)
  {
    m_HeapNodes[Result->m_Child].m_Parent = FibHeapNode::NullNodeIndex;
  }
  Result->m_Child = Result->m_Parent = FibHeapNode::NullNodeIndex;

  // If there was a child list, then we now merge it with the
  //  rest of the root list
  if (ChildHeap)
  {
    Union(ChildHeap);
  }

  // Consolidate heap to find new minimum and do reorganize work
  if (m_MinRoot != nullptr)
  {
    Consolidate();
  }

  // Return the minimum node, which is now disassociated with the heap
  // It has m_Left, m_Right, m_Parent, m_Child, m_Mark and m_Degree cleared.
  return Result;
}

//-----------------------------------------------------------------------------
int  FibHeap::DecreaseKey(FibHeapNode* theNode, NodeKeyValueType keyValue)
{
  if (theNode == nullptr || theNode->m_Key < keyValue)
  {
    return NOTOK;
  }

  (*theNode) = keyValue;

  FibHeapNode* theParent = HeapNodeFromIndex(theNode->m_Parent);
  if (theParent != nullptr && *theNode < *theParent)
  {
    Cut(theNode, theParent);
    CascadingCut(theParent);
  }

  if (*theNode < *m_MinRoot)
  {
    m_MinRoot = theNode;
  }

  return OK;
}

//-----------------------------------------------------------------------------
int  FibHeap::Delete(FibHeapNode *theNode)
{
  if (theNode == nullptr)
  {
    return NOTOK;
  }

  int result = DecreaseKey(theNode, FibHeapNode::NegativeInfinity);
  if (result == OK)
  {
    if (ExtractMin() == nullptr)
    {
      result = NOTOK;
    }
  }
  /*
  if (result == OK)
  {
    theNode->m_NegInfinityFlag = 0;
  }
  */

  return result;
}

//-----------------------------------------------------------------------------
void FibHeap::Print(FibHeapNode *tree, FibHeapNode *theParent)
{
  if (tree == nullptr)
  {
    tree = m_MinRoot;
  }

  FibHeapNode* temp = tree;
  do
  {
    if (temp->m_Left == FibHeapNode::NullNodeIndex)
    {
      cout << "(m_Left is nullptr)";
    }
    temp->Print();
    if (temp->m_Parent != theParent->m_Index)
    {
      cout << "(m_Parent is incorrect)";
    }
    if (temp->m_Right == FibHeapNode::NullNodeIndex)
    {
      cout << "(m_Right is nullptr)";
    }
    else if (m_HeapNodes[temp->m_Right].m_Left != temp->m_Index)
    {
      cout << "(Error in left link left) ->";
    }
    else
    {
      cout << " <-> ";
    }

    temp = HeapNodeFromIndex(temp->m_Right);
    /*
    if (kbhit() && getch() == 27)
    {
    cout << "Hit a key to resume or ESC to break\n";
    if (getch() == 27)
    break;
    }
    */
  } while (temp != nullptr && temp != tree);
  cout << '\n';

  temp = tree;
  do
  {
    cout << "Children of ";
    temp->Print();
    cout << ": ";
    if (temp->m_Child == FibHeapNode::NullNodeIndex)
    {
      cout << "NONE\n";
    }
    else
    {
      Print(HeapNodeFromIndex(temp->m_Child), temp);
    }
    temp = HeapNodeFromIndex(temp->m_Right);
  } while (temp !=nullptr && temp != tree);

  if (theParent == nullptr)
  {
    char ch;
    cout << "Done Printing.  Hit a key.\n";
    cin >> ch;
  }
}

//-----------------------------------------------------------------------------
void FibHeap::Consolidate()
{
  // Initialize the consolidation detection array
  const int Dn = 1 + 8 * sizeof(long);
  FibHeapNode *A[Dn]; // 1+lg(n)
  for (int i=0; i < Dn; i++)
  {
    A[i] = nullptr;
  }

  // We need to loop through all elements on root list.
  // When a collision of degree is found, the two trees
  // are consolidated in favor of the one with the lesser
  // element key value.  We first need to break the circle
  // so that we can have a stopping condition (we can't go
  // around until we reach the tree we started with
  // because all root trees are subject to becoming a
  // child during the consolidation).

  m_HeapNodes[m_MinRoot->m_Left].m_Right = FibHeapNode::NullNodeIndex;
  m_MinRoot->m_Left = FibHeapNode::NullNodeIndex;
  FibHeapNode* w = m_MinRoot;

  FibHeapNode* x;
  FibHeapNode* y;
  short d;
  do
  {
    //cout << "Top of Consolidate's loop\n";
    //Print(w);

    x = w;
    d = x->m_Degree;
    w = HeapNodeFromIndex(w->m_Right);

    // We need another loop here because the consolidated result
    // may collide with another large tree on the root list.

    while (A[d] != nullptr)
    {
      y = A[d];
      if (*y < *x)
      {
        Exchange(x, y);
      }
      if (w == y)
      {
        w = HeapNodeFromIndex(y->m_Right);
      }
      Link(y, x);
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
  m_MinRoot = nullptr;
  m_NumTrees = 0;
  for (int i = 0; i < Dn; i++)
  {
    if (A[i] != nullptr)
    {
      AddToRootList(A[i]);
    }
  }
}

//-----------------------------------------------------------------------------
void FibHeap::Link(FibHeapNode *y, FibHeapNode *x)
{
  // Remove node y from root list
  if (y->m_Right != FibHeapNode::NullNodeIndex)
  {
    m_HeapNodes[y->m_Right].m_Left = y->m_Left;
  }
  if (y->m_Left != FibHeapNode::NullNodeIndex)
  {
    m_HeapNodes[y->m_Left].m_Right = y->m_Right;
  }
  m_NumTrees--;

  // Make node y a singleton circular list with a parent of x
  y->m_Left = y->m_Right = y->m_Index;
  y->m_Parent = x->m_Index;

  if (x->m_Child == FibHeapNode::NullNodeIndex)
  {
    // If node x has no children, then list y is its new child list
    x->m_Child = y->m_Index;
  }
  else
  {
    // Otherwise, node y must be added to node x's child list
    y->m_Left = x->m_Child;
    y->m_Right = m_HeapNodes[x->m_Child].m_Right;
    m_HeapNodes[x->m_Child].m_Right = y->m_Index;
    m_HeapNodes[y->m_Right].m_Left = y->m_Index;
  }

  // Increase the degree of node x because it's now a bigger tree
  x->m_Degree ++;

  // Node y has just been made a child, so clear its mark
  if (y->m_Mark)
  {
    m_NumMarkedNodes--;
  }
  y->m_Mark = false;
}

//-----------------------------------------------------------------------------
void FibHeap::AddToRootList(FibHeapNode *x)
{
  if (x->m_Mark)
  {
    m_NumMarkedNodes--;
  }
  x->m_Mark = false;

  m_NumNodes--;
  Insert(x);
}

//-----------------------------------------------------------------------------
void FibHeap::Cut(FibHeapNode *x, FibHeapNode *y)
{
  if (y->m_Child == x->m_Index)
  {
    y->m_Child = x->m_Right;
  }
  if (y->m_Child == x->m_Index)
  {
    y->m_Child = FibHeapNode::NullNodeIndex;
  }

  y->m_Degree --;

  m_HeapNodes[x->m_Left].m_Right = x->m_Right;
  m_HeapNodes[x->m_Right].m_Left = x->m_Left;

  AddToRootList(x);
}

//-----------------------------------------------------------------------------
void FibHeap::CascadingCut(FibHeapNode *y)
{
  FibHeapNode *z = HeapNodeFromIndex(y->m_Parent);

  while (z != nullptr)
  {
    if (y->m_Mark == 0)
    {
      y->m_Mark = true;
      m_NumMarkedNodes++;
      z = nullptr;
    }
    else
    {
      Cut(y, z);
      y = z;
      z = HeapNodeFromIndex(y->m_Parent);
    }
  }
}
