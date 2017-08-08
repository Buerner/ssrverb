//
//  main.cpp
//  Converb
//
//  Created by Martin Bürner on 08.12.16.
//  Copyright © 2016 Martin Bürner. All rights reserved.
//

#include "JackRandomizer.hpp"
#include <iostream>

int main( int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf( "Usage: %s <PATH TO MONO IR.>\n", argv[0] );
        return 1;
    }

    //const char* file_path = &argv[1];
    SSRverb::JackRandomizer reverb( argv[1] );

    reverb.set_ssr_address("127.0.0.1", 4711);
    reverb.connect();
    reverb.clear_scene();
    reverb.setup_rev_sources();

    reverb.activate();

    printf("Hit ENTER to close. ");
    getchar();

    return 0;
}
