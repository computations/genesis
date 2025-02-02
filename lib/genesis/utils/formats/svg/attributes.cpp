/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2022 Lucas Czech

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
    Lucas Czech <lczech@carnegiescience.edu>
    Department of Plant Biology, Carnegie Institution For Science
    260 Panama Street, Stanford, CA 94305, USA
*/

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include "genesis/utils/formats/svg/attributes.hpp"

#include "genesis/utils/formats/svg/helper.hpp"
#include "genesis/utils/math/common.hpp"
#include "genesis/utils/text/string.hpp"
#include "genesis/utils/tools/color/functions.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>

namespace genesis {
namespace utils {

// =================================================================================================
//     Svg Stroke
// =================================================================================================

// -------------------------------------------------------------
//     Constructors and Rule of Five
// -------------------------------------------------------------

SvgStroke::SvgStroke( SvgStroke::Type type )
    : type( type )
    , color()
    , width( 1.0 )
    , width_unit()
    , line_cap( LineCap::kOmit )
    , line_join( LineJoin::kOmit )
    , miterlimit( 1.0 )
    , dash_array()
    , dash_offset( 0.0 )
{}

SvgStroke::SvgStroke( Color color_value, double width_value )
    : SvgStroke( Type::kColor )
{
    color = color_value;
    width = width_value;
}

SvgStroke::SvgStroke( std::string gradient_id_value )
    : SvgStroke( Type::kGradient )
{
    gradient_id = gradient_id_value;
}

// -------------------------------------------------------------
//     Drawing Function
// -------------------------------------------------------------

void SvgStroke::write( std::ostream& out ) const
{
    // Treat special cases.
    if( type == Type::kOmit ) {
        return;
    }
    if( type == Type::kNone ) {
        out << svg_attribute( "stroke", "none" );
        return;
    }
    if( type == Type::kGradient ) {
        out << svg_attribute( "stroke", "url(#" + gradient_id + ")" );
        return;
    }

    out << svg_attribute( "stroke", color_to_hex( color ) );
    out << svg_attribute( "stroke-opacity", color.a() );
    out << svg_attribute( "stroke-width", width, width_unit );

    switch( line_cap ) {
        case LineCap::kOmit:
            break;
        case LineCap::kButt:
            out << svg_attribute( "stroke-linecap", "butt" );
            break;
        case LineCap::kSquare:
            out << svg_attribute( "stroke-linecap", "square" );
            break;
        case LineCap::kRound:
            out << svg_attribute( "stroke-linecap", "round" );
            break;
    }

    switch( line_join ) {
        case LineJoin::kOmit:
            break;
        case LineJoin::kMiter:
            out << svg_attribute( "stroke-linejoin", "miter" );
            out << svg_attribute( "stroke-miterlimit", miterlimit );
            break;
        case LineJoin::kRound:
            out << svg_attribute( "stroke-linejoin", "round" );
            break;
        case LineJoin::kBevel:
            out << svg_attribute( "stroke-linejoin", "bevel" );
            break;
    }

    if( dash_array.size() > 0 ) {
        out << svg_attribute( "stroke-dasharray", join( dash_array, " " ));
        out << svg_attribute( "stroke-dashoffset", dash_offset );
    }
}

// =================================================================================================
//     Svg Fill
// =================================================================================================

// -------------------------------------------------------------
//     Constructors and Rule of Five
// -------------------------------------------------------------

SvgFill::SvgFill( SvgFill::Type type )
    : type( type )
    , color()
    , rule( Rule::kNone )
{}

SvgFill::SvgFill( Color color )
    : type( SvgFill::Type::kColor )
    , color( color )
    , rule( Rule::kNone )
{}

SvgFill::SvgFill( std::string gradient_id_value )
    : SvgFill( Type::kGradient )
{
    gradient_id = gradient_id_value;
}

// -------------------------------------------------------------
//     Drawing Function
// -------------------------------------------------------------

void SvgFill::write( std::ostream& out ) const
{
    // Treat special cases.
    if( type == Type::kOmit ) {
        return;
    }
    if( type == Type::kNone ) {
        out << svg_attribute( "fill", "none" );
        return;
    }
    if( type == Type::kGradient ) {
        out << svg_attribute( "fill", "url(#" + gradient_id + ")" );
        return;
    }

    out << svg_attribute( "fill", color_to_hex( color ) );
    out << svg_attribute( "fill-opacity", color.a() );

    switch( rule ) {
        case Rule::kNone:
            break;
        case Rule::kNonZero:
            out << svg_attribute( "fill-rule", "nonzero" );
            break;
        case Rule::kEvenOdd:
            out << svg_attribute( "fill-rule", "evenodd" );
            break;
    }
}

// =================================================================================================
//     Svg Font
// =================================================================================================

// -------------------------------------------------------------
//     Constructors and Rule of Five
// -------------------------------------------------------------

SvgFont::SvgFont( double size_value, std::string const& family_value )
    : size( size_value )
    , family( family_value )
{}

// -------------------------------------------------------------
//     Drawing Function
// -------------------------------------------------------------

void SvgFont::write( std::ostream& out ) const
{
    out << svg_attribute( "font-size", size );
    out << svg_attribute( "font-family", family );
}

// =================================================================================================
//     Svg Transformation
// =================================================================================================

// -------------------------------------------------------------------------
//     Subclass Translate
// -------------------------------------------------------------------------

void SvgTransform::Translate::write( std::ostream &out ) const
{
    if( tx != 0.0 || ty != 0.0 ) {
        out << "translate( " << tx << ", " << ty << " )";
    }
}

SvgPoint SvgTransform::Translate::apply( SvgPoint const& p ) const
{
    return SvgPoint( p.x + tx, p.y + ty );
}

// -------------------------------------------------------------------------
//     Subclass Rotate
// -------------------------------------------------------------------------

void SvgTransform::Rotate::write( std::ostream &out ) const
{
    if( a != 0.0 ) {
        if( cx != 0.0 || cy != 0.0 ) {
            out << "rotate( " << a << ", " << cx << ", " << cy << " )";
        } else {
            out << "rotate( " << a << " )";
        }
    }
}

SvgPoint SvgTransform::Rotate::apply( SvgPoint const& p ) const
{
    // Convert to radians, and precompute sin and cos.
    auto const r = a * utils::PI / 180.0;
    auto const sr = std::sin(r);
    auto const cr = std::cos(r);

    // We need to subtract the offset, rotate, and add the offset again.
    // https://stackoverflow.com/a/2259502/4184258
    // See also https://www.w3.org/TR/SVGTiny12/coords.html
    auto const nx = p.x - cx;
    auto const ny = p.y - cy;
    auto const rx = nx * cr - ny * sr;
    auto const ry = nx * sr + ny * cr;
    auto const fx = rx + cx;
    auto const fy = ry + cy;
    return SvgPoint( fx, fy );
}

// -------------------------------------------------------------------------
//     Subclass Scale
// -------------------------------------------------------------------------

void SvgTransform::Scale::write( std::ostream &out ) const
{
    if( sx != 1.0 || sy != 1.0 ) {
        if( sx == sy ) {
            out << "scale( " << sx << " )";
        } else {
            out << "scale( " << sx << ", " << sy << " )";
        }
    }
}

SvgPoint SvgTransform::Scale::apply( SvgPoint const& p ) const
{
    return SvgPoint( p.x * sx, p.y * sy );
}

// -------------------------------------------------------------------------
//     Subclass SkewX
// -------------------------------------------------------------------------

void SvgTransform::SkewX::write( std::ostream &out ) const
{
    if( ax != 0.0 ) {
        out << "skewX( " << ax << " )";
    }
}

SvgPoint SvgTransform::SkewX::apply( SvgPoint const& p ) const
{
    // Convert to radians, and compute skew.
    auto const rx = ax * utils::PI / 180.0;
    return SvgPoint( p.x + p.y * std::tan(rx), p.y );
}

// -------------------------------------------------------------------------
//     Subclass SkewY
// -------------------------------------------------------------------------

void SvgTransform::SkewY::write( std::ostream &out ) const
{
    if( ay != 0.0 ) {
        out << "skewY( " << ay << " )";
    }
}

SvgPoint SvgTransform::SkewY::apply( SvgPoint const& p ) const
{
    // Convert to radians, and compute skew.
    auto const ry = ay * utils::PI / 180.0;
    return SvgPoint( p.x, p.x * std::tan(ry) + p.y );
}

// -------------------------------------------------------------------------
//     Subclass Matrix
// -------------------------------------------------------------------------

void SvgTransform::Matrix::write( std::ostream &out ) const
{
    if( a != 1.0 || b != 0.0 || c != 0.0 || d != 1.0 || e != 0.0 || f != 0.0 ) {
        out << "matrix( " << a << ", " << b << ", " << c << ", ";
        out               << d << ", " << e << ", " << f << " )";
    }
}

SvgPoint SvgTransform::Matrix::apply( SvgPoint const& p ) const
{
    return SvgPoint(
        p.x * a + p.y * c + e,
        p.x * b + p.y * d + f
    );
}

// -------------------------------------------------------------------------
//     SvgTransform Main Class
// -------------------------------------------------------------------------

void SvgTransform::append( Transformation&& t )
{
    transformations.push_back( std::move( t ));
}

void SvgTransform::append( Transformation const& t )
{
    transformations.push_back( t );
}

SvgPoint SvgTransform::apply( SvgPoint const& p ) const
{
    // Svg transforms are applied from last to first,
    // see https://stackoverflow.com/a/18587460/4184258 for the rationale.
    auto r = p;
    for( auto t = transformations.crbegin(); t != transformations.crend(); ++t ) {
        r = t->apply( r );
    }
    return r;
}

SvgBox SvgTransform::apply( SvgBox const& b ) const
{
    assert( b.top_left.x <= b.bottom_right.x );
    assert( b.top_left.y <= b.bottom_right.y );

    // Compute the transformed corners.
    auto const tr_tl = apply( SvgPoint( b.top_left.x,     b.top_left.y ));
    auto const tr_tr = apply( SvgPoint( b.bottom_right.x, b.top_left.y ));
    auto const tr_bl = apply( SvgPoint( b.top_left.x,     b.bottom_right.y ));
    auto const tr_br = apply( SvgPoint( b.bottom_right.x, b.bottom_right.y ));

    // Get the overall surrounding box that fits all.
    auto const tlx = std::min( std::initializer_list<double>{ tr_tl.x, tr_tr.x, tr_bl.x, tr_br.x });
    auto const tly = std::min( std::initializer_list<double>{ tr_tl.y, tr_tr.y, tr_bl.y, tr_br.y });
    auto const brx = std::max( std::initializer_list<double>{ tr_tl.x, tr_tr.x, tr_bl.x, tr_br.x });
    auto const bry = std::max( std::initializer_list<double>{ tr_tl.y, tr_tr.y, tr_bl.y, tr_br.y });

    return SvgBox( SvgPoint( tlx, tly ), SvgPoint( brx, bry ));
}

void SvgTransform::write( std::ostream& out ) const
{
    if( ! transformations.empty() ) {
        out << " transform=\"";
        for( auto const& t : transformations ) {
            if( &t != &transformations[0] ) {
                out << " ";
            }
            t.write( out );
        }
        out << "\"";
    }
}

} // namespace utils
} // namespace genesis
