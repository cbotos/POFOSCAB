# Dev Guide

## Adding a new command

1. Pick any free command code in `0x80`-`0xFF` not yet used in
   `protocol.md`. Stay in this range: `0x00`-`0x06` is the ROM's own
   dispatch table, and `0x07`-`0x7F` is unverified safe - only `0x80`+
   is confirmed collision-free with the ROM.
2. Decide whether it belongs to an existing capability group or needs
   a new bit for a new command family (see `hello.inc`'s header) -
   don't add a new bit for just another file operation.
3. Document the wire format in `protocol.md` first - request/response
   byte layout only, no implementation details.
4. Implement `dispatch_<name>` in a new or existing `*.inc` file, wire
   it into `PFTD.asm`'s command detection (`%include` plus the
   `mov al, [cs:payload0]` / `call dispatch_<name>` pair in
   `pftd_int61_handler`).
5. Verify on real hardware before trusting any RBIL-documented DOS
   function contract - DIP DOS diverges from PC MS-DOS behavior.
6. Add a pytest case in `tests/test_pftd.py` covering the happy path.
