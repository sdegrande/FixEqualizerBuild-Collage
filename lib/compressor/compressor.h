
/* Copyright (c) 2009 - 2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#ifndef EQ_PLUGIN_COMPRESSOR
#define EQ_PLUGIN_COMPRESSOR 

#include <eq/plugins/compressor.h>

#include <eq/base/base.h>
#include <eq/base/buffer.h>
#include <typeinfo>
#include <vector>

/**
 * @file compressor/compressor.h
 * 
 * Compression plugin provided with Equalizer.
 */

namespace eq
{
namespace plugin
{
    class Compressor
    {
    public:
        typedef void  (*CompressorGetInfo_t)( EqCompressorInfo* const );
        typedef void* (*NewCompressor_t)( const EqCompressorInfo* );
        typedef void (*Decompress_t)( const void* const*, const 
                                      eq_uint64_t* const,
                                      const unsigned, void* const, 
                                      const eq_uint64_t, const bool );
        typedef bool ( *IsCompatible_t ) ( const GLEWContext* );
        typedef void ( *Download_t )( const GLEWContext*, const eq_uint64_t*,
                                      const unsigned, const eq_uint64_t,
                                      eq_uint64_t*, void** );
        typedef void ( *Upload_t )( const GLEWContext*, const void*,
                                    const eq_uint64_t*,
                                    const eq_uint64_t, const eq_uint64_t*,
                                    const unsigned );

        struct Functions
        {
            Functions();
            CompressorGetInfo_t  getInfo;
            NewCompressor_t      newCompressor;
            Decompress_t         decompress;
            IsCompatible_t       isCompatible;
            Download_t           download;
            Upload_t             upload;
        };

        /** Construct a new compressor. */
        Compressor( const EqCompressorInfo* info );

        virtual ~Compressor();

        /**
         * compress Data.
         *
         * @param inData data to compress.
         * @param nPixels number data to compress.
         * @param useAlpha use alpha channel in compression.
         */
        virtual void compress( const void* const inData,
                               const eq_uint64_t nPixels, 
                               const bool useAlpha ) { EQDONTCALL; };

        typedef eq::base::Bufferb Result;
        typedef std::vector< Result* > ResultVector;

        /** @return the vector containing the result data. */
        const ResultVector& getResults() const { return _results; }

        /** @return the number of result items produced. */
        unsigned getNResults() const { return _nResults; }

        /**
         * Transfer frame buffer data into main memory.
         * 
         * @param glewContext the initialized GLEW context describing
         *                    corresponding to the current OpenGL context.
         * @param inDims the dimensions of the input data (x, w, y, h).
         * @param source texture name to process.
         * @param flags capability flags for the compression (see description).
         * @param outDims the dimensions of the output data (see description).
         * @param out the pointer to the output data.
         */        
        virtual void download( const GLEWContext* glewContext,
                               const eq_uint64_t  inDims[4],
                               const unsigned     source,
                               const eq_uint64_t  flags,
                               eq_uint64_t        outDims[4],
                               void**             out ) { EQDONTCALL; }

        /**
         * Transfer data from main memory into GPU memory.
         *
         * @param glewContext the initialized GLEW context describing
         *                    corresponding to the current OpenGL context.
         * @param buffer the datas input.
         * @param inDims the dimension of data in the frame buffer.
         * @param flags capability flags for the compression.
         * @param outDims the result data size
         * @param destination the destination texture name.
         */
        virtual void upload( const GLEWContext* glewContext, 
                             const void*        buffer,
                             const eq_uint64_t  inDims[4],
                             const eq_uint64_t  flags,
                             const eq_uint64_t  outDims[4],  
                             const unsigned     destination ) { EQDONTCALL; }

    protected:
        ResultVector _results;  //!< The compressed data
        unsigned _nResults;     //!< Number of elements used in _results
    };
}
}

#endif // EQ_PLUGIN_COMPRESSOR

