/**
 * @brief Definition of the genesis Python module.
 *
 * @file
 * @ingroup python
 */

#include <python/src/common.hpp>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// =============================================================================
//     Forward declarations of all exported classes
// =============================================================================

    // -------------------------------------------
    //     Placement
    // -------------------------------------------

void BoostPythonExport_JplaceProcessor();
void BoostPythonExport_PlacementMap();
void BoostPythonExport_PlacementMapSet();
void BoostPythonExport_PlacementMapSerializer();
void BoostPythonExport_PlacementMeasures();
void BoostPythonExport_PlacementTree();
void BoostPythonExport_PlacementTreePhyloxmlAdapter();
void BoostPythonExport_PlacementSimulator();

    // -------------------------------------------
    //     Tree
    // -------------------------------------------

void BoostPythonExport_DefaultTree();
// void BoostPythonExport_NewickProcessor();
//~ void BoostPythonExport_TreeSet();
// void BoostPythonExport_PhyloxmlAdapter();
// void BoostPythonExport_PhyloxmlProcessor();

    // -------------------------------------------
    //     Utils
    // -------------------------------------------

void BoostPythonExport_Bitvector();
void BoostPythonExport_Logging();

// =============================================================================
//     Boost Python Module
// =============================================================================

BOOST_PYTHON_MODULE(genesis)
{
    // show genesis docstrings, python signature, but not c++ signature
    boost::python::docstring_options doc_options(true, true, false);

    using namespace boost::python;

    // TODO oursource to some utils scope
    boost::python::class_<std::vector<int> >("VectorInt")
        .def(vector_indexing_suite<std::vector<int> >())
    ;

    // class_< std::vector<int> > ("VectorInt")
    //     .def (indexing::container_suite< std::vector<int> >())
    // ;

    // -------------------------------------------
    //     Tree
    // -------------------------------------------

    BoostPythonExport_DefaultTree();
    // BoostPythonExport_NewickProcessor();
    //~ BoostPythonExport_TreeSet();
    // BoostPythonExport_PhyloxmlAdapter();
    // BoostPythonExport_PhyloxmlProcessor();

    // -------------------------------------------
    //     Placement
    // -------------------------------------------

    BoostPythonExport_JplaceProcessor();
    BoostPythonExport_PlacementMap();
    BoostPythonExport_PlacementMapSet();
    BoostPythonExport_PlacementMapSerializer();
    BoostPythonExport_PlacementMeasures();
    BoostPythonExport_PlacementTree();
    BoostPythonExport_PlacementTreePhyloxmlAdapter();
    BoostPythonExport_PlacementSimulator();

    // -------------------------------------------
    //     Utils
    // -------------------------------------------

    BoostPythonExport_Bitvector();
    BoostPythonExport_Logging();

}