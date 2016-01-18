#ifndef GENESIS_PLACEMENT_OPERATORS_H_
#define GENESIS_PLACEMENT_OPERATORS_H_

/**
 * @brief
 *
 * @file
 * @ingroup placement
 */

#include <iosfwd>
#include <string>

namespace genesis {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

class PlacementMap;

// =================================================================================================
//     Comparision and Equality
// =================================================================================================

bool compatible_trees (const PlacementMap& lhs, const PlacementMap& rhs);

// =================================================================================================
//     Verification
// =================================================================================================

bool has_correct_edge_nums( PlacementMap const& map );

// =================================================================================================
//     Output
// =================================================================================================

std::ostream& operator << ( std::ostream& out, PlacementMap const& map );

} // namespace genesis

#endif // include guard