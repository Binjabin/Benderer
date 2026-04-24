//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_BENDERER_IMAGE_H
#define BENDERER_BENDERER_IMAGE_H

//
// Created by binjabin on 7/8/25.
//

#ifndef BENDERER_STB_IMAGE_H
#define BENDERER_STB_IMAGE_H

//disable STRICT warnings
#ifdef _MSC_VER
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FALIURE_USERMSG
#include "../external/stb_image.h"

#include <cstdlib>
#include <iostream>

class benderer_image {
public:
    benderer_image() {
    }

    benderer_image( const char* image_filename ) {
        auto filename = std::string( image_filename );
        auto imagedir = getenv( "BENDERER_IMAGES" );

        // Hunt for the image file in some likely locations.
        if ( imagedir && load( std::string( imagedir ) + "/" + image_filename ) ) return;
        if ( load( filename ) ) return;
        if ( load( "Textures/" + filename ) ) return;
        if ( load( "../Textures/" + filename ) ) return;
        if ( load( "../../Textures/" + filename ) ) return;
        if ( load( "../../../Textures/" + filename ) ) return;
        if ( load( "../../../../Textures/" + filename ) ) return;
        if ( load( "../../../../../Textures/" + filename ) ) return;
        if ( load( "../../../../../../Textures/" + filename ) ) return;

        std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
    }

    ~benderer_image() {
        delete[] bdata;
        STBI_FREE( fdata );
    }

    bool load( const std::string& filename ) {
        auto n = bytes_per_pixel;
        fdata = stbi_loadf( filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel );
        if ( fdata == nullptr ) return false;

        bytes_per_scanline = image_width * bytes_per_pixel;
        convert_to_bytes();
        return true;
    }

    int width() const { return ( fdata == nullptr ) ? 0 : image_width; }
    int height() const { return ( fdata == nullptr ) ? 0 : image_height; }

    const unsigned char* pixel_data( int x, int y ) const {
        //no image -> return magenta
        static unsigned char magenta[] = {255, 0, 255};
        if ( bdata == nullptr ) return magenta;

        x = clamp( x, 0, image_width );
        y = clamp( y, 0, image_height );

        return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
    }

private:
    const int bytes_per_pixel = 3;
    float* fdata = nullptr;
    unsigned char* bdata = nullptr;
    int image_width = 0;
    int image_height = 0;
    int bytes_per_scanline = 0;

    static int clamp( int x, int low, int high ) {
        if ( x < low ) return low;
        if ( x < high ) return x;
        return high - 1;
    }

    static unsigned char float_to_byte( float value ) {
        if ( value <= 0.0 ) {
            return 0;
        }
        else if ( 1.0 <= value ) {
            return 255;
        }
        return static_cast<unsigned char>(256.0 * value);
    }

    void convert_to_bytes() {
        int total_bytes = image_width * image_height * bytes_per_pixel;
        bdata = new unsigned char[total_bytes];

        auto* bptr = bdata;
        auto* fptr = fdata;
        for ( auto i = 0; i < total_bytes; i++, fptr++, bptr++ ) {
            *bptr = float_to_byte( *fptr );
        }
    }
};

// Restore MSVC compiler warnings
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#endif //BENDERER_STB_IMAGE_H

#endif //BENDERER_BENDERER_IMAGE_H