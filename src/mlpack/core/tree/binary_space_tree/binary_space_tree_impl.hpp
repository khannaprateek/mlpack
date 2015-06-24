/**
 * @file binary_space_tree_impl.hpp
 *
 * Implementation of generalized space partitioning tree.
 */
#ifndef __MLPACK_CORE_TREE_BINARY_SPACE_TREE_BINARY_SPACE_TREE_IMPL_HPP
#define __MLPACK_CORE_TREE_BINARY_SPACE_TREE_BINARY_SPACE_TREE_IMPL_HPP

// In case it wasn't included already for some reason.
#include "binary_space_tree.hpp"

#include <mlpack/core/util/cli.hpp>
#include <mlpack/core/util/log.hpp>
#include <mlpack/core/util/string_util.hpp>
#include <queue>

namespace mlpack {
namespace tree {

// Each of these overloads is kept as a separate function to keep the overhead
// from the two std::vectors out, if possible.
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    const MatType& data,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(NULL),
    begin(0), /* This root node starts at index 0, */
    count(data.n_cols), /* and spans all of the dataset. */
    bound(data.n_rows),
    parentDistance(0), // Parent distance for the root is 0: it has no parent.
    dataset(new MatType(data)) // Copies the dataset.
{
  // Do the actual splitting of this node.
  SplitType splitter;
  SplitNode(maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    const MatType& data,
    std::vector<size_t>& oldFromNew,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(NULL),
    begin(0),
    count(data.n_cols),
    bound(data.n_rows),
    parentDistance(0), // Parent distance for the root is 0: it has no parent.
    dataset(new MatType(data)) // Copies the dataset.
{
  // Initialize oldFromNew correctly.
  oldFromNew.resize(data.n_cols);
  for (size_t i = 0; i < data.n_cols; i++)
    oldFromNew[i] = i; // Fill with unharmed indices.

  // Now do the actual splitting.
  SplitType splitter;
  SplitNode(oldFromNew, maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    const MatType& data,
    std::vector<size_t>& oldFromNew,
    std::vector<size_t>& newFromOld,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(NULL),
    begin(0),
    count(data.n_cols),
    bound(data.n_rows),
    parentDistance(0), // Parent distance for the root is 0: it has no parent.
    dataset(new MatType(data)) // Copies the dataset.
{
  // Initialize the oldFromNew vector correctly.
  oldFromNew.resize(data.n_cols);
  for (size_t i = 0; i < data.n_cols; i++)
    oldFromNew[i] = i; // Fill with unharmed indices.

  // Now do the actual splitting.
  SplitType splitter;
  SplitNode(oldFromNew, maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);

  // Map the newFromOld indices correctly.
  newFromOld.resize(data.n_cols);
  for (size_t i = 0; i < data.n_cols; i++)
    newFromOld[oldFromNew[i]] = i;
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    BinarySpaceTree* parent,
    const size_t begin,
    const size_t count,
    SplitType& splitter,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(parent),
    begin(begin),
    count(count),
    bound(parent->Dataset().n_rows),
    dataset(&parent->Dataset()) // Point to the parent's dataset.
{
  // Perform the actual splitting.
  SplitNode(maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    BinarySpaceTree* parent,
    const size_t begin,
    const size_t count,
    std::vector<size_t>& oldFromNew,
    SplitType& splitter,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(parent),
    begin(begin),
    count(count),
    bound(parent->Dataset().n_rows),
    dataset(&parent->Dataset())
{
  // Hopefully the vector is initialized correctly!  We can't check that
  // entirely but we can do a minor sanity check.
  assert(oldFromNew.size() == dataset->n_cols);

  // Perform the actual splitting.
  SplitNode(oldFromNew, maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    BinarySpaceTree* parent,
    const size_t begin,
    const size_t count,
    std::vector<size_t>& oldFromNew,
    std::vector<size_t>& newFromOld,
    SplitType& splitter,
    const size_t maxLeafSize) :
    left(NULL),
    right(NULL),
    parent(parent),
    begin(begin),
    count(count),
    bound(parent->Dataset()->n_rows),
    dataset(&parent->Dataset())
{
  // Hopefully the vector is initialized correctly!  We can't check that
  // entirely but we can do a minor sanity check.
  Log::Assert(oldFromNew.size() == dataset->n_cols);

  // Perform the actual splitting.
  SplitNode(oldFromNew, maxLeafSize, splitter);

  // Create the statistic depending on if we are a leaf or not.
  stat = StatisticType(*this);

  // Map the newFromOld indices correctly.
  newFromOld.resize(dataset->n_cols);
  for (size_t i = 0; i < dataset->n_cols; i++)
    newFromOld[oldFromNew[i]] = i;
}

/**
 * Create a binary space tree by copying the other tree.  Be careful!  This can
 * take a long time and use a lot of memory.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::BinarySpaceTree(
    const BinarySpaceTree& other) :
    left(NULL),
    right(NULL),
    parent(other.parent),
    begin(other.begin),
    count(other.count),
    bound(other.bound),
    stat(other.stat),
    parentDistance(other.parentDistance),
    furthestDescendantDistance(other.furthestDescendantDistance),
    // Copy matrix, but only if we are the root.
    dataset((other.parent == NULL) ? new MatType(*other.dataset) : NULL)
{
  // Create left and right children (if any).
  if (other.Left())
  {
    left = new BinarySpaceTree(*other.Left());
    left->Parent() = this; // Set parent to this, not other tree.
  }

  if (other.Right())
  {
    right = new BinarySpaceTree(*other.Right());
    right->Parent() = this; // Set parent to this, not other tree.
  }

  // Propagate matrix, but only if we are the root.
  if (parent == NULL)
  {
    std::queue<BinarySpaceTree*> queue;
    if (left)
      queue.push(left);
    if (right)
      queue.push(right);
    while (!queue.empty())
    {
      BinarySpaceTree* node = queue.front();
      queue.pop();

      node->dataset = dataset;
      if (node->left)
        queue.push(node->left);
      if (node->right)
        queue.push(node->right);
    }
  }
}

/**
 * Initialize the tree from an archive.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
template<typename Archive>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
BinarySpaceTree(
    Archive& ar,
    const typename boost::enable_if<typename Archive::is_loading>::type*) :
    BinarySpaceTree() // Create an empty BinarySpaceTree.
{
  // We've delegated to the constructor which gives us an empty tree, and now we
  // can serialize from it.
  ar >> data::CreateNVP(*this, "tree");
}

/**
 * Deletes this node, deallocating the memory for the children and calling their
 * destructors in turn.  This will invalidate any pointers or references to any
 * nodes which are children of this one.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
  ~BinarySpaceTree()
{
  if (left)
    delete left;
  if (right)
    delete right;

  // If we're the root, delete the matrix.
  if (!parent)
    delete dataset;
}

/**
 * Find a node in this tree by its begin and count.
 *
 * Every node is uniquely identified by these two numbers.
 * This is useful for communicating position over the network,
 * when pointers would be invalid.
 *
 * @param queryBegin The Begin() of the node to find.
 * @param queryCount The Count() of the node to find.
 * @return The found node, or NULL if nothing is found.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
const BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>*
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::FindByBeginCount(
    size_t queryBegin,
    size_t queryCount) const
{
  Log::Assert(queryBegin >= begin);
  Log::Assert(queryCount <= count);

  if (begin == queryBegin && count == queryCount)
    return this;
  else if (IsLeaf())
    return NULL;
  else if (queryBegin < right->Begin())
    return left->FindByBeginCount(queryBegin, queryCount);
  else
    return right->FindByBeginCount(queryBegin, queryCount);
}

/**
 * Find a node in this tree by its begin and count (const).
 *
 * Every node is uniquely identified by these two numbers.
 * This is useful for communicating position over the network,
 * when pointers would be invalid.
 *
 * @param queryBegin the Begin() of the node to find
 * @param queryCount the Count() of the node to find
 * @return the found node, or NULL
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>*
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::FindByBeginCount(
    const size_t queryBegin,
    const size_t queryCount)
{
  mlpack::Log::Assert(begin >= queryBegin);
  mlpack::Log::Assert(count <= queryCount);

  if (begin == queryBegin && count == queryCount)
    return this;
  else if (IsLeaf())
    return NULL;
  else if (queryBegin < left->End())
    return left->FindByBeginCount(queryBegin, queryCount);
  else if (right)
    return right->FindByBeginCount(queryBegin, queryCount);
  else
    return NULL;
}

/* TODO: we can likely calculate this earlier, then store the
 *   result in a private member variable; for now, we can
 *   just calculate as needed...
 *
 *   Also, perhaps we should rewrite these recursive functions
 *     to avoid exceeding the stack limit
 */

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    TreeSize() const
{
  // Recursively count the nodes on each side of the tree.  The plus one is
  // because we have to count this node, too.
  return 1 + (left ? left->TreeSize() : 0) + (right ? right->TreeSize() : 0);
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    TreeDepth() const
{
  // Recursively count the depth on each side of the tree.  The plus one is
  // because we have to count this node, too.
  return 1 + std::max((left ? left->TreeDepth() : 0),
                      (right ? right->TreeDepth() : 0));
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline bool BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    IsLeaf() const
{
  return !left;
}

/**
 * Returns the number of children in this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    NumChildren() const
{
  if (left && right)
    return 2;
  if (left)
    return 1;

  return 0;
}

/**
 * Return a bound on the furthest point in the node from the centroid.  This
 * returns 0 unless the node is a leaf.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline double BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    FurthestPointDistance() const
{
  if (!IsLeaf())
    return 0.0;

  // Otherwise return the distance from the centroid to a corner of the bound.
  return 0.5 * bound.Diameter();
}

/**
 * Return the furthest possible descendant distance.  This returns the maximum
 * distance from the centroid to the edge of the bound and not the empirical
 * quantity which is the actual furthest descendant distance.  So the actual
 * furthest descendant distance may be less than what this method returns (but
 * it will never be greater than this).
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline double BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    FurthestDescendantDistance() const
{
  return furthestDescendantDistance;
}

//! Return the minimum distance from the center to any bound edge.
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline double BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    MinimumBoundDistance() const
{
  return bound.MinWidth() / 2.0;
}

/**
 * Return the specified child.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>&
    BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
        Child(const size_t child) const
{
  if (child == 0)
    return *left;
  else
    return *right;
}

/**
 * Return the number of points contained in this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    NumPoints() const
{
  if (left)
    return 0;

  return count;
}

/**
 * Return the number of descendants contained in the node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    NumDescendants() const
{
  return count;
}

/**
 * Return the index of a particular descendant contained in this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    Descendant(const size_t index) const
{
  return (begin + index);
}

/**
 * Return the number of descendant nodes of this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    NumDescendantNodes() const
{
  // The lack of caching here means this may be slow to calculate...
  if (left && right)
    return 2 + left->NumDescendantNodes() + right->NumDescendantNodes();
  else if (left)
    return 1 + left->NumDescendantNodes();
  else if (right)
    return 1 + right->NumDescendantNodes();
  else
    return 0;
}

/**
 * Return a particular descendant node of this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>&
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::DescendantNode(
    const size_t index) const
{
  // I don't really like this strategy.  We're going to do a breadth-first
  // traversal until we find the descendant we're looking for.  This could be
  // disastrously slow...
  std::queue<BinarySpaceTree*> queue;
  size_t currentIndex = 0;

  // Add first-level descendant nodes.
  if (left)
    queue.push(left);
  if (right)
    queue.push(right);

  // Iterate over all descendants.
  while (!queue.empty())
  {
    BinarySpaceTree* node = queue.front();
    queue.pop();

    if (currentIndex == index)
      return *node;

    if (node->Left())
      queue.push(node->Left());
    if (node->Right())
      queue.push(node->Right());
    ++currentIndex;
  }

  // If we get to here, then the user must have passed a bad argument.
  throw std::invalid_argument("BinarySpaceTree::DescendantNode(): invalid "
      "index");
}

/**
 * Return the index of a particular point contained in this node.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    Point(const size_t index) const
{
  return (begin + index);
}

/**
 * Gets the index one beyond the last index in the series.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
inline size_t BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    End() const
{
  return begin + count;
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
void BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::SplitNode(
    const size_t maxLeafSize,
    SplitType& splitter)
{
  // We need to expand the bounds of this node properly.
  bound |= dataset->cols(begin, begin + count - 1);

  // Calculate the furthest descendant distance.
  furthestDescendantDistance = 0.5 * bound.Diameter();

  // Now, check if we need to split at all.
  if (count <= maxLeafSize)
    return; // We can't split this.

  // splitCol denotes the two partitions of the dataset after the split. The
  // points on its left go to the left child and the others go to the right
  // child.
  size_t splitCol;

  // Split the node. The elements of 'data' are reordered by the splitting
  // algorithm. This function call updates splitCol.
  const bool split = splitter.SplitNode(bound, *dataset, begin, count,
      splitCol);

  // The node may not be always split. For instance, if all the points are the
  // same, we can't split them.
  if (!split)
    return;

  // Now that we know the split column, we will recursively split the children
  // by calling their constructors (which perform this splitting process).
  left = new BinarySpaceTree(this, begin, splitCol - begin, splitter,
      maxLeafSize);
  right = new BinarySpaceTree(this, splitCol, begin + count - splitCol,
      splitter, maxLeafSize);

  // Calculate parent distances for those two nodes.
  arma::vec centroid, leftCentroid, rightCentroid;
  Centroid(centroid);
  left->Centroid(leftCentroid);
  right->Centroid(rightCentroid);

  const double leftParentDistance = bound.Metric().Evaluate(centroid,
      leftCentroid);
  const double rightParentDistance = bound.Metric().Evaluate(centroid,
      rightCentroid);

  left->ParentDistance() = leftParentDistance;
  right->ParentDistance() = rightParentDistance;
}

template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
void BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::SplitNode(
    std::vector<size_t>& oldFromNew,
    const size_t maxLeafSize,
    SplitType& splitter)
{
  // This should be a single function for Bound.
  // We need to expand the bounds of this node properly.
  bound |= dataset->cols(begin, begin + count - 1);

  // Calculate the furthest descendant distance.
  furthestDescendantDistance = 0.5 * bound.Diameter();

  // First, check if we need to split at all.
  if (count <= maxLeafSize)
    return; // We can't split this.

  // splitCol denotes the two partitions of the dataset after the split. The
  // points on its left go to the left child and the others go to the right
  // child.
  size_t splitCol;

  // Split the node. The elements of 'data' are reordered by the splitting
  // algorithm. This function call updates splitCol and oldFromNew.
  const bool split = splitter.SplitNode(bound, *dataset, begin, count, splitCol,
      oldFromNew);

  // The node may not be always split. For instance, if all the points are the
  // same, we can't split them.
  if (!split)
    return;

  // Now that we know the split column, we will recursively split the children
  // by calling their constructors (which perform this splitting process).
  left = new BinarySpaceTree(this, begin, splitCol - begin, oldFromNew,
      splitter, maxLeafSize);
  right = new BinarySpaceTree(this, splitCol, begin + count - splitCol,
      oldFromNew, splitter, maxLeafSize);

  // Calculate parent distances for those two nodes.
  arma::vec centroid, leftCentroid, rightCentroid;
  Centroid(centroid);
  left->Centroid(leftCentroid);
  right->Centroid(rightCentroid);

  const double leftParentDistance = bound.Metric().Evaluate(centroid,
      leftCentroid);
  const double rightParentDistance = bound.Metric().Evaluate(centroid,
      rightCentroid);

  left->ParentDistance() = leftParentDistance;
  right->ParentDistance() = rightParentDistance;
}

// Default constructor (private), for boost::serialization.
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    BinarySpaceTree() :
    left(NULL),
    right(NULL),
    parent(NULL),
    begin(0),
    count(0),
    stat(*this),
    parentDistance(0),
    furthestDescendantDistance(0),
    dataset(NULL)
{
  // Nothing to do.
}

/**
 * Serialize the tree.  This overload is necessary to make the SFINAE for
 * boost::serialization work (we must match the signature that is requried
 * exactly).
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
template<typename Archive>
void BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::Serialize(
    Archive& ar,
    const unsigned int version)
{
  Serialize(ar, version, 0);
}

/**
 * Serialize the tree.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
template<typename Archive>
void BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::Serialize(
    Archive& ar,
    const unsigned int /* version */,
    const unsigned int maxDepth)
{
  using data::CreateNVP;

  // If we're loading, and we have children, they need to be deleted.
  if (Archive::is_loading::value)
  {
    if (left)
      delete left;
    if (right)
      delete right;
    if (!parent)
      delete dataset;
  }

  // Note that we don't save the parent.  This is because we re-link the parent
  // after we load the children.  This helps avoid duplications of the tree by
  // boost::serialization.
  ar & CreateNVP(begin, "begin");
  ar & CreateNVP(count, "count");
  ar & CreateNVP(bound, "bound");
  ar & CreateNVP(stat, "statistic");
  ar & CreateNVP(parentDistance, "parentDistance");
  ar & CreateNVP(furthestDescendantDistance, "furthestDescendantDistance");
  ar & CreateNVP(dataset, "dataset");

  // This is a hack to allow saving only certain levels of the tree: we will
  // temporarily modify parentDistance (this member was chosen arbitrarily) to
  // hold the number of levels remaining.  The child will check the parent's
  // parentDistance member, to see if it should continue to recurse.
  const double oldParentDistance = parentDistance;
  if (Archive::is_saving::value)
  {
    // If we are the root, we must set the maximum depth.
    if (parent == NULL)
      parentDistance = double(maxDepth) - 1; // This can be negative.
    else
      parentDistance = parent->ParentDistance() - 1;

    BinarySpaceTree* oldLeft;
    BinarySpaceTree* oldRight;
    if (std::abs(parentDistance) < 1e-10) // Tolerance for floating-point...
    {
      // Hide the children so we don't recurse.
      oldLeft = left;
      oldRight = right;
      left = NULL;
      right = NULL;
    }

    ar & CreateNVP(left, "left");
    ar & CreateNVP(right, "right");

    // Restore the children, if we hid them.
    if (std::abs(parentDistance) < 1e-10)
    {
      left = oldLeft;
      right = oldRight;
    }

    // Restore parent distance.
    parentDistance = oldParentDistance;
  }
  else
  {
    // We are loading; nothing special needs to happen here.
    ar & CreateNVP(left, "left");
    ar & CreateNVP(right, "right");

    // Link the children to the parent.
    if (left)
      left->Parent() = this;
    if (right)
      right->Parent() = this;
  }
}

/**
 * Returns a string representation of this object.
 */
template<typename BoundType,
         typename StatisticType,
         typename MatType,
         typename SplitType>
std::string BinarySpaceTree<BoundType, StatisticType, MatType, SplitType>::
    ToString() const
{
  std::ostringstream convert;
  convert << "BinarySpaceTree [" << this << "]" << std::endl;
  convert << "  First point: " << begin << std::endl;
  convert << "  Number of descendants: " << count << std::endl;
  convert << "  Bound: " << std::endl;
  convert << mlpack::util::Indent(bound.ToString(), 2);
  convert << "  Statistic: " << std::endl;
  convert << mlpack::util::Indent(stat.ToString(), 2);

  // How many levels should we print?  This will print the top two tree levels.
  if (left != NULL && parent == NULL)
  {
    convert << " Left child:" << std::endl;
    convert << mlpack::util::Indent(left->ToString(), 2);
  }
  if (right != NULL && parent == NULL)
  {
    convert << " Right child:" << std::endl;
    convert << mlpack::util::Indent(right->ToString(), 2);
  }
  return convert.str();
}

}; // namespace tree
}; // namespace mlpack

#endif
