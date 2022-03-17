# LibNDI
Open-source, cross-platform and maintained.

## Dependencies
* Ffmpeg: libavcodec, libavformat, libavutil
* Bonjour (MacOS and Windows)
* Avahi (Linux)

## Acknowledgements
This library is based on the work by Jean-Baptiste Kempf (https://code.videolan.org/jbk/libndi).

## License
LGPL v2.1

## Building (AA EDIT)

1. Install the following extra system dependencies:
    * libavahi-client-dev
    * More that I will add to this list soon
2. `cmake -S . -B .` in the top directory
3. `cd examples/example-recv-gl`
4. `cmake -S . -B build`
5. `cd build && make`
