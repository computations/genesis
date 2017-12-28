#ifndef GENESIS_SEQUENCE_FUNCTIONS_SIGNATURE_SPECIFICATIONS_H_
#define GENESIS_SEQUENCE_FUNCTIONS_SIGNATURE_SPECIFICATIONS_H_

/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2017 Lucas Czech

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
 * @ingroup sequence
 */

#include "genesis/utils/tools/char_lookup.hpp"

#include<limits>
#include<string>
#include<vector>

namespace genesis {
namespace sequence {

// =================================================================================================
//     Forwardd Declarations
// =================================================================================================

class Sequence;
class SequenceSet;

// =================================================================================================
//     Signature Specifications
// =================================================================================================

/**
 * @brief
 */
class SignatureSpecifications
{
public:

    // -------------------------------------------------------------------------
    //     Typedefs and Enums
    // -------------------------------------------------------------------------

    /**
     * @brief What to do when a char that is not part of the alphabet is encountered while
     * counting kmers.
     */
    enum class UnknownCharBehavior
    {
        /**
         * @brief Simply ignore the char by skipping it.
         */
        kSkip,

        /**
         * @brief Throw an exception.
         */
        kThrow
    };

    /**
     * @brief Value that is used to indicate an invalid (non-alphabet) char when using index_of().
     */
    static size_t const InvalidCharIndex;

    // -------------------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------------------

    SignatureSpecifications() = default;
    SignatureSpecifications( std::string const& alphabet, size_t k );

    /**
     * @brief Default destructor.
     */
    ~SignatureSpecifications() = default;

    /**
    * @brief Default copy constructor.
    */
    SignatureSpecifications( SignatureSpecifications const& ) = default;

    /**
    * @brief Default move constructor.
    */
    SignatureSpecifications( SignatureSpecifications&& )      = default;

    /**
    * @brief Default copy assignment.
    */
    SignatureSpecifications& operator= ( SignatureSpecifications const& ) = default;

    /**
    * @brief Default move assignment.
    */
    SignatureSpecifications& operator= ( SignatureSpecifications&& )      = default;

    // -------------------------------------------------------------------------
    //     Accessors
    // -------------------------------------------------------------------------

    std::string const& alphabet() const
    {
        return alphabet_;
    }

    size_t k() const
    {
        return k_;
    }

    UnknownCharBehavior unknown_char_behavior() const
    {
        return unknown_char_behavior_;
    }

    bool is_nucleic_acids() const
    {
        return is_nucleic_acids_;
    }

    /**
     * @brief Return the index of a char within the alphabet().
     *
     * For chars that are not in the alphabet, InvalidCharIndex is returned as
     * an indicator value.
     */
    size_t index_of( char c ) const
    {
        return index_lookup_[c];
    }

    size_t kmer_list_size() const;

    // -------------------------------------------------------------------------
    //     Modifiers
    // -------------------------------------------------------------------------

    SignatureSpecifications& unknown_char_behavior( UnknownCharBehavior value )
    {
        unknown_char_behavior_ = value;
        return *this;
    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    // Direct Settings
    std::string alphabet_;
    size_t      k_ = 0;
    UnknownCharBehavior unknown_char_behavior_ = UnknownCharBehavior::kSkip;

    // Induced Settings and Helpers
    bool        is_nucleic_acids_ = false;
    utils::CharLookup<size_t> index_lookup_;
};

} // namespace sequence
} // namespace genesis

#endif // include guard
