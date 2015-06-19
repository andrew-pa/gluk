/* ============================================================================
* Freetype GL - A C OpenGL Freetype engine
* Platform:    Any
* WWW:         http://code.google.com/p/freetype-gl/
* ----------------------------------------------------------------------------
* Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Nicolas P. Rougier.
* ============================================================================
moved vector.h, texture_atlas.h and texture_font.h into one header (Andrew Palmer)

from texture_atlas.h's intro note:
* This source is based on the article by Jukka Jylänki :
* "A Thousand Ways to Pack the Bin - A Practical Approach to
* Two-Dimensional Rectangle Bin Packing", February 27, 2010.
*
* More precisely, this is an implementation of the Skyline Bottom-Left
* algorithm based on C++ sources provided by Jukka Jylänki at:
* http://clb.demon.fi/files/RectangleBinPack/
*/
#pragma once
#include "cmmn.h"
#include <stdlib.h>
#include <stddef.h>

namespace freetype_gl {
#pragma region VECTOR_H
    /**
     * @file   vector.h
     * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
     *
     * @defgroup vector Vector
     *
     * The vector structure and accompanying functions loosely mimic the STL C++
     * vector class. It is used by @ref texture-atlas (for storing nodes), @ref
     * texture-font (for storing glyphs) and @ref font-manager (for storing fonts).
     * More information at http://www.cppreference.com/wiki/container/vector/start
     *
     * <b>Example Usage</b>:
     * @code
     * #include "vector.h"
     *
     * int main( int arrgc, char *argv[] )
     * {
     *   int i,j = 1;
     *   vector_t * vector = vector_new( sizeof(int) );
     *   vector_push_back( &i );
     *
     *   j = * (int *) vector_get( vector, 0 );
     *   vector_delete( vector);
     *
     *   return 0;
     * }
     * @endcode
     *
     * @{
     */

    /**
     *  Generic vector structure.
     *
     * @memberof vector
     */
    typedef struct vector_t
     {
         /** Pointer to dynamically allocated items. */
         void * items;

         /** Number of items that can be held in currently allocated storage. */
         size_t capacity;

         /** Number of items. */
         size_t size;

         /** Size (in bytes) of a single item. */
         size_t item_size;
    } vector_t;


    /**
     * Creates a new empty vector.
     *
     * @param   item_size    item size in bytes
     * @return               a new empty vector
     *
     */
      vector_t *
      vector_new( size_t item_size );


    /**
     *  Deletes a vector.
     *
     *  @param self a vector structure
     *
     */
      void
      vector_delete( vector_t *self );


    /**
     *  Returns a pointer to the item located at specified index.
     *
     *  @param  self  a vector structure
     *  @param  index the index of the item to be returned
     *  @return       pointer on the specified item
     */
      const void *
      vector_get( const vector_t *self,
                  size_t index );


    /**
     *  Returns a pointer to the first item.
     *
     *  @param  self  a vector structure
     *  @return       pointer on the first item
     */
      const void *
      vector_front( const vector_t *self );


    /**
     *  Returns a pointer to the last item
     *
     *  @param  self  a vector structure
     *  @return pointer on the last item
     */
      const void *
      vector_back( const vector_t *self );


    /**
     *  Check if an item is contained within the vector.
     *
     *  @param  self  a vector structure
     *  @param  item  item to be searched in the vector
     *  @param  cmp   a pointer a comparison function
     *  @return       1 if item is contained within the vector, 0 otherwise
     */
      int
      vector_contains( const vector_t *self,
                       const void *item,
                       int (*cmp)(const void *, const void *) );


    /**
     *  Checks whether the vector is empty.
     *
     *  @param  self  a vector structure
     *  @return       1 if the vector is empty, 0 otherwise
     */
      int
      vector_empty( const vector_t *self );


    /**
     *  Returns the number of items
     *
     *  @param  self  a vector structure
     *  @return       number of items
     */
      size_t
      vector_size( const vector_t *self );


    /**
     *  Reserve storage such that it can hold at last size items.
     *
     *  @param  self  a vector structure
     *  @param  size  the new storage capacity
     */
      void
      vector_reserve( vector_t *self,
                      const size_t size );


    /**
     *  Returns current storage capacity
     *
     *  @param  self  a vector structure
     *  @return       storage capacity
     */
      size_t
      vector_capacity( const vector_t *self );


    /**
     *  Decrease capacity to fit actual size.
     *
     *  @param  self  a vector structure
     */
      void
      vector_shrink( vector_t *self );


    /**
     *  Removes all items.
     *
     *  @param  self  a vector structure
     */
      void
      vector_clear( vector_t *self );


    /**
     *  Replace an item.
     *
     *  @param  self  a vector structure
     *  @param  index the index of the item to be replaced
     *  @param  item  the new item
     */
      void
      vector_set( vector_t *self,
                  const size_t index,
                  const void *item );


    /**
     *  Erase an item.
     *
     *  @param  self  a vector structure
     *  @param  index the index of the item to be erased
     */
      void
      vector_erase( vector_t *self,
                    const size_t index );


    /**
     *  Erase a range of items.
     *
     *  @param  self  a vector structure
     *  @param  first the index of the first item to be erased
     *  @param  last  the index of the last item to be erased
     */
      void
      vector_erase_range( vector_t *self,
                          const size_t first,
                          const size_t last );


    /**
     *  Appends given item to the end of the vector.
     *
     *  @param  self a vector structure
     *  @param  item the item to be inserted
     */
      void
      vector_push_back( vector_t *self,
                        const void *item );


    /**
     *  Removes the last item of the vector.
     *
     *  @param  self a vector structure
     */
      void
      vector_pop_back( vector_t *self );


    /**
     *  Resizes the vector to contain size items
     *
     *  If the current size is less than size, additional items are appended and
     *  initialized with value. If the current size is greater than size, the
     *  vector is reduced to its first size elements.
     *
     *  @param  self a vector structure
     *  @param  size the new size
     */
      void
      vector_resize( vector_t *self,
                     const size_t size );


    /**
     *  Insert a single item at specified index.
     *
     *  @param  self  a vector structure
     *  @param  index location before which to insert item
     *  @param  item  the item to be inserted
     */
      void
      vector_insert( vector_t *self,
                     const size_t index,
                     const void *item );


    /**
     *  Insert raw data at specified index.
     *
     *  @param  self  a vector structure
     *  @param  index location before which to insert item
     *  @param  data  a pointer to the items to be inserted
     *  @param  count the number of items to be inserted
     */
      void
      vector_insert_data( vector_t *self,
                          const size_t index,
                          const void * data,
                          const size_t count );


    /**
     *  Append raw data to the end of the vector.
     *
     *  @param  self  a vector structure
     *  @param  data  a pointer to the items to be inserted
     *  @param  count the number of items to be inserted
     */
      void
      vector_push_back_data( vector_t *self,
                             const void * data,
                             const size_t count );


    /**
     *  Sort vector items according to cmp function.
     *
     *  @param  self  a vector structure
     *  @param  cmp   a pointer a comparison function
     */
      void
      vector_sort( vector_t *self,
                   int (*cmp)(const void *, const void *) );


    /** @} */
#pragma endregion

#pragma region TEXTURE_ATLAS_H
    /**
    * @file   texture-atlas.h
    * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
    *
    * @defgroup texture-atlas Texture atlas
    *
    * A texture atlas is used to pack several small regions into a single texture.
    *
    * The actual implementation is based on the article by Jukka Jylänki : "A
    * Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
    * Rectangle Bin Packing", February 27, 2010.
    * More precisely, this is an implementation of the Skyline Bottom-Left
    * algorithm based on C++ sources provided by Jukka Jylänki at:
    * http://clb.demon.fi/files/RectangleBinPack/
    *
    *
    * Example Usage:
    * @code
    * #include "texture-atlas.h"
    *
    * ...
    *
    * / Creates a new atlas of 512x512 with a depth of 1
    * texture_atlas_t * atlas = texture_atlas_new( 512, 512, 1 );
    *
    * // Allocates a region of 20x20
    * ivec4 region = texture_atlas_get_region( atlas, 20, 20 );
    *
    * // Fill region with some data
    * texture_atlas_set_region( atlas, region.x, region.y, region.width, region.height, data, stride )
    *
    * ...
    *
    * @endcode
    *
    * @{
    */


    /**
    * A texture atlas is used to pack several small regions into a single texture.
    */
    typedef struct texture_atlas_t
    {
    /**
     * Allocated nodes
     */
    vector_t * nodes;

    /**
     *  Width (in pixels) of the underlying texture
     */
    size_t width;

    /**
     * Height (in pixels) of the underlying texture
     */
    size_t height;

    /**
     * Depth (in bytes) of the underlying texture
     */
    size_t depth;

    /**
     * Allocated surface size
     */
    size_t used;

    /**
     * Texture identity (OpenGL)
     */
    unsigned int id;

    /**
     * Atlas data
     */
    unsigned char * data;

    } texture_atlas_t;



    /**
    * Creates a new empty texture atlas.
    *
    * @param   width   width of the atlas
    * @param   height  height of the atlas
    * @param   depth   bit depth of the atlas
    * @return          a new empty texture atlas.
    *
    */
    texture_atlas_t *
    texture_atlas_new( const size_t width,
                     const size_t height,
                     const size_t depth );


    /**
    *  Deletes a texture atlas.
    *
    *  @param self a texture atlas structure
    *
    */
    void
    texture_atlas_delete( texture_atlas_t * self );


    /**
    *  Upload atlas to video memory.
    *
    *  @param self a texture atlas structure
    *
    */
    void
    texture_atlas_upload( texture_atlas_t * self );


    /**
    *  Allocate a new region in the atlas.
    *
    *  @param self   a texture atlas structure
    *  @param width  width of the region to allocate
    *  @param height height of the region to allocate
    *  @return       Coordinates of the allocated region
    *
    */
    ivec4
    texture_atlas_get_region( texture_atlas_t * self,
                            const size_t width,
                            const size_t height );


    /**
    *  Upload data to the specified atlas region.
    *
    *  @param self   a texture atlas structure
    *  @param x      x coordinate the region
    *  @param y      y coordinate the region
    *  @param width  width of the region
    *  @param height height of the region
    *  @param data   data to be uploaded into the specified region
    *  @param stride stride of the data
    *
    */
    void
    texture_atlas_set_region( texture_atlas_t * self,
                            const size_t x,
                            const size_t y,
                            const size_t width,
                            const size_t height,
                            const unsigned char *data,
                            const size_t stride );

    /**
    *  Remove all allocated regions from the atlas.
    *
    *  @param self   a texture atlas structure
    */
    void
    texture_atlas_clear( texture_atlas_t * self );


    /** @} */
#pragma endregion

#pragma region TEXTURE_FONT_H
    /**
     * A structure that hold a kerning value relatively to a charcode.
     *
     * This structure cannot be used alone since the (necessary) right charcode is
     * implicitely held by the owner of this structure.
     */
    typedef struct kerning_t
    {
        /**
         * Left character code in the kern pair.
         */
        wchar_t charcode;

        /**
         * Kerning value (in fractional pixels).
         */
        float kerning;

    } kerning_t;




    /*
     * Glyph metrics:
     * --------------
     *
     *                       xmin                     xmax
     *                        |                         |
     *                        |<-------- width -------->|
     *                        |                         |
     *              |         +-------------------------+----------------- ymax
     *              |         |    ggggggggg   ggggg    |     ^        ^
     *              |         |   g:::::::::ggg::::g    |     |        |
     *              |         |  g:::::::::::::::::g    |     |        |
     *              |         | g::::::ggggg::::::gg    |     |        |
     *              |         | g:::::g     g:::::g     |     |        |
     *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
     *              |         | g:::::g     g:::::g     |     |        |
     *              |         | g::::::g    g:::::g     |     |        |
     *              |         | g:::::::ggggg:::::g     |     |        |
     *              |         |  g::::::::::::::::g     |     |      height
     *              |         |   gg::::::::::::::g     |     |        |
     *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
     *            / |         |             g:::::g     |              |
     *     origin   |         | gggggg      g:::::g     |              |
     *              |         | g:::::gg   gg:::::g     |              |
     *              |         |  g::::::ggg:::::::g     |              |
     *              |         |   gg:::::::::::::g      |              |
     *              |         |     ggg::::::ggg        |              |
     *              |         |         gggggg          |              v
     *              |         +-------------------------+----------------- ymin
     *              |                                   |
     *              |------------- advance_x ---------->|
     */

    /**
     * A structure that describe a glyph.
     */
    typedef struct texture_glyph_t
    {
        /**
         * Wide character this glyph represents
         */
        wchar_t charcode;

        /**
         * Glyph id (used for display lists)
         */
        unsigned int id;

        /**
         * Glyph's width in pixels.
         */
        size_t width;

        /**
         * Glyph's height in pixels.
         */
        size_t height;

        /**
         * Glyph's left bearing expressed in integer pixels.
         */
        int offset_x;

        /**
         * Glyphs's top bearing expressed in integer pixels.
         *
         * Remember that this is the distance from the baseline to the top-most
         * glyph scanline, upwards y coordinates being positive.
         */
        int offset_y;

        /**
         * For horizontal text layouts, this is the horizontal distance (in
         * fractional pixels) used to increment the pen position when the glyph is
         * drawn as part of a string of text.
         */
        float advance_x;

        /**
         * For vertical text layouts, this is the vertical distance (in fractional
         * pixels) used to increment the pen position when the glyph is drawn as
         * part of a string of text.
         */
        float advance_y;

        /**
         * First normalized texture coordinate (x) of top-left corner
         */
        float s0;

        /**
         * Second normalized texture coordinate (y) of top-left corner
         */
        float t0;

        /**
         * First normalized texture coordinate (x) of bottom-right corner
         */
        float s1;

        /**
         * Second normalized texture coordinate (y) of bottom-right corner
         */
        float t1;

        /**
         * A vector of kerning pairs relative to this glyph.
         */
        vector_t * kerning;

        /**
         * Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
         */
        int outline_type;

        /**
         * Glyph outline thickness
         */
        float outline_thickness;

    } texture_glyph_t;

	typedef enum {
		TEXTURE_FONT_FILE = 0,
		TEXTURE_FONT_MEMORY,
	} load_type_t;

    /**
     *  Texture font structure.
     */
    typedef struct texture_font_t
    {
        /**
         * Vector of glyphs contained in this font.
         */
        vector_t * glyphs;

        /**
         * Atlas structure to store glyphs data.
         */
        texture_atlas_t * atlas;

        /**
         * font location
         */
        load_type_t location;

        union {
            /**
             * Font filename, for when location == TEXTURE_FONT_FILE
             */
            char *filename;

            /**
             * Font memory address, for when location == TEXTURE_FONT_MEMORY
             */
            struct {
                const void *base;
                size_t size;
            } memory;
        };

        /**
         * Font size
         */
        float size;

        /**
         * Whether to use autohint when rendering font
         */
        int hinting;

        /**
         * Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
         */
        int outline_type;

        /**
         * Outline thickness
         */
        float outline_thickness;

        /**
         * Whether to use our own lcd filter.
         */
        int filtering;

        /**
         * Whether to use kerning if available
         */
        int kerning;

        /**
         * LCD filter weights
         */
        unsigned char lcd_weights[5];

        /**
         * This field is simply used to compute a default line spacing (i.e., the
         * baseline-to-baseline distance) when writing text with this font. Note
         * that it usually is larger than the sum of the ascender and descender
         * taken as absolute values. There is also no guarantee that no glyphs
         * extend above or below subsequent baselines when using this distance.
         */
        float height;

        /**
         * This field is the distance that must be placed between two lines of
         * text. The baseline-to-baseline distance should be computed as:
         * ascender - descender + linegap
         */
        float linegap;

        /**
         * The ascender is the vertical distance from the horizontal baseline to
         * the highest 'character' coordinate in a font face. Unfortunately, font
         * formats define the ascender differently. For some, it represents the
         * ascent of all capital latin characters (without accents), for others it
         * is the ascent of the highest accented character, and finally, other
         * formats define it as being equal to bbox.yMax.
         */
        float ascender;

        /**
         * The descender is the vertical distance from the horizontal baseline to
         * the lowest 'character' coordinate in a font face. Unfortunately, font
         * formats define the descender differently. For some, it represents the
         * descent of all capital latin characters (without accents), for others it
         * is the ascent of the lowest accented character, and finally, other
         * formats define it as being equal to bbox.yMin. This field is negative
         * for values below the baseline.
         */
        float descender;

        /**
         * The position of the underline line for this face. It is the center of
         * the underlining stem. Only relevant for scalable formats.
         */
        float underline_position;

        /**
         * The thickness of the underline for this face. Only relevant for scalable
         * formats.
         */
        float underline_thickness;

    } texture_font_t;



    /**
     * This function creates a new texture font from given filename and size.  The
     * texture atlas is used to store glyph on demand. Note the depth of the atlas
     * will determine if the font is rendered as alpha channel only (depth = 1) or
     * RGB (depth = 3) that correspond to subpixel rendering (if available on your
     * freetype implementation).
     *
     * @param atlas     A texture atlas
     * @param pt_size   Size of font to be created (in points)
     * @param filename  A font filename
     *
     * @return A new empty font (no glyph inside yet)
     *
     */
      texture_font_t *
      texture_font_new_from_file( texture_atlas_t * atlas,
                                  const float pt_size,
                                  const char * filename );


    /**
     * This function creates a new texture font from a memory location and size.
     * The texture atlas is used to store glyph on demand. Note the depth of the
     * atlas will determine if the font is rendered as alpha channel only
     * (depth = 1) or RGB (depth = 3) that correspond to subpixel rendering (if
     * available on your freetype implementation).
     *
     * @param atlas       A texture atlas
     * @param pt_size     Size of font to be created (in points)
     * @param memory_base Start of the font file in memory
     * @param memory_size Size of the font file memory region, in bytes
     *
     * @return A new empty font (no glyph inside yet)
     *
     */
      texture_font_t *
      texture_font_new_from_memory( texture_atlas_t *atlas,
                                    float pt_size,
                                    const void *memory_base,
                                    size_t memory_size );

    /**
     * Delete a texture font. Note that this does not delete the glyph from the
     * texture atlas.
     *
     * @param self a valid texture font
     */
      void
      texture_font_delete( texture_font_t * self );


    /**
     * Request a new glyph from the font. If it has not been created yet, it will
     * be.
     *
     * @param self     A valid texture font
     * @param charcode Character codepoint to be loaded.
     *
     * @return A pointer on the new glyph or 0 if the texture atlas is not big
     *         enough
     *
     */
      texture_glyph_t *
      texture_font_get_glyph( texture_font_t * self,
                              wchar_t charcode );


    /**
     * Request the loading of several glyphs at once.
     *
     * @param self      a valid texture font
     * @param charcodes character codepoints to be loaded.
     *
     * @return Number of missed glyph if the texture is not big enough to hold
     *         every glyphs.
     */
      size_t
      texture_font_load_glyphs( texture_font_t * self,
                                const wchar_t * charcodes );

    /**
     * Get the kerning between two horizontal glyphs.
     *
     * @param self      a valid texture glyph
     * @param charcode  codepoint of the peceding glyph
     *
     * @return x kerning value
     */
    float
    texture_glyph_get_kerning( const texture_glyph_t * self,
                               const wchar_t charcode );


    /**
     * Creates a new empty glyph
     *
     * @return a new empty glyph (not valid)
     */
    texture_glyph_t *
    texture_glyph_new( void );

    /** @} */
#pragma endregion
}
