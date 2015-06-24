/**
 * @file distributed_binary_traversal_impl.hpp
 * @author Ryan Curtin
 *
 * Use MPI to perform a distributed traversal.
 */
#ifndef __MLPACK_CORE_TREE_BINARY_SPACE_TREE_DISTRIBUTED_BINARY_TRAVERSAL_IMPL_HPP
#define __MLPACK_CORE_TREE_BINARY_SPACE_TREE_DISTRIBUTED_BINARY_TRAVERSAL_IMPL_HPP

#include "distributed_binary_traversal.hpp"
#include "../binary_space_tree.hpp"
#include "dual_tree_traverser.hpp"
#include <boost/mpi.hpp>

namespace mlpack {
namespace tree {

template<typename RuleType>
DistributedBinaryTraversal<RuleType>::DistributedBinaryTraversal(
    RuleType& rule) :
    rule(&rule),
    world()
{
  // Nothing to do.
}

template<typename RuleType>
DistributedBinaryTraversal<RuleType>::DistributedBinaryTraversal() :
    rule(NULL),
    world()
{
  // We are an MPI child.  We must receive and construct our own RuleType
  // object, query tree, and reference tree.  Once we have done that, we kick
  // off the usual recursion, and when we're done, we send the results back.
  typename RuleType::MPIWrapper wrapper;
  Log::Info << "Process " << world.rank() << " is waiting for a message.\n";
  Timer::Start("child_receive");
  world.recv(0, 0, wrapper);
  Timer::Stop("child_receive");
  Log::Info << "Process " << world.rank() << " has received a message.\n";


  // We've now received our information.  Start the recursion.
  this->rule = wrapper.Rules();
  Timer::Start("child_traversal");
  Traverse(*wrapper.QueryTree(), *wrapper.ReferenceTree());
  Timer::Stop("child_traversal");

  // Now, we have to ship the neighbors and distances back to the master.
  typename RuleType::MPIResultsWrapper resultsWrapper(rule->Neighbors(),
                                                      rule->Distances());
  Log::Info << "Process " << world.rank() << " is sending results.\n";
  Timer::Start("send_results");
  world.send(0, 0, resultsWrapper);
  Timer::Stop("send_results");
  Log::Info << "Process " << world.rank() << " is finished.\n";
}

template<typename RuleType>
template<typename TreeType>
void DistributedBinaryTraversal<RuleType>::Traverse(const size_t queryIndex,
                                                    TreeType& referenceNode)
{

}

template<typename RuleType>
template<typename TreeType>
void DistributedBinaryTraversal<RuleType>::Traverse(TreeType& queryNode,
                                                    TreeType& referenceNode)
{
  // If we are the master, call the master traversal.  Otherwise, call the child
  // traversal.
  if (world.rank() == 0)
  {
    // Initialize the result request array.
    resultRequests = new boost::mpi::request[world.size() - 1];
    results = new typename RuleType::MPIResultsWrapper[world.size() - 1];
    Log::Info << "Result requests length " << world.size() - 1 << ".\n";

    // Start the traversal, and pass the work to the children.
    MasterTraverse(queryNode, referenceNode);

    // Wait until we have all the results, then merge them in.
    Log::Info << "Waiting on all results.\n";
    boost::mpi::wait_all(resultRequests, resultRequests + world.size() - 1);

    Log::Info << "Received all results; merging.\n";
    Timer::Start("merging_results");
    for (int i = 0; i < world.size() - 1; ++i)
      results[i].Merge(*rule);

    delete[] resultRequests;
    delete[] results;
    Timer::Stop("merging_results");
  }
  else
  {
    ChildTraverse(queryNode, referenceNode);
  }
}

template<typename RuleType>
template<typename TreeType>
void DistributedBinaryTraversal<RuleType>::MasterTraverse(
    TreeType& queryNode,
    TreeType& referenceNode,
    const size_t level)
{
  // Okay, we are the MPI master.  We need to recurse for a handful of levels,
  // before we are able to ship off tasks.
  Log::Info << "Required level is " << std::ceil(std::log2(world.size() - 1)) / 2 << "; current level is " << level << ".\n";
  if (level < std::ceil(std::log2(world.size() - 1)) / 2)
  {
    // Perform unprioritized dual-tree recursion.
    const double score = rule->Score(queryNode, referenceNode);

    if (score == DBL_MAX)
      return; // Pruned at a high level.

    // Otherwise, perform base cases.
    for (size_t i = 0; i < queryNode.NumPoints(); ++i)
      for (size_t j = 0; j < referenceNode.NumPoints(); ++j)
        rule->BaseCase(queryNode.Point(i), referenceNode.Point(j));

    // Now, perform unprioritized recursion.
    if (!queryNode.IsLeaf() && !referenceNode.IsLeaf())
    {
      MasterTraverse(*queryNode.Left(), *referenceNode.Left(), level + 1);
      MasterTraverse(*queryNode.Left(), *referenceNode.Right(), level + 1);
      MasterTraverse(*queryNode.Right(), *referenceNode.Left(), level + 1);
      MasterTraverse(*queryNode.Right(), *referenceNode.Right(), level + 1);
    }
    else if (queryNode.IsLeaf() && !referenceNode.IsLeaf())
    {
      // Hopefully this does not happen because GetTarget() won't handle it.
      MasterTraverse(queryNode, *referenceNode.Left(), level + 1);
      MasterTraverse(queryNode, *referenceNode.Right(), level + 1);
    }
    else if (!queryNode.IsLeaf() && referenceNode.IsLeaf())
    {
      // Hopefully this won't happen because GetTarget() won't handle it.
      MasterTraverse(*queryNode.Left(), referenceNode, level + 1);
      MasterTraverse(*queryNode.Right(), referenceNode, level + 1);
    }
  }
  else
  {
    // We are now ready to ship off tasks to children.  We have up to four
    // recursions we can perform here.  First, prepare the MPIWrapper object,
    // which is what we'll send.
    Log::Info << "Preparing MPI wrapper.\n";
    typename RuleType::MPIWrapper wrapper(&referenceNode, &queryNode, rule);
    Log::Info << "Get target id.\n";
    const size_t target = GetTarget(queryNode, referenceNode);
    Log::Info << "Sending trees to " << target << ".\n";
    Timer::Start("world_send");
    world.send(target, 0, wrapper);
    Timer::Stop("world_send");
    Log::Info << "Message queued to " << target << "!\n";

    // Wait for results.
    for (size_t i = 0; i < 1000000; ++i) { }
    resultRequests[target - 1] = world.irecv(target, 0, results[target - 1]);
    Log::Info << "Called irecv() for target " << target << ".\n";
  }
}

template<typename RuleType>
template<typename TreeType>
void DistributedBinaryTraversal<RuleType>::ChildTraverse(
    TreeType& queryNode,
    TreeType& referenceNode)
{
  // We'll just call out to the standard dual-tree traversal for a single node.
  typename TreeType::template DualTreeTraverser<RuleType> traverser(*rule);

  traverser.Traverse(queryNode, referenceNode);
}

template<typename RuleType>
template<typename TreeType>
size_t DistributedBinaryTraversal<RuleType>::GetTarget(
    TreeType& queryNode,
    TreeType& referenceNode) const
{
  // We assemble the ID of the target process in a bitwise manner.  The leftmost
  // combination maps to process 0.  At any level of recursion, because this is
  // a binary recursion, the query node may be either the left (L) child or the
  // right (R) child, and the same applies to the reference node.  Thus the
  // direction we have gone at a recursion can have four possibilities: LL, LR,
  // RL, and RR.  Take L = 0 and R = 1; now a single recursion can be
  // represented as two bits.  The highest-level recursion will be the two most
  // significant bits and the most recent recursion will be the two least
  // significant bits.  Thus, if the most recent recursion was RL and the
  // higher-level recursion was LR, and there were no higher recursions than
  // that, the index will be LRRL -> 0110 -> 6.  If any recursion was not a dual
  // recursion, undefined behavior will happen.  It probably won't crash.
  size_t index = 0;

  TreeType* currentQuery = &queryNode;
  TreeType* currentRef = &referenceNode;
  size_t level = 0;
  while (currentQuery->Parent() != NULL && currentRef->Parent() != NULL)
  {
    // Assemble this index.
    size_t currentIndex = 0; // Assume LL, change if otherwise.
    if (currentQuery->Parent()->Right() == currentQuery)
      currentIndex += 2; // Now it's RL.
    if (currentRef->Parent()->Right() == currentRef)
      currentIndex++; // Now it's LR or RR.

    // Append this index.
    index += (currentIndex << (level * 2));
    ++level;

    currentQuery = currentQuery->Parent();
    currentRef = currentRef->Parent();
  }

  return index + 1; // Index 0 is the root.
}

} // namespace tree
} // namespace mlpack

#endif
