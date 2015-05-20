/**
 * @file distributed_traversal.hpp
 * @author Ryan Curtin
 *
 * Use MPI to perform a distributed traversal.
 */
#ifndef __MLPACK_CORE_TREE_BINARY_SPACE_TREE_DISTRIBUTED_TRAVERSAL_IMPL_HPP
#define __MLPACK_CORE_TREE_BINARY_SPACE_TREE_DISTRIBUTED_TRAVERSAL_IMPL_HPP

#include "distributed_traversal.hpp"

namespace mlpack {
namespace tree {

template<typename RuleType>
DistributedBinaryTraversal::DistributedBinaryTraversal(RuleType& rule) :
    rule(rule)
{
  // Initialize a communicator?
  boost::mpi::communicator world;
}

template<typename TreeType>
void DistributedBinaryTraversal::Traverse(const size_t queryIndex,
                                          TreeType& referenceNode)
{

}

template<typename TreeType>
void DistributedBinaryTraversal::Traverse(TreeType& queryNode,
                                          TreeType& referenceNode)
{
  // Recurse until we are at the correct level for splitting.
  if (level < world.size())
  {
    
  }
  else
  {
    // Alright, now we have to ship this task off to a different process.
  }

  // Now, we will wait on the results from all processes, and then combine them.
}

} // namespace tree
} // namespace mlpack

#endif
