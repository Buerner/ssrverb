# SSRverb

This is an attempt to bring reverberation to the [SoundScape Renderer](http://spatialaudio.net/ssr/). It was created within the scope of a masters thesis in [Audio Communication and Technology](http://www.ak.tu-berlin.de/akt/menue/studiengang_audiokommunikation_und_technologie/parameter/en/) at [Technische Universität Berlin](http://www.tu-berlin.de/menue/home/parameter/en/).

The reverberation methods used include:
1. Feedback delay network with multichannel output.
2. Image source model
3. Convolution using spatialized versions of a mono impulse response.

A GUI for a reverberator using a combination of 1. and 2. is available. The incorporated ISM adapts to the position of a selectable source in the SSR. 

Also included is a command line application using the 3. method.

## Dependencies
- [Jack Audio Connection Kit](http://www.jackaudio.org/)
- [sndfile](http://www.mega-nerd.com/libsndfile/)
- [ssrface](https://github.com/Buerner/ssrface) (Included as submodule)
    - [Asio](http://think-async.com/)
    - [RapidXml](http://rapidxml.sourceforge.net/)
- [laproque](https://github.com/Buerner/laproque) (Included as submodule)
    - [FFTW](http://www.fftw.org/)

## Compiling
`make`

--> In case the make process completed without errors, the library files are placed in the **build/libs** directory. Binaries are placed in **build/bins**.

## Documentation
Doxygen documentation for most classes is available. Doxyfiles are included in the rep.

`doxygen`
