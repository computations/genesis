#ifndef GNS_PLACEMENT_PLACEMENTS_H_
#define GNS_PLACEMENT_PLACEMENTS_H_

/**
 * @brief
 *
 * @file
 * @ingroup placement
 */

#include <deque>
#include <string>
#include <unordered_map>

#include "placement/placement_tree.hpp"

namespace genesis {

// =============================================================================
//     Forward Declarations
// =============================================================================

class  JsonDocument;
class  JsonLexer;
struct Pquery;

// =============================================================================
//     Pquery Placement
// =============================================================================

struct PqueryPlacement
{
    PqueryPlacement() : edge_num(0), likelihood(0.0), like_weight_ratio(0.0), distal_length(0.0),
                        pendant_length(0.0), parsimony(0), pquery(nullptr), edge(nullptr)
    {}

    PqueryPlacement(const PqueryPlacement* other) :
        edge_num(other->edge_num),
        likelihood(other->likelihood),
        like_weight_ratio(other->like_weight_ratio),
        distal_length(other->distal_length),
        pendant_length(other->pendant_length),
        parsimony(other->parsimony),
        pquery(nullptr),
        edge(nullptr)
    {}

    int     edge_num;
    double  likelihood;
    double  like_weight_ratio;
    double  distal_length;
    double  pendant_length;
    int     parsimony;

    Pquery* pquery;
    PlacementTree::EdgeType* edge;
};

// =============================================================================
//     Pquery Name
// =============================================================================

struct PqueryName
{
    PqueryName() : name(""), multiplicity(0.0), pquery(nullptr)
    {}

    PqueryName(const PqueryName* other) :
        name(other->name),
        multiplicity(other->multiplicity),
        pquery(nullptr)
    {}

    std::string name;
    double      multiplicity;

    Pquery* pquery;
};

// =============================================================================
//     Pquery
// =============================================================================

struct Pquery
{
    ~Pquery()
    {
        for (PqueryPlacement* p : placements) {
            delete p;
        }
        for (PqueryName* n : names) {
            delete n;
        }
        placements.clear();
        names.clear();
    }

    std::deque<PqueryPlacement*> placements;
    std::deque<PqueryName*>      names;
};

// =============================================================================
//     Placements
// =============================================================================

class Placements
{
public:
    // -----------------------------------------------------
    //     Constructor & Destructor
    // -----------------------------------------------------

    Placements () {}
    Placements (PlacementTree& ptree) : tree(ptree) {}
    void clear();
    ~Placements();

    typedef std::unordered_map<int, PlacementTree::EdgeType*> EdgeNumMapType;
    EdgeNumMapType* EdgeNumMap();

    bool Merge(Placements& other);
    void RestrainToMaxWeightPlacements();

    // -----------------------------------------------------
    //     Placement Weight
    // -----------------------------------------------------

    static double EMD (Placements& left, Placements& right);
    double EMD (Placements& other);

    size_t PlacementCount();
    double PlacementMass();
    void   COG();
    double Variance();

    // -----------------------------------------------------
    //     Dump and Debug
    // -----------------------------------------------------

    std::string Dump();
    bool Validate (bool check_values = false, bool break_on_values = false);

    // -----------------------------------------------------
    //     Members
    // -----------------------------------------------------

    std::deque<Pquery*>                          pqueries;
    PlacementTree                                tree;
    std::unordered_map<std::string, std::string> metadata;
};

} // namespace genesis

#endif // include guard