/**
 * @brief Implementation of basic tree functions.
 *
 * For reasons of readability, in this implementation file, the template data types
 * NodeDataType and EdgeDataType are abbreviated using NDT and EDT, respectively.
 *
 * @file
 * @ingroup tree
 */

#include <algorithm>
#include <assert.h>
#include <sstream>

#include "tree/newick_processor.hpp"
#include "utils/logging.hpp"
#include "utils/utils.hpp"

namespace genesis {

// -------------------------------------------------------------------------
//     Construction and Destruction
// -------------------------------------------------------------------------

/**
 * @brief Deletes all data of the tree, including all links, nodes and edges.
 */
template <class NDT, class EDT>
void Tree<NDT, EDT>::clear()
{
    for (LinkType* link : links_) {
        delete link;
    }
    for (NodeType* node : nodes_) {
        delete node;
    }
    for (EdgeType* edge : edges_) {
        delete edge;
    }

    std::vector<LinkType*>().swap(links_);
    std::vector<NodeType*>().swap(nodes_);
    std::vector<EdgeType*>().swap(edges_);
}

/**
 * @brief Destructor. Calls clear() to free all memory used by the tree and its substructures.
 */
template <class NDT, class EDT>
Tree<NDT, EDT>::~Tree ()
{
    clear();
}

/**
 * @brief Imports all elements of a tree.
 *
 * This function overwrites the topology and data of this tree with a given set of links, nodes
 * and edges. Use with care! No checks are done concerning the validity of the passed input.
 *
 * Caveat: Only the pointers to the tree elements are copied, not the elements themselves. Thus,
 * this function is not intended for creating a deep copy. It merely is a fast way to pass pointers
 * to tree elements.
 *
 * Therefore, the main usage of this function is to get a tree from different TreeProcessor objects
 * for reading trees from files.
 */
template <class NDT, class EDT>
void Tree<NDT, EDT>::Import(LinkArray& links, NodeArray& nodes, EdgeArray& edges)
{
    clear();
    links_ = links;
    nodes_ = nodes;
    edges_ = edges;
}

/**
 * @brief Exports all elements of a tree.
 *
 * Caveat: Only the pointers to the tree elements are copied, not the elements themselves. Thus,
 * this function is not intended for creating a deep copy. It merely is a fast way to pass pointers
 * to tree elements.
 */
template <class NDT, class EDT>
void Tree<NDT, EDT>::Export(LinkArray& links, NodeArray& nodes, EdgeArray& edges)
{
    links = links_;
    nodes = nodes_;
    edges = edges_;
}

// -------------------------------------------------------------------------
//     Member Functions
// -------------------------------------------------------------------------

/**
 * @brief Returns the highest rank of the nodes of the Tree.
 */
template <class NDT, class EDT>
int Tree<NDT, EDT>::MaxRank() const
{
    int max = -1;
    for (size_t i = 0; i < nodes_.size(); ++i) {
        int rank = nodes_[i]->Rank();
        if (rank == 1) {
            LOG_WARN << "Node with rank 1 found. This is a node without furcation, and usually "
                     << "indicates an error.";
        }
        max = std::max(rank, max);
    }
    return max;
}

/**
 * @brief Returns whether the Tree is bifurcating.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::IsBifurcating() const
{
    return MaxRank() == 2;
}

/**
 * @brief
 */
template <class NDT, class EDT>
Matrix<int>* Tree<NDT, EDT>::NodeDepthMatrix()
{
    Matrix<int>* mat = new Matrix<int>(NodesSize(), NodesSize());
    // TODO
    return mat;
}

/**
 * @brief
 *
 * If no Node pointer is provided, the root is taken as node.
 */
template <class NDT, class EDT>
std::vector<int> Tree<NDT, EDT>::NodeDepthVector(NodeType* node)
{
    if (!node) {
        node = RootNode();
    }

    // store the distance from each node to the given node.
    std::vector<int> vec;
    vec.resize(NodesSize(), -1);
    vec[0] = 0;

    // calculate the distance vector via levelorder iteration.
    for (
        IteratorLevelorder it = BeginLevelorder(node);
        it != EndLevelorder();
        ++it
    ) {
        // skip the root (it is already set to 0).
        if (it.IsFirstIteration()) {
            continue;
        }

        // the distance is the distance from the "parent" node (the next one in direction towards
        // the given node) plus 1.
        assert(vec[it.Node()->Index()] == -1);
        vec[it.Node()->Index()] = 1 + vec[it.Link()->Outer()->Node()->Index()];
    }

    return vec;
}

/**
 * @brief Returns a distance matrix containing pairwise distances between all Nodes, using the
 * branch_length of the Edges as distance measurement.
 */
template <class NDT, class EDT>
Matrix<double>* Tree<NDT, EDT>::NodeDistanceMatrix()
{
    Matrix<double>* mat = new Matrix<double>(NodesSize(), NodesSize());

    // fill every row of the matrix
    for (NodeType* row_node : nodes_) {
        // set the diagonal element of the matrix.
        (*mat)(row_node->Index(), row_node->Index()) = 0.0;

        // the columns are filled using a levelorder traversal. this makes sure that for every node
        // we know how to calculate the distance to the current row node.
        for (
            IteratorLevelorder it = BeginLevelorder(row_node->Link());
            it != EndLevelorder();
            ++it
        ) {
            // skip the diagonal of the matrix.
            if (it.IsFirstIteration()) {
                continue;
            }

            // the distance to the current row node is: the length of the current branch plus
            // the distance from the other end of that branch to the row node.
            (*mat)(row_node->Index(), it.Node()->Index())
                = it.Edge()->data.branch_length
                + (*mat)(row_node->Index(), it.Link()->Outer()->Node()->Index());
        }
    }

    return mat;
}

/**
 * @brief
 *
 * If no Node pointer is provided, the root is taken as node.
 */
template <class NDT, class EDT>
std::vector<double> Tree<NDT, EDT>::NodeDistanceVector(NodeType* node)
{
    if (!node) {
        node = RootNode();
    }

    std::vector<double> vec;
    vec.resize(NodesSize(), 0.0);
    // TODO
    return vec;
}

/**
 * @brief Compares two trees for equality given a binary comparator functional.
 *
 * This function does a preorder traversal of both trees in parallel and calls a comparator
 * functional for each position of the iterator. It returns true iff the comparator is true for
 * every position.
 *
 * The comparator functional can be either a function pointer, function object, or lambda
 * expression.
 *
 * Furthermore, the trees are checked for equal topology: their elements (links, nodes, edges)
 * have to be equal in size and the rank of each node during the traversal has to be identical in
 * both trees. Those assumptions are made because two trees that do not have identical topology
 * are never considered equal for the purposes of this framework.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::Equal(
    TreeType& lhs,
    TreeType& rhs,
    std::function<bool (TreeType::IteratorPreorder&, TreeType::IteratorPreorder&)> comparator
) {
    // check array sizes
    if (lhs.links_.size() != rhs.links_.size() ||
        lhs.nodes_.size() != rhs.nodes_.size() ||
        lhs.edges_.size() != rhs.edges_.size()
    ) {
        return false;
    }

    // do a preorder traversal on both trees in parallel
    TreeType::IteratorPreorder it_l = lhs.BeginPreorder();
    TreeType::IteratorPreorder it_r = rhs.BeginPreorder();
    for (
        ;
        it_l != lhs.EndPreorder() && it_r != rhs.EndPreorder();
        ++it_l, ++it_r
    ) {
        if (it_l.Node()->Rank() != it_r.Node()->Rank() || !comparator(it_l, it_r)) {
            return false;
        }
    }

    // check whether we are done with both trees
    if (it_l != lhs.EndPreorder() || it_r != rhs.EndPreorder()) {
        return false;
    }

    return true;
}

/**
 * @brief Compares the tree to another one given a binary comparator functional.
 *
 * See other Equal() function for more information.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::Equal(
    TreeType& other,
    std::function<bool (TreeType::IteratorPreorder&, TreeType::IteratorPreorder&)> comparator
) {
    return Equal(*this, other, comparator);
}

// TODO make const! (need to add const versions of the tree iterators first...)
/**
 * @brief Returns true iff both trees have an identical topology.
 *
 * The topology is considered identical only if the order of edges is also the same in both trees.
 * This means, although two trees might have the same number of tips and branches, they might still
 * be not identical (with respect to this function) when the branches appear in a different order
 * or when the root sits at a different node.
 *
 * Thus, this function is mainly intended to check whether two trees have been produced from the
 * same input, for example from the same Newick file.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::HasIdenticalTopology(TreeType& right)
{
    auto comparator = [] (TreeType::IteratorPreorder&, TreeType::IteratorPreorder&)
    {
        return true;
    };

    return Equal(right, comparator);
}

/**
 * @brief Returns true iff both trees contain identical data on all their edges.
 *
 * See HasIdenticalData() for more information.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::HasIdenticalEdgeData(TreeType& right) const
{
    // check array size
    if (this->edges_.size() != right.edges_.size()) {
        return false;
    }

    // check edge data
    for (size_t i = 0; i < this->edges_.size(); ++i) {
        if (this->edges_[i]->data != right.edges_[i]->data) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Returns true iff both trees contain identical data on all their nodes.
 *
 * See HasIdenticalData() for more information.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::HasIdenticalNodeData(TreeType& right) const
{
    // check array sizes
    if (this->nodes_.size() != right.nodes_.size()) {
        return false;
    }

    // check node data
    for (size_t i = 0; i < this->nodes_.size(); ++i) {
        if (this->nodes_[i]->data != right.nodes_[i]->data) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Returns true iff both trees contain identical data on all their nodes and edges.
 *
 * It is first checked whether both trees have the same number of nodes and edges. It is however
 * not checked whether they have an identical topology. See HasIdenticalTopology() for this.
 * As this function relies on the order of nodes and edges in memory, it is however quite
 * improbable to have two trees with identical data but not identical topology.
 *
 * Thus, this function is mainly intended to check whether two trees have been produced from the
 * same input, for example from the same Newick file.
 */
template <class NDT, class EDT>
bool Tree<NDT, EDT>::HasIdenticalData(TreeType& right) const
{
    return HasIdenticalEdgeData(right) && HasIdenticalNodeData(right);
}

// -------------------------------------------------------------------------
//     Dump and Debug Functions
// -------------------------------------------------------------------------


    // TODO introduce a validate function that checks the integrity of the tree:
    // TODO are all links, edges and nodes connected corretly to each other,
    // TODO is every one of them coverd exactly once when doing a full traversal?
    // TODO do all node->link_ links point to the root? same for all edge->primary?
    // TODO all objects coupled correctly?
    // TODO also, if we introduce indices to them for faster access, are those correct?
    // TODO this function will be curtial to ensure correctness of invariants once
    // TODO we start implementing stuff that modifies a tree (add nodes, move branches...)!
    // TODO do all iterators and check consistency! eg is a round trip covering every object (links onces, branches twice, nodes rank many times)?
    // TODO iterator over all links and count if edges are encountered exactly twice each.


template <class NDT, class EDT>
bool Tree<NDT, EDT>::Validate() const
{
    // check that the member arrays are valid: if at least one of them is empty, the tree is not
    // fully initialized, so either it is a new tree without any data (all arrays empty, valid),
    // or some are empty, but others not (not valid).
    if (links_.empty() || nodes_.empty() || edges_.empty()) {
        bool emp = links_.empty() && nodes_.empty() && edges_.empty();
        if (emp) {
            LOG_INFO << "Tree is empty.";
        } else {
            LOG_INFO << "Tree is not empty, but one of its data members is.";
        }
        return emp;
    }

    if (links_.front()->node_ != nodes_.front()) {
        LOG_INFO << "The first link does not correspond to the first node.";
        return false;
    }

    for (size_t i = 0; i < links_.size(); ++i) {
        if (i != links_[i]->index_) {
            LOG_INFO << "Link at index " << i << " has wrong index (" << links_[i]->index_ << ").";
            return false;
        }
    }

    for (size_t i = 0; i < nodes_.size(); ++i) {
        if (i != nodes_[i]->index_) {
            LOG_INFO << "Node at index " << i << " has wrong index (" << nodes_[i]->index_ << ").";
            return false;
        }
    }

    for (size_t i = 0; i < edges_.size(); ++i) {
        if (i != edges_[i]->index_) {
            LOG_INFO << "Edge at index " << i << " has wrong index (" << edges_[i]->index_ << ").";
            return false;
        }
    }

    // if we are here, all three arrays (links, nodes, edges) contain data, so we can start a full
    // traversal along all links.
    LinkType* link = links_.front();
    do {
        link = link->next_->outer_;
    } while (link != links_.front());

    return true;
}

/**
 * @brief Returns the combined dump of DumpLinks(), DumpNodes() and DumpEdges().
 */
template <class NDT, class EDT>
std::string Tree<NDT, EDT>::Dump() const
{
    return DumpLinks() + "\n" + DumpNodes() + "\n" + DumpEdges();
}

/**
 * @brief Returns a list of all links including their next and outer link numbers as well as their
 * node and edge numbers.
 */
template <class NDT, class EDT>
std::string Tree<NDT, EDT>::DumpLinks() const
{
    std::ostringstream out;
    for (size_t i = 0; i < links_.size(); ++i) {
        out << "Link " << i
            << " \t Next: "  << LinkPointerToIndex(links_[i]->next_)
            << " \t Outer: " << LinkPointerToIndex(links_[i]->outer_)
            << " \t Node: "  << NodePointerToIndex(links_[i]->node_)
            << " \t Edge: "  << EdgePointerToIndex(links_[i]->edge_)
            << " \t " << links_[i]->Dump()
            << "\n";
    }
    return out.str();
}

/**
 * @brief Returns a list of all nodes including their name and the number of one of their links.
 */
template <class NDT, class EDT>
std::string Tree<NDT, EDT>::DumpNodes() const
{
    std::ostringstream out;
    for (size_t i = 0; i < nodes_.size(); ++i) {
        out << "Node " << i
            << " \t Link: " << LinkPointerToIndex(nodes_[i]->link_)
            << " \t " << nodes_[i]->Dump() << "\n";
    }
    return out.str();
}

/**
 * @brief Returns a list of all edges including their link numbers and branch lengths.
 */
template <class NDT, class EDT>
std::string Tree<NDT, EDT>::DumpEdges() const
{
    std::ostringstream out;
    for (size_t i = 0; i < edges_.size(); ++i) {
        out << "Edge " << i
            << " \t Link P: " << LinkPointerToIndex(edges_[i]->link_p_)
            << " \t Link S: " << LinkPointerToIndex(edges_[i]->link_s_)
            << " \t " << edges_[i]->Dump() << "\n";
    }
    return out.str();
}

/**
 * @brief Do a full tree traversal and return a list of all visited node names.
 *
 * Leaf nodes appear once in this list, while inner nodes appear every time the traversal visits
 * them. Thus, a node of rank 3 (meaning, it has three immediate children), is visited four times:
 * One time when coming from its parent, and then once each time the traversal returns from its
 * children.
 */
template <class NDT, class EDT>
std::string Tree<NDT, EDT>::DumpRoundtrip() const
{
    if (links_.empty()) {
        return "";
    }

    std::string out;
    LinkType* link = links_.front();

    do {
        out += link->node_->name_ + "\n";
        link = link->next_->outer_;
    } while (link != links_.front());

    return out;
}

/**
 * @brief Returns the index of a given link pointer within the link pointer array links_.
 *
 * This is useful for debugging purposes, particularly for the Dump functions.
 * Returns `-1` if the pointer was not found.
 */
template <class NDT, class EDT>
int Tree<NDT, EDT>::LinkPointerToIndex (LinkType* link) const
{
    for (size_t i = 0; i < links_.size(); ++i) {
        if (links_[i] == link) {
            assert(link->index_ == i);
            return i;
        }
    }
    return -1;
}

/**
 * @brief Returns the index of a given node pointer within the node pointer array nodes_.
 *
 * This is useful for debugging purposes, particularly for the Dump functions.
 * Returns `-1` if the pointer was not found.
 */
template <class NDT, class EDT>
int Tree<NDT, EDT>::NodePointerToIndex (NodeType* node) const
{
    for (size_t i = 0; i < nodes_.size(); ++i) {
        if (nodes_[i] == node) {
            assert(node->index_ == i);
            return i;
        }
    }
    return -1;
}

/**
 * @brief Returns the index of a given edge pointer within the edge pointer array edges_.
 *
 * This is useful for debugging purposes, particularly for the Dump functions.
 * Returns `-1` if the pointer was not found.
 */
template <class NDT, class EDT>
int Tree<NDT, EDT>::EdgePointerToIndex (EdgeType* edge) const
{
    for (size_t i = 0; i < edges_.size(); ++i) {
        if (edges_[i] == edge) {
            assert(edge->index_ == i);
            return i;
        }
    }
    return -1;
}

} // namespace genesis