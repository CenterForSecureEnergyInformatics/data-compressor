Data Compressor - Framework for smart meter compression algorithms
==================================================================

This software implements a number of lossless smart meter data compression algorithms and can also be used to evaluate them. If you use this software, please cite

[1] Andreas Unterweger and Dominik Engel. Resumable Load Data Compression in Smart Grids. IEEE Transactions on Smart Grid, 6(2):919-929, March 2015.

[2] Andreas Unterweger, Dominik Engel, and Martin Ringwelski. The Effect of Data Granularity on Load Data Compression. In Energy Informatics 2015 - 4th D-A-CH Conference, EI 2015, volume 9424 of Lecture Notes in Computer Science, pages 69-80. Springer International Publishing, Switzerland, November 2015.

These papers are also available as BibTeX entries in the [papers.bib file](papers.bib) for convenience.

This software comes with no warranty whatsoever. You may use it without charge, as long as the original copyright notices remain and the papers listed above are cited. See the [LICENSE file](LICENSE) for details.

How to build the software
=========================

On Windows, start Visual Studio 2013 and open [DataCompressor/build/MSVC/DataCompressor.sln](DataCompressor/build/MSVC/DataCompressor.sln). Build the solution, i.e., all projects. Other versions of Visual Studio may work fine, but are currently not supported explicitly.

On Linux, make sure that GNU Make 3.81 and gcc 4.8.4 are installed. Navigate to [DataCompressor/build/gcc](DataCompressor/build/gcc) and type `make`. Other versions of GNU Make and gcc may work fine, but are currently not supported explicitly.

This software has been tested on Intel x86-64 and ARM v7 precessors (Raspberry Pi 2). On Windows, both 32-bit and 64-bit versions of the software work out of the box. On Linux, gcc-specific parameters can be added to [DataCompressor/common/build/gcc/common.mak](DataCompressor/common/build/gcc/common.mak) (e.g., `COMMON_CFLAGS`) to build for a different "bitness" than the compiler/target default.

How to use the software
=======================

The front-end of the software is DCCLI, a command line interface for Data Compressor, which resides in the folder named DCCLI. There is a short description of the software in [DataCompressor/DCCLI/doc/overview.txt](DataCompressor/DCCLI/doc/overview.txt). Apart from that, the software itself outputs notes on usage when called without arguments or with incorrect ones.

Always use the release version of the software when evaluating algorithms. On Linux, you can use `make test` in either [DataCompressor/build/gcc/](DataCompressor/build/gcc/) or [DataCompressor/DCCLI/build/gcc/](DataCompressor/DCCLI/build/gcc/) to compress the supplied test file with the DEGA algorithm [1] and decompress it again for verification.

Here are some example calls for the evaluation from [2] for the MIT REDD data set, where each channel is first pre-processed using

    ./DCCLI "$c" "$temp_ref" decode csv separator_char=' ' column=2 # encode csv

where `$c` is the input channel file name and `$temp_ref` is the output file name of the pre-processed file. To process this file to a compressed output file, `$temp_out`, proceed as follows:

For DEGA coding, use `./DCCLI $temp_ref $temp_out decode csv # encode normalize # encode diff # encode seg # encode bac adaptive`

For LZMH coding, use `./DCCLI $temp_ref $temp_out encode lzmh`

For A-XDR coding, use `./DCCLI $temp_ref $temp_out decode csv # encode normalize`

For combined compression and decompression, use, e.g., for DEGA: `./DCCLI $temp_ref $temp_out decode csv # encode normalize # encode diff # encode seg # encode bac adaptive # decode bac adaptive # decode seg # decode diff # decode normalize # encode csv`


How to use the software on less powerful hardware
=================================================

The software allows specifying small(er) bit sizes for I/O and processing.

For I/O, the option `IO_SIZE_BITS` documented in [DataCompressor/common/doc/overview.txt](DataCompressor/common/doc/overview.txt) can be set to reduce the I/O bit size. On Linux, you can set `IO_SIZE_BITS` in [DataCompressor/common/build/gcc/](DataCompressor/common/build/gcc/) to a corresponding value for convenience. In the debug version of the software, the number of usable bits for I/O is printed on application startup.

For processing, most encoders/decoders have parameters like the block size or the value size (in bits). The parameters of each encoder are described in [DataCompressor/DCLib/doc/overview.txt](DataCompressor/DCLib/doc/overview.txt). Other parameters like memory buffer sizes are documented in [DataCompressor/DCCLI/doc/overview.txt](DataCompressor/DCCLI/doc/overview.txt).


How to modify the software
==========================

The software is split into separate projects with an according folder structure. Each project has a short description in `$project_name/doc/overview.txt`. `$projectname` DCCLI is the Data Compressor command line interface for the data compression library, DCLib, which uses the I/O library DCIOLib. The commonly shared code can be found in the folder [DataCompressor/common/](DataCompressor/common/).

When modifying the software, make sure to build the debug versions of all projects. In Visual Studio, this can be done by choosing the Debug configuration and rebuilding the solution. With GNU make, `make debug` builds a debug version.

Note that, by default, debug and release versions cannot be built simultaneously at the moment using the Makefiles. When switching between debug and release versions, make sure to call `make clean` in between. If you wish to build some of the projects in their debug version, it is possible to use the Makfiles in `$project_name/build/gcc` individually with `make debug` (or `make`).

If you modify the software, please keep it mostly C90-compliant, as assured by the Makefile options in [DataCompressor/common/build/gcc/](DataCompressor/common/build/gcc/). This is done for compatibility reasons (older Visual verisons and architectures with compilers that do not support C99).
