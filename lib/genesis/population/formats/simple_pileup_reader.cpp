/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2020 Lucas Czech

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
 * @ingroup population
 */

#include "genesis/population/formats/simple_pileup_reader.hpp"

#include "genesis/sequence/functions/codes.hpp"
#include "genesis/utils/io/char.hpp"
#include "genesis/utils/io/parser.hpp"
#include "genesis/utils/io/scanner.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace genesis {
namespace population {

// =================================================================================================
//     Reading & Parsing
// =================================================================================================

std::vector<SimplePileupReader::Record> SimplePileupReader::read(
    std::shared_ptr< utils::BaseInputSource > source
) const {
    std::vector<SimplePileupReader::Record> result;
    utils::InputStream it( source );

    Record rec;
    while( parse_line_( it, rec ) ) {
        result.push_back( rec );
    }
    return result;
}

bool SimplePileupReader::parse_line(
    utils::InputStream& input_stream,
    SimplePileupReader::Record& record
) const {
    return parse_line_( input_stream, record );
}

// =================================================================================================
//     Internal Members
// =================================================================================================

// -------------------------------------------------------------------------
//     Parse Line
// -------------------------------------------------------------------------

bool SimplePileupReader::parse_line_(
    utils::InputStream& input_stream,
    Record&             record
) const {
    // Shorthand.
    auto& it = input_stream;

    // If we reached the end of the input stream, reset the record. We do not reset per default,
    // in order to avoid costly re-initialization of the sample vector. But when we finish with
    // an input stream, we want to reset, so that subsequent usage of this reader class does not
    // fail if the pileip file contains a different number of samples.
    // Still, the user will currently get an error when using the same reader instance to
    // simultaneously (interlaced) read from multiple pileup files with differing number of samples
    // into the same record... But who does that?! If you are a user having this issue, please
    // let me know!
    if( !it ) {
        record = Record();
        return false;
    }
    assert( it );
    if( *it == '\n' ) {
        throw std::runtime_error(
            "Malformed pileup " + it.source_name() + " at " + it.at() + ": Invalid empty line"
        );
    }

    // Read chromosome.
    utils::affirm_char_or_throw( it, utils::is_graph );
    record.chromosome = utils::read_while( it, utils::is_graph );
    assert( !it || !utils::is_graph( *it ));

    // Read position.
    next_field_( it );
    record.position = utils::parse_unsigned_integer<size_t>( it );
    assert( !it || !utils::is_digit( *it ));

    // Read reference base.
    next_field_( it );
    auto const rb = utils::to_upper( *it );
    if( rb != 'A' && rb != 'C' && rb != 'G' && rb != 'T' && rb != 'N' ) {
        throw std::runtime_error(
            "Malformed pileup " + it.source_name() + " at " + it.at() +
            ": Invalid reference base that is not in [ACGTN]"
        );
    }
    record.reference_base = rb;
    ++it;

    // Read the samples. We switch once for the first line, and thereafter check that we read the
    // same number of samples each time.
    if( record.samples.empty() ) {
        while( it && *it != '\n' ) {
            record.samples.emplace_back();
            process_sample_( it, record, record.samples.size() - 1 );
        }
    } else {
        size_t index = 0;
        while( it && *it != '\n' ) {
            if( index >= record.samples.size() ) {
                throw std::runtime_error(
                    "Malformed pileup " + it.source_name() + " at " + it.at() +
                    ": Line with different number of samples."
                );
            }
            process_sample_( it, record, index );
            ++index;
        }
        if( index != record.samples.size() ) {
            throw std::runtime_error(
                "Malformed pileup " + it.source_name() + " at " + it.at() +
                ": Line with different number of samples."
            );
        }
    }

    assert( !it || *it == '\n' );
    ++it;
    return true;
}

// -------------------------------------------------------------------------
//     Process Sample
// -------------------------------------------------------------------------

void SimplePileupReader::process_sample_(
    utils::InputStream& input_stream,
    Record&             record,
    size_t              index
) const {

    // Get the sample to which to write to, and reset it.
    assert( index < record.samples.size() );
    auto& sample = record.samples[index];
    sample = Sample();

    // Fill its basic fields from input data, and compute the tallies.
    parse_sample_fields_( input_stream, record, sample );
    tally_sample_counts_( input_stream, sample );
}

// -------------------------------------------------------------------------
//     Parse Sample Fields
// -------------------------------------------------------------------------

void SimplePileupReader::parse_sample_fields_(
    utils::InputStream& input_stream,
    Record&             record,
    Sample&             sample
) const {
    // Shorthand.
    auto& it = input_stream;

    // Read the total read count / coverage.
    next_field_( it );
    sample.read_coverage = utils::parse_unsigned_integer<size_t>( it );
    assert( !it || !utils::is_digit( *it ));

    // Read the nucleotides, skipping everything that we don't want. We need to store these
    // in a string first, as we want to do quality checks.
    next_field_( it );
    sample.read_bases.reserve( sample.read_coverage );
    while( it && utils::is_graph( *it )) {
        auto const c = *it;
        switch( c ) {
            case '+':
            case '-': {
                // A sequence matching `[+-][0-9]+[ACGTNacgtn]+` is an insertion or deletion.
                // We skip/ignore those.

                // We first here allowed degenerated chars, and made a buffer for the codes
                // that are allowed in indels - like this:
                // static const std::string allowed_codes = sequence::nucleic_acid_codes_all_letters();

                // But later, we changed this to use the the proper pileup definition here,
                // see http://www.htslib.org/doc/samtools-mpileup.html
                static const std::string allowed_codes = "ACGTNacgtn*#";

                // First, we need to get how many chars there are in this indel.
                ++it;
                size_t const indel_cnt = utils::parse_unsigned_integer<size_t>( it );

                // Then, we skip that many chars, making sure that all is in order.
                for( size_t i = 0; i < indel_cnt; ++i ) {
                    if( !it || !std::strchr( allowed_codes.c_str(), utils::to_upper( *it ))) {
                        throw std::runtime_error(
                            "Malformed pileup " + it.source_name() + " at " + it.at() +
                            ": Line with invalid indel character " + utils::char_to_hex( *it )
                        );
                    }
                    ++it;
                }
                break;
            }
            case '^': {
                // Caret marks the start of a read segment, followed by a char for the mapping
                // quality. We skip both of these.
                ++it;
                if( !it ) {
                    throw std::runtime_error(
                        "Malformed pileup " + it.source_name() + " at " + it.at() +
                        ": Line with invalid start of read segment marker"
                    );
                }
                ++it;
                break;
            }
            case '$': {
                // Dollar marks the end of a read segment. Skip.
                ++it;
                break;
            }
            case '.': {
                sample.read_bases += utils::to_upper( record.reference_base );
                ++it;
                break;
            }
            case ',': {
                sample.read_bases += utils::to_lower( record.reference_base );
                ++it;
                break;
            }
            default: {
                sample.read_bases += c;
                ++it;
                break;
            }
        }
    }
    assert( !it || !utils::is_graph( *it ) );

    // Now read the quality codes, if present.
    if( with_quality_string_ ) {
        next_field_( it );
        sample.phred_scores.reserve( sample.read_coverage );
        while( it && utils::is_graph( *it )) {
            sample.phred_scores.push_back( sequence::quality_decode_to_phred_score(
                *it, quality_encoding_
            ));
            ++it;
        }
        assert( !it || !utils::is_graph( *it ) );

        if( sample.read_bases.size() != sample.phred_scores.size() ) {
            throw std::runtime_error(
                "Malformed pileup " + it.source_name() + " at " + it.at() +
                ": Line contains " + std::to_string( sample.read_bases.size() ) + " bases, but " +
                std::to_string( sample.phred_scores.size() ) + " quality score codes."
            );
        }
    }
    assert( sample.phred_scores.empty() || sample.read_bases.size() == sample.phred_scores.size() );
    assert( !it || !utils::is_graph( *it ) );

    // Final file sanity checks.
    if( it && !( utils::is_blank( *it ) || utils::is_newline( *it ))) {
        throw std::runtime_error(
            "Malformed pileup " + it.source_name() + " at " + it.at() +
            ": Invalid characters."
        );
    }
}

// -------------------------------------------------------------------------
//     Tally Sample Counts
// -------------------------------------------------------------------------

void SimplePileupReader::tally_sample_counts_(
    utils::InputStream& input_stream,
    Sample&             sample
) const {
    // We expect default values.
    assert( sample.a_count + sample.c_count + sample.g_count + sample.t_count == 0 );
    assert( sample.n_count + sample.d_count == 0 );

    // Shorthand.
    auto& it = input_stream;

    // Finally, tally up the bases.
    size_t total_count = 0;
    size_t skip_count  = 0;
    size_t rna_count   = 0;
    for( size_t i = 0; i < sample.read_bases.size(); ++i ) {

        // Quality control if available. Skip bases that are below the threshold.
        if( !sample.phred_scores.empty() && sample.phred_scores[i] < min_phred_score_ ) {
            ++skip_count;
            continue;
        }

        ++total_count;
        switch( sample.read_bases[i] ) {
            case 'a':
            case 'A': {
                ++sample.a_count;
                break;
            }
            case 'c':
            case 'C': {
                ++sample.c_count;
                break;
            }
            case 'g':
            case 'G': {
                ++sample.g_count;
                break;
            }
            case 't':
            case 'T': {
                ++sample.t_count;
                break;
            }
            case 'n':
            case 'N': {
                ++sample.n_count;
                break;
            }
            case '*':
            case '#': {
                ++sample.d_count;
                break;
            }
            case '<':
            case '>': {
                // Skipping RNA symbols. But count them, for sanity check.
                (void) rna_count;
                ++rna_count;
                break;
            }
            default: {
                throw std::runtime_error(
                    "Malformed pileup " + it.source_name() + " near " + it.at() +
                    ": Invalid allele character " + utils::char_to_hex( sample.read_bases[i] )
                );
            }
        }
    }

    // Sanity checks and assertions.
    (void) total_count;
    assert(
        total_count ==
        sample.a_count + sample.c_count + sample.g_count + sample.t_count +
        sample.n_count + sample.d_count + rna_count
    );
    assert( skip_count + total_count == sample.read_bases.size() );

    // Sum sanity checks. There seems to be a very weird special case (found in the PoPoolation2
    // test dataset) where a line contains a deletion with a low phred score (`*`) that is not
    // counted in the "Number of reads covering this position" counter:
    // `  89795 2R      113608  N       1       T$      A       0       *       *`
    // We account for this here by allowing exactly one such base that is either a deletion
    // or a skip due to low phred score. There is no information that we know of about how
    // "empty" lines should be treated in pileip, so we have to guess, and that here seems to work.
    auto const base_count =
        sample.a_count + sample.c_count + sample.g_count + sample.t_count + sample.n_count
    ;
    if(
        sample.read_bases.size() != sample.read_coverage &&
        !( base_count == 0 && sample.d_count + skip_count == 1 )
    ) {

        throw std::runtime_error(
            "Malformed pileup " + it.source_name() + " near " + it.at() +
            ": Given read count (" + std::to_string( sample.read_coverage ) +
            ") does not match the number of bases found in the sample (" +
            std::to_string( sample.read_bases.size() ) + ")"
        );
    }
}

// -------------------------------------------------------------------------
//     Next Field
// -------------------------------------------------------------------------

void SimplePileupReader::next_field_( utils::InputStream& input_stream ) const
{
    // There needs to be at last some whitespace that separates the field. Affirm that,
    // then skip it until we are at the content of the next field.
    utils::affirm_char_or_throw( input_stream, utils::is_blank );
    utils::skip_while( input_stream, utils::is_blank );
    assert( !input_stream || !utils::is_blank( *input_stream ));
}

} // namespace population
} // namespace genesis
