Overview
---

DCCLI is a command line application which allows compressing and decompressing (referred to as encoding and decoding henceforth) files using DCLib.

Usage: `<input file> <output file> <list of encoders/decoders with options>`

The list of encoders/decoders is separated by a separate #. Each encoder/decoder must specify either `encode` or `decode`, followed by the encoder/decoder name. Options can be specified separately after that. They affect only encoder/decoder that precedes them in the command line. Options are specified as `<name>=<value>` or `<name>` for boolean options.

Example: `input.dat output.dat encode copy # decode copy blocksize=8`

Notes on usage
---

* If a fractional number of bytes (i.e., a number of bits not divisible by eight) is written to the output file, decoding said output file later may lead to errors at the last byte when processing the superfluous bits at the end of the file.
* When using only one encoder/decoder, data read from the input file is processed and written directly (buffered) to the output file, requiring no additional memory. If, however, multiple encoders/decoders are used, data read from the input file is processed and written to a temporary buffer. For all but the last encoder/decoder, data is read from this temporary buffer, processed and written to another temporary buffer. For the last encoder/decoder, data from this temporary buffer is read, processed and written to the output file. Since all data is processed by one encoder/decoder after another, all intermediate data will be held in the described temporary buffers. Processing large files can therefore lead to high memory consumption.
* The size of the temporary buffers described above may be reduced at compile-time via `TEMP_BUFFER_SIZE`. However, since the buffers resize themselves automatically, `TEMP_BUFFER_SIZE` is only their initial size, which is no indicator of the acutal memory consumption when processing larger files with more than one encoder/decoder
* The size of the input and output file buffers may be reduced at compile-time via `READ_BUFFER_SIZE` and `WRITE_BUFFER_SIZE`. Both are guaranteed to remain unchanged throughout the execution of the program.
