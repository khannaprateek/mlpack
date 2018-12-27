/**
 * @file get_param.hpp
 * @author Ryan Curtin
 *
 * Get a parameter for a Python binding.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_BINDINGS_PYTHON_GET_PARAM_HPP
#define MLPACK_BINDINGS_PYTHON_GET_PARAM_HPP

#include <mlpack/prereqs.hpp>

namespace mlpack {
namespace bindings {
namespace markdown {

/**
 * All Python binding types are exactly what is held in the ParamData, so no
 * special handling is necessary.
 */
template<typename T>
void GetParam(const util::ParamData& d,
              const void* /* input */,
              void* output)
{
  util::ParamData& dmod = const_cast<util::ParamData&>(d);
  std::cout << "call GetParam()\n";
  *((T**) output) = boost::any_cast<T>(&dmod.value);
}

} // namespace markdown
} // namespace bindings
} // namespace mlpack

#endif
