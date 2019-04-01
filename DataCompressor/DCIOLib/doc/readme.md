Overview
---

DCIOLib is a library which allows performing bit-wise reading and writing operations on buffers which are linked to either files or memory.

`buffer.h` (`buffer_t`): A buffer implementation for byte-wise reading and writing operations. It can be resized, if necessary, while retaining the old data. Life cycle: `AllocateBuffer` -> `InitBuffer` -> (read, write or other operations) -> `UninitBuffer` -> `FreeBuffer`.

`file_buffer.h` (`file_buffer_t`): A buffer implementation for byte-wise reading and writing operations on files. It wraps a `buffer_t` and can thus also be used to read or write in memory. It is possible to switch between reading and writing. Life cycle: `AllocateFileBuffer` -> `InitFileBuffer` with an opened file or `InitFileBufferInMemory` -> (read, write or other operations) -> `UninitFileBuffer` -> `FreeFileBuffer`.

`bit_file_buffer.h` (`bit_file_buffer_t`): A buffer implementation for bit-wise reading and writing on files or in memory. It provides single-bit and constant-bit-size read/write access. It uses uses a `file_buffer_t` which needs to be initialized and uninitialized separately. It is possible to switch from writing to reading; the opposite way is not supported. Life cycle: `AllocateBitFileBuffer` -> `InitBitFileBuffer` with an initialized `file_buffer_t` instance -> (read, write or other operations) -> `UninitBitFileBuffer` -> `FreeBitFileBuffer`.

Notes on usage
---

* Although `bit_file_buffer_t` cannot be changed from reading mode back to writing mode, it is possible to reset the buffer, which discards buffered data.
* When `file_buffer_t` is used to write to memory, the underlying buffer will be automatically resized when it is too small.
* `file_buffer_t` and `bit_file_buffer_t` flush contents automatically when they are uninitialized. To do so before uninitializing, an explicit flush operation is required.
