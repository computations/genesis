/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2016 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab, Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup test
 */

#include "common.hpp"

#include "lib/sequence/codes.hpp"
#include "lib/sequence/functions.hpp"
#include "lib/sequence/sequence_set.hpp"
#include "lib/sequence/io/fasta_reader.hpp"

#include <string>

using namespace genesis::sequence;

TEST( Sequence, FastaReaderValidating )
{
    // Skip test if no data availabe.
    NEEDS_TEST_DATA;

    // Load sequence file.
    std::string infile = environment->data_dir + "sequence/dna_354.fasta";
    SequenceSet sset;
    FastaReader()
        .validate_chars( nucleic_acid_codes_all() )
        .from_file(infile, sset);

    // Check data.
    EXPECT_EQ( 354, sset.size() );
    EXPECT_EQ( 460,                    sset[0].length() );
    EXPECT_EQ( "Di106BGTue",           sset[0].label() );
    EXPECT_EQ( "",                     sset[0].metadata() );
    EXPECT_EQ( "TCGAAACCTGC------CTA", sset[0].sites().substr( 0, 20 ) );
}
