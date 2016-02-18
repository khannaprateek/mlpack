/**
 * @file all_dimension_split.hpp
 * @author Ryan Curtin
 *
 * This splitting strategy checks all dimensions for splits.
 */
#ifndef __MLPACK_METHODS_HOEFFDING_TREE_ALL_DIMENSION_SPLIT_HPP
#define __MLPACK_METHODS_HOEFFDING_TREE_ALL_DIMENSION_SPLIT_HPP

#include <mlpack/core.hpp>
#include "gini_impurity.hpp"
#include "hoeffding_numeric_split.hpp"
#include "hoeffding_categorical_split.hpp"

namespace mlpack {
namespace tree {

/**
 * The AllDimensionSplit class is a split selection strategy for the
 * HoeffdingTree class that considers all dimensions for splitting.  It is the
 * standard decision tree split strategy, and will select the best of all
 * possible splits.
 *
 * @tparam FitnessFunction Fitness function to evaluate gains with.
 * @tparam NumericSplitType Type to use for numeric splits.
 * @tparam CategoricalSplitType Type to use for categorical splits.
 */
template<typename FitnessFunction = GiniImpurity,
         template<typename> class NumericSplitType =
             HoeffdingDoubleNumericSplit,
         template<typename> class CategoricalSplitType =
             HoeffdingCategoricalSplit>
class AllDimensionSplit
{
 public:
  /**
   * Create the AllDimensionSplit object.  This will initialize all the possible
   * splits with their default parameters.
   *
   * @param datasetInfo Dataset information.
   * @param numClasses Number of classes in dataset.
   */
  AllDimensionSplit(const data::DatasetInfo& datasetInfo,
                    const size_t numClasses);

  /**
   * Create the AllDimensionSplit object.  This will initialize all the possible
   * splits with the parameters of the splits in the other object.
   *
   * @param datasetInfo Dataset information.
   * @param numClasses Number of classes in dataset.
   * @param other AllDimensionSplit object to copy parameters from.
   */
  AllDimensionSplit(const data::DatasetInfo& datasetInfo,
                    const size_t numClasses,
                    const AllDimensionSplit& other);

  /**
   * Create the AllDimensionSplit object.  This will initialize all the possible
   * splits with the parameters of the splits in the other object.
   *
   * @param datasetInfo Dataset information.
   * @param numClasses Number of classes in dataset.
   * @param categoricalSplit Categorical split to take parameters from.
   * @param numericSplit Numeric split to take parameters from.
   */
  AllDimensionSplit(const data::DatasetInfo& datasetInfo,
                    const size_t numClasses,
                    const CategoricalSplitType<FitnessFunction>&
                        categoricalSplit,
                    const NumericSplitType<FitnessFunction>& numericSplit);

  //! Set the datasetInfo object.
  const data::DatasetInfo*& DatasetInfo() { return datasetInfo; }

  /**
   * Train the splits on a given point.
   */
  template<typename VecType>
  void Train(const VecType& point, const size_t label);

  /**
   * Given the data we have currently collected, determine whether or not a
   * split should be performed.  Returns the split dimension.
   */
  size_t SplitCheck(
      const double epsilon,
      const bool forceSplit,
      arma::Mat<size_t>& childCounts,
      size_t& splitDimension,
      typename CategoricalSplitType<FitnessFunction>::SplitInfo&
          categoricalSplitInfo,
      typename NumericSplitType<FitnessFunction>::SplitInfo& numericSplitInfo);

  /**
   * Serialize the object.
   */
  template<typename Archive>
  void Serialize(Archive& ar, const unsigned int /* version */);

 private:
  // The dataset information.  We never own this.
  const data::DatasetInfo* datasetInfo;

  //! Information for potential numeric splits in each numeric dimension.
  std::vector<NumericSplitType<FitnessFunction>> numericSplits;
  //! Information for potential categorical splits in each categorical
  //! dimension.
  std::vector<CategoricalSplitType<FitnessFunction>> categoricalSplits;
};

} // namespace tree
} // namespace mlpack

// Include implementation.
#include "all_dimension_split_impl.hpp"

#endif
