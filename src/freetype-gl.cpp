#include "freetype-gl.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
*/
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <wchar.h>

namespace freetype_gl {
#pragma region VECTOR_H
    // ------------------------------------------------------------- vector_new ---
    vector_t *
    vector_new( size_t item_size )
    {
        vector_t *self = (vector_t *) malloc( sizeof(vector_t) );
        assert( item_size );

        if( !self )
        {
            fprintf( stderr,
                     "line %d: No more memory for allocating data\n", __LINE__ );
            exit( EXIT_FAILURE );
        }
        self->item_size = item_size;
        self->size      = 0;
        self->capacity  = 1;
        self->items     = malloc( self->item_size * self->capacity );
        return self;
    }



    // ---------------------------------------------------------- vector_delete ---
    void
    vector_delete( vector_t *self )
    {
        assert( self );

        free( self->items );
        free( self );
    }



    // ------------------------------------------------------------- vector_get ---
    const void *
    vector_get( const vector_t *self,
            size_t index )
    {
        assert( self );
        assert( self->size );
        assert( index  < self->size );

        return (char*)(self->items) + index * self->item_size;
    }



    // ----------------------------------------------------------- vector_front ---
    const void *
    vector_front( const vector_t *self )
    {
        assert( self );
        assert( self->size );

        return vector_get( self, 0 );
    }


    // ------------------------------------------------------------ vector_back ---
    const void *
    vector_back( const vector_t *self )
    {
        assert( self );
        assert( self->size );

        return vector_get( self, self->size-1 );
    }


    // -------------------------------------------------------- vector_contains ---
    int
    vector_contains( const vector_t *self,
                 const void *item,
                 int (*cmp)(const void *, const void *) )
    {
        size_t i;
        assert( self );

        for( i=0; i<self->size; ++i )
        {
            if( (*cmp)(item, vector_get(self,i) ) == 0 )
            {
                return 1;
            }
        }
        return 0;
    }


    // ----------------------------------------------------------- vector_empty ---
    int
    vector_empty( const vector_t *self )
    {
        assert( self );

        return self->size == 0;
    }


    // ------------------------------------------------------------ vector_size ---
    size_t
    vector_size( const vector_t *self )
    {
        assert( self );

        return self->size;
    }


    // --------------------------------------------------------- vector_reserve ---
    void
    vector_reserve( vector_t *self,
                const size_t size )
    {
        assert( self );

        if( self->capacity < size)
        {
            self->items = realloc( self->items, size * self->item_size );
            self->capacity = size;
        }
    }


    // -------------------------------------------------------- vector_capacity ---
    size_t
    vector_capacity( const vector_t *self )
    {
        assert( self );

        return self->capacity;
    }


    // ---------------------------------------------------------- vector_shrink ---
    void
    vector_shrink( vector_t *self )
    {
        assert( self );

        if( self->capacity > self->size )
        {
            self->items = realloc( self->items, self->size * self->item_size );
        }
        self->capacity = self->size;
    }


    // ----------------------------------------------------------- vector_clear ---
    void
    vector_clear( vector_t *self )
    {
        assert( self );

        self->size = 0;
    }


    // ------------------------------------------------------------- vector_set ---
    void
    vector_set( vector_t *self,
            const size_t index,
            const void *item )
    {
        assert( self );
        assert( self->size );
        assert( index  < self->size );

        memcpy( (char *)(self->items) + index * self->item_size,
                item, self->item_size );
    }


    // ---------------------------------------------------------- vector_insert ---
    void
    vector_insert( vector_t *self,
               const size_t index,
               const void *item )
    {
        assert( self );
        assert( index <= self->size);

        if( self->capacity <= self->size )
        {
            vector_reserve(self, 2 * self->capacity );
        }
        if( index < self->size )
        {
            memmove( (char *)(self->items) + (index + 1) * self->item_size,
                     (char *)(self->items) + (index + 0) * self->item_size,
                     (self->size - index)  * self->item_size);
        }
        self->size++;
        vector_set( self, index, item );
    }


    // ----------------------------------------------------- vector_erase_range ---
    void
    vector_erase_range( vector_t *self,
                    const size_t first,
                    const size_t last )
    {
        assert( self );
        assert( first < self->size );
        assert( last  < self->size+1 );
        assert( first < last );

        memmove( (char *)(self->items) + first * self->item_size,
                 (char *)(self->items) + last  * self->item_size,
                 (self->size - last)   * self->item_size);
        self->size -= (last-first);
    }


    // ----------------------------------------------------------- vector_erase ---
    void
    vector_erase( vector_t *self,
              const size_t index )
    {
        assert( self );
        assert( index < self->size );

        vector_erase_range( self, index, index+1 );
    }


    // ------------------------------------------------------- vector_push_back ---
    void
    vector_push_back( vector_t *self,
                  const void *item )
    {
        vector_insert( self, self->size, item );
    }


    // -------------------------------------------------------- vector_pop_back ---
    void
    vector_pop_back( vector_t *self )
    {
        assert( self );
        assert( self->size );

        self->size--;
    }


    // ---------------------------------------------------------- vector_resize ---
    void
    vector_resize( vector_t *self,
               const size_t size )
    {
        assert( self );

        if( size > self->capacity)
        {
            vector_reserve( self, size );
            self->size = self->capacity;
        }
        else
        {
            self->size = size;
        }
    }


    // -------------------------------------------------- vector_push_back_data ---
    void
    vector_push_back_data( vector_t *self,
                       const void * data,
                       const size_t count )
    {
        assert( self );
        assert( data );
        assert( count );

        if( self->capacity < (self->size+count) )
        {
            vector_reserve(self, self->size+count);
        }
        memmove( (char *)(self->items) + self->size * self->item_size, data,
                 count*self->item_size );
        self->size += count;
    }


    // ----------------------------------------------------- vector_insert_data ---
    void
    vector_insert_data( vector_t *self,
                    const size_t index,
                    const void * data,
                    const size_t count )
    {
        assert( self );
        assert( index < self->size );
        assert( data );
        assert( count );

        if( self->capacity < (self->size+count) )
        {
            vector_reserve(self, self->size+count);
        }
        memmove( (char *)(self->items) + (index + count ) * self->item_size,
                 (char *)(self->items) + (index ) * self->item_size,
                 count*self->item_size );
        memmove( (char *)(self->items) + index * self->item_size, data,
                 count*self->item_size );
        self->size += count;
    }


    // ------------------------------------------------------------ vector_sort ---
    void
    vector_sort( vector_t *self,
             int (*cmp)(const void *, const void *) )
    {
        assert( self );
        assert( self->size );

        qsort(self->items, self->size, self->item_size, cmp);
    }
#pragma endregion

#pragma region TEXTURE_ATLAS_H
    // ------------------------------------------------------ texture_atlas_new ---
    texture_atlas_t *
    texture_atlas_new( const size_t width,
                       const size_t height,
                       const size_t depth )
    {
        texture_atlas_t *self = (texture_atlas_t *) malloc( sizeof(texture_atlas_t) );

        // We want a one pixel border around the whole atlas to avoid any artefact when
        // sampling texture
        ivec3 node = ivec3(1,1,width-2);

        assert( (depth == 1) || (depth == 3) || (depth == 4) );
        if( self == NULL)
        {
            fprintf( stderr,
                     "line %d: No more memory for allocating data\n", __LINE__ );
            exit( EXIT_FAILURE );
        }
        self->nodes = vector_new( sizeof(ivec3) );
        self->used = 0;
        self->width = width;
        self->height = height;
        self->depth = depth;
        self->id = 0;

        vector_push_back( self->nodes, &node );
        self->data = (unsigned char *)
            calloc( width*height*depth, sizeof(unsigned char) );

        if( self->data == NULL)
        {
            fprintf( stderr,
                     "line %d: No more memory for allocating data\n", __LINE__ );
            exit( EXIT_FAILURE );
        }

        return self;
    }


    // --------------------------------------------------- texture_atlas_delete ---
    void
    texture_atlas_delete( texture_atlas_t *self )
    {
        assert( self );
        vector_delete( self->nodes );
        if( self->data )
        {
            free( self->data );
        }
        if( self->id )
        {
            glDeleteTextures( 1, &self->id );
        }
        free( self );
    }


    // ----------------------------------------------- texture_atlas_set_region ---
    void
    texture_atlas_set_region( texture_atlas_t * self,
                              const size_t x,
                              const size_t y,
                              const size_t width,
                              const size_t height,
                              const unsigned char * data,
                              const size_t stride )
    {
        size_t i;
        size_t depth;
        size_t charsize;

        assert( self );
        assert( x > 0);
        assert( y > 0);
        assert( x < (self->width-1));
        assert( (x + width) <= (self->width-1));
        assert( y < (self->height-1));
        assert( (y + height) <= (self->height-1));

        depth = self->depth;
		//assert(depth == 1);
        charsize = sizeof(char);
        for( i=0; i<height; ++i )
        {
            memcpy( self->data+((y+i)*self->width + x ) * charsize * depth,
                   data + (i*stride) * charsize, width * charsize * depth  );
        }
    }


    // ------------------------------------------------------ texture_atlas_fit ---
    int
    texture_atlas_fit( texture_atlas_t * self,
                       const size_t index,
                       const size_t width,
                       const size_t height )
    {
        ivec3 *node;
        int x, y, width_left;
    	size_t i;

        assert( self );

        node = (ivec3 *) (vector_get( self->nodes, index ));
        x = node->x;
    	y = node->y;
        width_left = width;
    	i = index;

    	if ( (x + width) > (self->width-1) )
        {
    		return -1;
        }
    	y = node->y;
    	while( width_left > 0 )
    	{
            node = (ivec3 *) (vector_get( self->nodes, i ));
            if( node->y > y )
            {
                y = node->y;
            }
    		if( (y + height) > (self->height-1) )
            {
    			return -1;
            }
    		width_left -= node->z;
    		++i;
    	}
    	return y;
    }


    // ---------------------------------------------------- texture_atlas_merge ---
    void
    texture_atlas_merge( texture_atlas_t * self )
    {
        ivec3 *node, *next;
        size_t i;

        assert( self );

    	for( i=0; i< self->nodes->size-1; ++i )
        {
            node = (ivec3 *) (vector_get( self->nodes, i ));
            next = (ivec3 *) (vector_get( self->nodes, i+1 ));
    		if( node->y == next->y )
    		{
    			node->z += next->z;
                vector_erase( self->nodes, i+1 );
    			--i;
    		}
        }
    }


    // ----------------------------------------------- texture_atlas_get_region ---
    ivec4
    texture_atlas_get_region( texture_atlas_t * self,
                              const size_t width,
                              const size_t height )
    {

    	int y, best_height, best_width, best_index;
        ivec3 *node, *prev;
        ivec4 region = ivec4(0,0,width,height);
        size_t i;

        assert( self );

        best_height = INT_MAX;
        best_index  = -1;
        best_width = INT_MAX;
    	for( i=0; i<self->nodes->size; ++i )
    	{
            y = texture_atlas_fit( self, i, width, height );
    		if( y >= 0 )
    		{
                node = (ivec3 *) vector_get( self->nodes, i );
    			if( ( (y + height) < best_height ) ||
                    ( ((y + height) == best_height) && (node->z < best_width)) )
    			{
    				best_height = y + height;
    				best_index = i;
    				best_width = node->z;
    				region.x = node->x;
    				region.y = y;
    			}
            }
        }

    	if( best_index == -1 )
        {
            region.x = -1;
            region.y = -1;
            region.z = 0;
            region.w = 0;
            return region;
        }

        node = (ivec3 *) malloc( sizeof(ivec3) );
        if( node == NULL)
        {
            fprintf( stderr,
                     "line %d: No more memory for allocating data\n", __LINE__ );
            exit( EXIT_FAILURE );
        }
        node->x = region.x;
        node->y = region.y + height;
        node->z = width;
        vector_insert( self->nodes, best_index, node );
        free( node );

        for(i = best_index+1; i < self->nodes->size; ++i)
        {
            node = (ivec3 *) vector_get( self->nodes, i );
            prev = (ivec3 *) vector_get( self->nodes, i-1 );

            if (node->x < (prev->x + prev->z) )
            {
                int shrink = prev->x + prev->z - node->x;
                node->x += shrink;
                node->z -= shrink;
                if (node->z <= 0)
                {
                    vector_erase( self->nodes, i );
                    --i;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        texture_atlas_merge( self );
        self->used += width * height;
        return region;
    }


    // ---------------------------------------------------- texture_atlas_clear ---
    void
    texture_atlas_clear( texture_atlas_t * self )
    {
        ivec3 node = ivec3(1,1,1);

        assert( self );
        assert( self->data );

        vector_clear( self->nodes );
        self->used = 0;
        // We want a one pixel border around the whole atlas to avoid any artefact when
        // sampling texture
        node.z = self->width-2;

        vector_push_back( self->nodes, &node );
        memset( self->data, 0, self->width*self->height*self->depth );
    }


    // --------------------------------------------------- texture_atlas_upload ---
    void
    texture_atlas_upload( texture_atlas_t * self )
    {
        assert( self );
        assert( self->data );

        if( !self->id )
        {
            glGenTextures( 1, &self->id );
        }

        glBindTexture( GL_TEXTURE_2D, self->id );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        if( self->depth == 4 )
        {
    #ifdef GL_UNSIGNED_INT_8_8_8_8_REV
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height,
                          0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, self->data );
    #else
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height,
                          0, GL_RGBA, GL_UNSIGNED_BYTE, self->data );
    #endif
        }
        else if( self->depth == 3 )
        {
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, self->width, self->height,
                          0, GL_RGB, GL_UNSIGNED_BYTE, self->data );
        }
        else
        {
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, self->width, self->height,
                          0, GL_RED, GL_UNSIGNED_BYTE, self->data );
        }
    }
#pragma endregion

#pragma region TEXTURE_FONT_H


    const int HRES  = 64;
    const float HRESf = 64.f;
    const int DPI =  72;


    // ------------------------------------------------- texture_font_load_face ---
    static int
    texture_font_load_face(texture_font_t *self, float size,
            FT_Library *library, FT_Face *face)
    {
        FT_Error error;
        FT_Matrix matrix = {
            (int)((1.0/HRES) * 0x10000L),
            (int)((0.0)      * 0x10000L),
            (int)((0.0)      * 0x10000L),
            (int)((1.0)      * 0x10000L)};

        assert(library);
        assert(size);

        /* Initialize library */
        error = FT_Init_FreeType(library);
        if(error) {
			throw;
        }

        /* Load face */
        switch (self->location) {
        case TEXTURE_FONT_FILE:
            error = FT_New_Face(*library, self->filename, 0, face);
            break;

        case TEXTURE_FONT_MEMORY:
            error = FT_New_Memory_Face(*library,
                (const FT_Byte*)self->memory.base, self->memory.size, 0, face);
            break;
        }

        if(error) {
            //fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
            //       __LINE__, FT_Errors[error].code, FT_Errors[error].message);
			FT_Done_FreeType(*library);
			throw;
        }

        /* Select charmap */
        error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
        if(error) {
            FT_Done_Face(*face);
            FT_Done_FreeType(*library);
			throw;
        }

        /* Set char size */
        error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

        if(error) {
            FT_Done_Face(*face);
            FT_Done_FreeType(*library);
			throw;
        }

        /* Set transform matrix */
        FT_Set_Transform(*face, &matrix, NULL);

        return 1;
    }

    static int
    texture_font_get_face_with_size(texture_font_t *self, float size,
            FT_Library *library, FT_Face *face)
    {
        return texture_font_load_face(self, size, library, face);
    }

    static int
    texture_font_get_face(texture_font_t *self,
            FT_Library *library, FT_Face *face)
    {
        return texture_font_get_face_with_size(self, self->size, library, face);
    }

    static int
    texture_font_get_hires_face(texture_font_t *self,
            FT_Library *library, FT_Face *face)
    {
        return texture_font_get_face_with_size(self,
                self->size * 100.f, library, face);
    }

    // ------------------------------------------------------ texture_glyph_new ---
    texture_glyph_t *
    texture_glyph_new(void)
    {
        texture_glyph_t *self = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
        if(self == NULL) {
            fprintf( stderr,
                    "line %d: No more memory for allocating data\n", __LINE__);
            return NULL;
        }

        self->id        = 0;
        self->width     = 0;
        self->height    = 0;
        self->outline_type = 0;
        self->outline_thickness = 0.0;
        self->offset_x  = 0;
        self->offset_y  = 0;
        self->advance_x = 0.0;
        self->advance_y = 0.0;
        self->s0        = 0.0;
        self->t0        = 0.0;
        self->s1        = 0.0;
        self->t1        = 0.0;
        self->kerning   = vector_new( sizeof(kerning_t) );
        return self;
    }


    // --------------------------------------------------- texture_glyph_delete ---
    void
    texture_glyph_delete( texture_glyph_t *self )
    {
        assert( self );
        vector_delete( self->kerning );
        free( self );
    }

    // ---------------------------------------------- texture_glyph_get_kerning ---
    float
    texture_glyph_get_kerning( const texture_glyph_t * self,
                               const wchar_t charcode )
    {
        size_t i;

        assert( self );
        for( i=0; i<vector_size(self->kerning); ++i )
        {
            kerning_t * kerning = (kerning_t *) vector_get( self->kerning, i );
            if( kerning->charcode == charcode )
            {
                return kerning->kerning;
            }
        }
        return 0;
    }


    // ------------------------------------------ texture_font_generate_kerning ---
    void
    texture_font_generate_kerning( texture_font_t *self )
    {
        size_t i, j;
        FT_Library library;
        FT_Face face;
        FT_UInt glyph_index, prev_index;
        texture_glyph_t *glyph, *prev_glyph;
        FT_Vector kerning;

        assert( self );

        /* Load font */
        if(!texture_font_get_face(self, &library, &face))
            return;

        /* For each glyph couple combination, check if kerning is necessary */
        /* Starts at index 1 since 0 is for the special backgroudn glyph */
        for( i=1; i<self->glyphs->size; ++i )
        {
            glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
            glyph_index = FT_Get_Char_Index( face, glyph->charcode );
            vector_clear( glyph->kerning );

            for( j=1; j<self->glyphs->size; ++j )
            {
                prev_glyph = *(texture_glyph_t **) vector_get( self->glyphs, j );
                prev_index = FT_Get_Char_Index( face, prev_glyph->charcode );
                FT_Get_Kerning( face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning );
                // printf("%c(%d)-%c(%d): %ld\n",
                //       prev_glyph->charcode, prev_glyph->charcode,
                //       glyph_index, glyph_index, kerning.x);
                if( kerning.x )
                {
                    kerning_t k = {prev_glyph->charcode, kerning.x / (float)(HRESf*HRESf)};
                    vector_push_back( glyph->kerning, &k );
                }
            }
        }

        FT_Done_Face( face );
        FT_Done_FreeType( library );
    }

    // ------------------------------------------------------ texture_font_init ---
    static int
    texture_font_init(texture_font_t *self)
    {
        FT_Library library;
        FT_Face face;
        FT_Size_Metrics metrics;

        assert(self->atlas);
        assert(self->size > 0);
        assert((self->location == TEXTURE_FONT_FILE && self->filename)
            || (self->location == TEXTURE_FONT_MEMORY
                && self->memory.base && self->memory.size));

        self->glyphs = vector_new(sizeof(texture_glyph_t *));
        self->height = 0;
        self->ascender = 0;
        self->descender = 0;
        self->outline_type = 0;
        self->outline_thickness = 0.0;
        self->hinting = 1;
        self->kerning = 1;
        self->filtering = 1;

        // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
        // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
        self->lcd_weights[0] = 0x10;
        self->lcd_weights[1] = 0x40;
        self->lcd_weights[2] = 0x70;
        self->lcd_weights[3] = 0x40;
        self->lcd_weights[4] = 0x10;

        /* Get font metrics at high resolution */
        if (!texture_font_get_hires_face(self, &library, &face))
            return -1;

        self->underline_position = face->underline_position / (float)(HRESf*HRESf) * self->size;
        self->underline_position = round( self->underline_position );
        if( self->underline_position > -2 )
        {
            self->underline_position = -2.0;
        }

        self->underline_thickness = face->underline_thickness / (float)(HRESf*HRESf) * self->size;
        self->underline_thickness = round( self->underline_thickness );
        if( self->underline_thickness < 1 )
        {
            self->underline_thickness = 1.0;
        }

        metrics = face->size->metrics;
        self->ascender = (metrics.ascender >> 6) / 100.0;
        self->descender = (metrics.descender >> 6) / 100.0;
        self->height = (metrics.height >> 6) / 100.0;
        self->linegap = self->height - self->ascender + self->descender;
        FT_Done_Face( face );
        FT_Done_FreeType( library );

        /* -1 is a special glyph */
        texture_font_get_glyph( self, -1 );

        return 0;
    }

    // --------------------------------------------- texture_font_new_from_file ---
    texture_font_t *
    texture_font_new_from_file(texture_atlas_t *atlas, const float pt_size,
            const char *filename)
    {
        texture_font_t *self;

        assert(filename);

        self = (texture_font_t*)calloc(1, sizeof(*self));
        if (!self) {
            fprintf(stderr,
                    "line %d: No more memory for allocating data\n", __LINE__);
            return NULL;
        }

        self->atlas = atlas;
        self->size  = pt_size;

        self->location = TEXTURE_FONT_FILE;
        self->filename = _strdup(filename);

        if (texture_font_init(self)) {
            texture_font_delete(self);
            return NULL;
        }

        return self;
    }

    // ------------------------------------------- texture_font_new_from_memory ---
    texture_font_t *
    texture_font_new_from_memory(texture_atlas_t *atlas, float pt_size,
            const void *memory_base, size_t memory_size)
    {
        texture_font_t *self;

        assert(memory_base);
        assert(memory_size);

        self = (texture_font_t*)calloc(1, sizeof(*self));
        if (!self) {
            fprintf(stderr,
                    "line %d: No more memory for allocating data\n", __LINE__);
            return NULL;
        }

        self->atlas = atlas;
        self->size  = pt_size;

        self->location = TEXTURE_FONT_MEMORY;
        self->memory.base = memory_base;
        self->memory.size = memory_size;

        if (texture_font_init(self)) {
            texture_font_delete(self);
            return NULL;
        }

        return self;
    }

    // ---------------------------------------------------- texture_font_delete ---
    void
    texture_font_delete( texture_font_t *self )
    {
        size_t i;
        texture_glyph_t *glyph;

        assert( self );

        if(self->location == TEXTURE_FONT_FILE && self->filename)
            free( self->filename );

        for( i=0; i<vector_size( self->glyphs ); ++i)
        {
            glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
            texture_glyph_delete( glyph);
        }

        vector_delete( self->glyphs );
        free( self );
    }


    // ----------------------------------------------- texture_font_load_glyphs ---
    size_t
    texture_font_load_glyphs( texture_font_t * self,
                              const wchar_t * charcodes )
    {
        size_t i, j, x, y, width, height, depth, w, h;

        FT_Library library;
        FT_Error error;
        FT_Face face;
        FT_Glyph ft_glyph;
        FT_GlyphSlot slot;
        FT_Bitmap ft_bitmap;

        FT_UInt glyph_index;
        texture_glyph_t *glyph;
        FT_Int32 flags = 0;
        int ft_glyph_top = 0;
        int ft_glyph_left = 0;

        ivec4 region;
        size_t missed = 0;
        char pass;

        assert( self );
        assert( charcodes );


        width  = self->atlas->width;
        height = self->atlas->height;
        depth  = self->atlas->depth;

        if (!texture_font_get_face(self, &library, &face))
            return wcslen(charcodes);

        /* Load each glyph */
        for( i = 0; i < wcslen(charcodes); ++i ) {
            pass = 0;
            /* Check if charcode has been already loaded */
            for( j = 0; j < self->glyphs->size; ++j ) {
                glyph = *(texture_glyph_t **) vector_get( self->glyphs, j );
                // If charcode is -1, we don't care about outline type or thickness
                // if( (glyph->charcode == charcodes[i])) {
                if( (glyph->charcode == charcodes[i]) &&
                    ((charcodes[i] == (wchar_t)(-1) ) ||
                     ((glyph->outline_type == self->outline_type) &&
                      (glyph->outline_thickness == self->outline_thickness)) ))
                {
                    pass = 1;
                    break;
                }
            }

            if(pass)
                continue;

            flags = 0;
            ft_glyph_top = 0;
            ft_glyph_left = 0;
            glyph_index = FT_Get_Char_Index( face, charcodes[i] );
            // WARNING: We use texture-atlas depth to guess if user wants
            //          LCD subpixel rendering

            if( self->outline_type > 0 )
            {
                flags |= FT_LOAD_NO_BITMAP;
            }
            else
            {
                flags |= FT_LOAD_RENDER;
            }

            if( !self->hinting )
            {
                flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
            }
            else
            {
                flags |= FT_LOAD_FORCE_AUTOHINT;
            }


            if( depth == 3 )
            {
                FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT );
                flags |= FT_LOAD_TARGET_LCD;
                if( self->filtering )
                {
                    FT_Library_SetLcdFilterWeights( library, self->lcd_weights );
                }
            }

            error = FT_Load_Glyph( face, glyph_index, flags );
            if( error )
            {
                FT_Done_Face( face );
                FT_Done_FreeType( library );
                throw wcslen(charcodes)-i;
            }


            if( self->outline_type == 0 )
            {
                slot            = face->glyph;
                ft_bitmap       = slot->bitmap;
                ft_glyph_top    = slot->bitmap_top;
                ft_glyph_left   = slot->bitmap_left;
            }
            else
            {
                FT_Stroker stroker;
                FT_BitmapGlyph ft_bitmap_glyph;
                error = FT_Stroker_New( library, &stroker );
                if( error )
                {
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
					throw;
                }
                FT_Stroker_Set(stroker,
                                (int)(self->outline_thickness * HRES),
                                FT_STROKER_LINECAP_ROUND,
                                FT_STROKER_LINEJOIN_ROUND,
                                0);
                error = FT_Get_Glyph( face->glyph, &ft_glyph);
                if( error )
                {
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
					throw;
                }

                if( self->outline_type == 1 )
                {
                    error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
                }
                else if ( self->outline_type == 2 )
                {
                    error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
                }
                else if ( self->outline_type == 3 )
                {
                    error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
                }
                if( error )
                {
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
					throw;
                }

                if( depth == 1)
                {
                    error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                    if( error )
                    {
                        FT_Done_Face( face );
                        FT_Stroker_Done( stroker );
                        FT_Done_FreeType( library );
						throw;
                    }
                }
                else
                {
                    error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
                    if( error )
                    {
                        FT_Done_Face( face );
                        FT_Stroker_Done( stroker );
                        FT_Done_FreeType( library );
						throw;
                    }
                }
                ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
                ft_bitmap       = ft_bitmap_glyph->bitmap;
                ft_glyph_top    = ft_bitmap_glyph->top;
                ft_glyph_left   = ft_bitmap_glyph->left;
                FT_Stroker_Done(stroker);
            }


            // We want each glyph to be separated by at least one black pixel
            // (for example for shader used in demo-subpixel.c)
            w = ft_bitmap.width/depth + 1;
            h = ft_bitmap.rows + 1;
            region = texture_atlas_get_region( self->atlas, w, h );
            if ( region.x < 0 )
            {
                missed++;
                fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
                continue;
            }
            w = w - 1;
            h = h - 1;
            x = region.x;
            y = region.y;
            texture_atlas_set_region( self->atlas, x, y, w, h,
                                      ft_bitmap.buffer, ft_bitmap.pitch );

            glyph = texture_glyph_new( );
            glyph->charcode = charcodes[i];
            glyph->width    = w;
            glyph->height   = h;
            glyph->outline_type = self->outline_type;
            glyph->outline_thickness = self->outline_thickness;
            glyph->offset_x = ft_glyph_left;
            glyph->offset_y = ft_glyph_top;
            glyph->s0       = x/(float)width;
            glyph->t0       = y/(float)height;
            glyph->s1       = (x + glyph->width)/(float)width;
            glyph->t1       = (y + glyph->height)/(float)height;

            // Discard hinting to get advance
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
            slot = face->glyph;
            glyph->advance_x = slot->advance.x / HRESf;
            glyph->advance_y = slot->advance.y / HRESf;

            vector_push_back( self->glyphs, &glyph );

            if( self->outline_type > 0 )
            {
                FT_Done_Glyph( ft_glyph );
            }
        }

        FT_Done_Face( face );
        FT_Done_FreeType( library );
        texture_atlas_upload( self->atlas );
        texture_font_generate_kerning( self );
        return missed;
    }


    // ------------------------------------------------- texture_font_get_glyph ---
    texture_glyph_t *
    texture_font_get_glyph( texture_font_t * self,
                            wchar_t charcode )
    {
        size_t i;
        wchar_t buffer[2] = {0,0};
        texture_glyph_t *glyph;

        assert( self );
        assert( self->filename );
        assert( self->atlas );

        /* Check if charcode has been already loaded */
        for( i=0; i<self->glyphs->size; ++i )
        {
            glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
            // If charcode is -1, we don't care about outline type or thickness
            if( (glyph->charcode == charcode) &&
                ((charcode == (wchar_t)(-1) ) ||
                 ((glyph->outline_type == self->outline_type) &&
                  (glyph->outline_thickness == self->outline_thickness)) ))
            {
                return glyph;
            }
        }

        /* charcode -1 is special : it is used for line drawing (overline,
         * underline, strikethrough) and background.
         */
        if( charcode == (wchar_t)(-1) )
        {
            size_t width  = self->atlas->width;
            size_t height = self->atlas->height;
            ivec4 region = texture_atlas_get_region( self->atlas, 5, 5 );
            texture_glyph_t * glyph = texture_glyph_new( );
            static unsigned char data[4*4*3] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
            if ( region.x < 0 )
            {
                fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
                return NULL;
            }
            texture_atlas_set_region( self->atlas, region.x, region.y, 4, 4, data, 0 );
            glyph->charcode = (wchar_t)(-1);
            glyph->s0 = (region.x+2)/(float)width;
            glyph->t0 = (region.y+2)/(float)height;
            glyph->s1 = (region.x+3)/(float)width;
            glyph->t1 = (region.y+3)/(float)height;
            vector_push_back( self->glyphs, &glyph );
            return glyph; //*(texture_glyph_t **) vector_back( self->glyphs );
        }

        /* Glyph has not been already loaded */
        buffer[0] = charcode;
        if( texture_font_load_glyphs( self, buffer ) == 0 )
        {
            return *(texture_glyph_t **) vector_back( self->glyphs );
        }
        return NULL;
    }

#pragma endregion
}
