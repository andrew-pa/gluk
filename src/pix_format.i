#pragma once
#include "cmmn.h"

#ifdef GA_GLES
#define GL_R16 -1
#define GL_R16_SNORM -1
#define GL_RG16 -1
#define GL_RG16_SNORM -1
#define GL_RGB16 -1
#define GL_RGB16_SNORM -1
#define GL_RGBA16 -1
#define GL_RGBA16_SNORM -1
#define GL_DEPTH_COMPONENT32 -1
#endif


/* Nasty bad code from GLUK, many strange special cases and such (see unorm vs uint)*/


        GLenum pixel_format::get_gl_format_internal() const
        {
            if(depth == 8)
            {
                switch (comp)
                {
                case pixel_components::r:
                    if(type == pixel_type::unorm )    return GL_R8;
                    if(type == pixel_type::snorm )    return GL_R8_SNORM;
                    if(type == pixel_type::floatp )   throw;//return GL_PX_FMT(R, F, 8);
                    if(type == pixel_type::integer )  return GL_R8I;
                    if(type == pixel_type::uinteger ) return GL_R8UI;
                    break;
                case pixel_components::rg:
                    if(type == pixel_type::unorm )    return GL_RG8;
                    if(type == pixel_type::snorm )    return GL_RG8_SNORM;
                    if(type == pixel_type::floatp )   throw;//return GL_PX_FMT(R, F, 8);
                    if(type == pixel_type::integer )  return GL_RG8I;
                    if(type == pixel_type::uinteger ) return GL_RG8UI;
                    break;
                case pixel_components::rgb:
                    if(type == pixel_type::unorm )    return GL_RGB8;
                    if(type == pixel_type::snorm )    return GL_RGB8_SNORM;
                    if(type == pixel_type::floatp )   throw;// return GL_PX_FMT(R, F, 8);
                    if(type == pixel_type::integer )  return GL_RGB8I;
                    if(type == pixel_type::uinteger ) return GL_RGB8UI;
                    break;
                case pixel_components::rgba:
                    if(type == pixel_type::unorm )    return GL_RGBA8;
                    if(type == pixel_type::snorm )    return GL_RGBA8_SNORM;
                    if(type == pixel_type::floatp )   throw;// return GL_PX_FMT(RGBA, F, 8);
                    if(type == pixel_type::integer )  return GL_RGBA8I;
                    if(type == pixel_type::uinteger ) return GL_RGBA8UI;
                    break;
                }
            }
            else if(depth == 16)
            {
                switch (comp)
                {
                case pixel_components::r:
                    if(type == pixel_type::unorm )    return GL_R16;
                    if(type == pixel_type::snorm )    return GL_R16_SNORM;
                    if(type == pixel_type::floatp )   return GL_R16F;
                    if(type == pixel_type::integer )  return GL_R16I;
                    if(type == pixel_type::uinteger ) return GL_R16UI;
                    break;
                case pixel_components::rg:
                    if(type == pixel_type::unorm )    return GL_RG16;
                    if(type == pixel_type::snorm )    return GL_RG16_SNORM;
                    if(type == pixel_type::floatp )   return GL_R16F;
                    if(type == pixel_type::integer )  return GL_RG16I;
                    if(type == pixel_type::uinteger ) return GL_RG16UI;
                    break;
                case pixel_components::rgb:
                    if(type == pixel_type::unorm )    return GL_RGB16;
                    if(type == pixel_type::snorm )    return GL_RGB16_SNORM;
                    if(type == pixel_type::floatp )   return GL_R16F;
                    if(type == pixel_type::integer )  return GL_RGB16I;
                    if(type == pixel_type::uinteger ) return GL_RGB16UI;
                    break;
                case pixel_components::rgba:
                    if(type == pixel_type::unorm )    return GL_RGBA16;
                    if(type == pixel_type::snorm )    return GL_RGBA16_SNORM;
                    if(type == pixel_type::floatp )   return GL_RGBA16F;
                    if(type == pixel_type::integer )  return GL_RGBA16I;
                    if(type == pixel_type::uinteger ) return GL_RGBA16UI;
                    break;
                case pixel_components::depth:
                    if (type != pixel_type::unorm) throw;
                    return GL_DEPTH_COMPONENT16;
                }
            }
            else if(depth == 32)
            {
                switch (comp)
                {
                case pixel_components::r:
                    if(type == pixel_type::unorm )    throw;// return GL_PX_FMT(R, , 32);
                    if(type == pixel_type::snorm )    throw;// return GL_PX_FMT(R, _SNORM, 32);
                    if(type == pixel_type::floatp )   return GL_R32F;
                    if(type == pixel_type::integer )  return GL_R32I;
                    if(type == pixel_type::uinteger ) return GL_R32UI;
                    break;
                case pixel_components::rg:
                    if(type == pixel_type::unorm )    throw;// return GL_PX_FMT(RG, , 32);
                    if(type == pixel_type::snorm )    throw;// return GL_PX_FMT(RG, _SNORM, 32);
                    if(type == pixel_type::floatp )   return GL_R32F;
                    if(type == pixel_type::integer )  return GL_RG32I;
                    if(type == pixel_type::uinteger ) return GL_RG32UI;
                    break;
                case pixel_components::rgb:
                    if(type == pixel_type::unorm )    throw;// return GL_PX_FMT(RGB, , 32);
                    if(type == pixel_type::snorm )    throw;// return GL_PX_FMT(RGB, _SNORM, 32);
                    if(type == pixel_type::floatp )   return GL_R32F;
                    if(type == pixel_type::integer )  return GL_RGB32I;
                    if(type == pixel_type::uinteger ) return GL_RGB32UI;
                    break;
                case pixel_components::rgba:
                    if(type == pixel_type::unorm )    throw;// return GL_PX_FMT(RGBA, , 32);
                    if(type == pixel_type::snorm )    throw;// return GL_PX_FMT(RGBA, _SNORM, 32);
                    if(type == pixel_type::floatp )   return GL_RGBA32F;
                    if(type == pixel_type::integer )  return GL_RGBA32I;
                    if(type == pixel_type::uinteger ) return GL_RGBA32UI;
                    break;
                case pixel_components::depth:
                    if (type == pixel_type::unorm) return GL_DEPTH_COMPONENT32;
                    else if (type == pixel_type::floatp) return GL_DEPTH_COMPONENT32F;
                    else throw;
                    break;
                case pixel_components::depth_stencil:
                    if (type == pixel_type::floatp)
                        return GL_DEPTH32F_STENCIL8;
                    else throw;
                    break;
                }
            }
            else if(depth == 24)
            {
                if (comp == pixel_components::depth)
                {
                    if(type == pixel_type::unorm)
                        return GL_DEPTH_COMPONENT24;
                    else throw;
                }
                else if(comp == pixel_components::depth_stencil)
                {
                    if (type == pixel_type::unorm)
                        return GL_DEPTH24_STENCIL8;
                    else throw;
                }
                else throw;
            }
            else throw;
            throw;
        }
        GLenum pixel_format::get_gl_type() const
        {
            if(type == pixel_type::floatp)
            {
                if (comp == pixel_components::depth_stencil)
                    return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                if (depth == 16) return GL_HALF_FLOAT;
                else return GL_FLOAT;
            }
            else if(type == pixel_type::snorm)
            {
                if (depth == 16) return GL_SHORT;
                else if (depth == 8) return GL_BYTE;
                else return GL_INT;
            }
            else if (type == pixel_type::unorm)
            {
                if (depth == 24 && comp == pixel_components::depth_stencil)
                    return GL_UNSIGNED_INT_24_8;
                if (depth == 16) return GL_UNSIGNED_SHORT;
                else if (depth == 8) return GL_UNSIGNED_BYTE;
                else return GL_UNSIGNED_INT;
            }
            else if(type == pixel_type::integer)
            {
                if (depth == 16) return GL_SHORT;
                else return GL_INT;
            }
            else if(type == pixel_type::uinteger)
            {
                if (depth == 16) return GL_UNSIGNED_SHORT;
                else return GL_UNSIGNED_INT;
            }
            return GL_INVALID_ENUM;
        }
        GLenum pixel_format::get_gl_format() const
        {
            if(type == pixel_type::integer || type == pixel_type::uinteger)
            {
                switch (comp)
                {
                case pixel_components::r:
                    return GL_RED_INTEGER;
                case pixel_components::rg:
                    return GL_RG_INTEGER;
                case pixel_components::rgb:
                    return GL_RGB_INTEGER;
                case pixel_components::rgba:
                    return GL_RGBA_INTEGER;
                }
            }
            else
            {
                switch (comp)
                {
                case pixel_components::r:
                    return GL_RED;
                case pixel_components::rg:
                    return GL_RG;
                case pixel_components::rgb:
                    return GL_RGB;
                case pixel_components::rgba:
                    return GL_RGBA;
                case pixel_components::depth:
                    return GL_DEPTH_COMPONENT;
                case pixel_components::depth_stencil:
                    return GL_DEPTH_STENCIL;
                }
            }
            throw;
        }
