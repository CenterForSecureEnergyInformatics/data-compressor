Overview
---

DCLib is a library which allows compressing and decompressing (referred to as encoding and decoding henceforth) data from buffers (see DCIOLib).

`enc_dec.h`: Allows listing and using all implemented encoders/decoders as well as their options. Life cycle: `GetEncoder` -> (optional option configuration, see below) -> `enc_dec_t.encoder` (for encoding) or `enc_dec_t.decoder` (for decoding) call on initialized input and output bit buffers. Optional option configuration: (optional) `OptionNameExists` -> (optional) `EncoderSupportsOption` -> `GetOptionType` -> `GetAllowedOptionValueRange` -> `SetOptionValue<Type>`.

Encoders/decoders
---

* aggregate: Sums of `num_values` (option name) consecutive floating-point values (no decoder!).
* bac: Performs binary arithmetic coding as implemented by Witten et al.
* copy: Copies the input to the output, i.e., it performs no compression whatsoever. This encoder/decoder operates on blocks of `blocksize` (option name) bits size.
* csv: Reads lines of comma-separated values and converts the strings in column number `column` (option name) of each line to a list of (binary) floating-point values when encoding; performs the reverse conversion when decoding and inserts blank columns if necessary.
* diff: Encodes (signed) differences between consecutive (unsigned) values of `valuesize` (option name) bits size when encoding; reconstructs (unsigned) values of `valuesize` (option name) bits size from their consecutive (signed) differences when decoding
* lzmh: Performs LZMH coding and decoding from Ringwelski et al. This is an integrated third-party implementation.
* normalize: Converts floating-point values to (signed) integer values of `valuesize` (option name) bits size when encoding; performs the reverse conversion when decoding. To preserve decimal places after the decimal point, all values are multiplied by `normalization_factor` (option name) when encoding, and divided when decoding.
* seg: Creates Exponential Golomb code words from values when encoding; reconstructs Exponential Golomb code words when decoding. All values are `valuesize` (option name) bits in size and signed.

Supported encoder input and output formats
---

Note: Decoder input and formats are reversed, if there is a decoder).

* aggregate: binary float in, binary float out
* bac: arbitrary in, binary out
* copy: arbitrary in, arbitrary out
* csv: ASCII float in, binary float out
* diff: unsigned int in, signed int out
* lzmh: ASCII float in, binary out
* normalize: float in, signed int out
* seg: signed int in, binary out

Notes on usage
---

* GetEncoderNames requires a `char*` array with `GetNumberOfEncoders` fields.
* When adding or renaming encoders/decoders or options, make sure the arrays remain sorted by name. Otherwise, the find operations will not work as expected.
