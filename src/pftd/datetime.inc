; datetime.inc - GETDATETIME (0x8D) and SETDATETIME (0x8E) commands:
; read/write the Portfolio's system date and time (int 0x21
; AH=0x2A/0x2C - Get Date/Get Time, and AH=0x2B/0x2D - Set Date/Set
; Time).
;
; Command bytes 0x8D/0x8E
;
; Date/time wire format: packed DOS format
; dta_buf+0x18/+0x16 fields, passed through from Find First's DTA
; unmodified):
;   date (2 bytes): bits 15-9 = year-1980, bits 8-5 = month, bits 4-0 = day
;   time (2 bytes): bits 15-11 = hour, bits 10-5 = minute, bits 4-0 = second/2
;
; GETDATETIME (0x8D): request is a single byte.
; Response (fixed 4 bytes):
; packed date(2B) + packed time(2B).
;
; SETDATETIME (0x8E): request is 0x8E, 0x00, 0x70 + packed date(2B)
; + packed time(2B) - a fixed-size binary payload
; Response (fixed 2 bytes):
;   offset 0  status  (0x20 = ok, 0x10 = error)
;   offset 1  errcode (0 on success; 4 = access denied/invalid - AH=0x2B
;                       or AH=0x2D returned AL=0xFF, the only documented
;                       failure mode for either call, i.e. an
;                       out-of-range date or time value; 0xFF = critical
;                       error fired - see below)
;

GETDATETIME_CMD equ 0x8D
SETDATETIME_CMD equ 0x8E

; dispatch_getdatetime: called from pftd_int61_handler with AL =
; payload[0] and CS=DS. No extra payload needed (same as
; dispatch_hello/dispatch_drives). Preserves AX/CX/DX/DS - see
; dispatch_hello's header comment for why.
dispatch_getdatetime:
        cmp     al, GETDATETIME_CMD
        jne     .not_getdatetime

        push    ax
        push    bx
        push    cx
        push    dx
        push    ds

        push    cs
        pop     ds

        ; Get Date: CX=year, DH=month, DL=day
        mov     ah, 0x2A
        int     0x21

        ; Pack date: year<<9 | month<<5 | day
        sub     cx, 1980
        mov     ax, cx
        mov     cl, 9
        shl     ax, cl
        mov     bx, ax

        xor     ah, ah
        mov     al, dh
        mov     cl, 5
        shl     ax, cl
        or      bx, ax

        xor     dh, dh
        or      bl, dl
        mov     [datetime_response], bx

        ; Get Time: CH=hour, CL=minute, DH=second
        mov     ah, 0x2C
        int     0x21

        ; Pack time: hour<<11 | minute<<5 | second/2
        mov     bl, cl                  ; save minutes
        mov     bh, dh                  ; save seconds

        xor     ah, ah
        mov     al, ch
        mov     cl, 11
        shl     ax, cl                  ; hour << 11
        xchg    ax, bx                  ; BX=hour shifted, AX=saved values

        ; AX now contains seconds in AH, minutes in AL
        mov     ch, ah                  ; save seconds
        xor     ah, ah
        mov     cl, 5
        shl     ax, cl                  ; minute << 5
        or      bx, ax

        xor     ah, ah
        mov     al, ch
        shr     ax, 1                   ; second / 2
        or      bx, ax
        mov     [datetime_response+2], bx

        mov     dx, datetime_response
        mov     cx, 4
        mov     ax, 0x3000
        int     0x61

        pop     ds
        pop     dx
        pop     cx
        pop     bx
        pop     ax
.not_getdatetime:
        ret

; dispatch_setdatetime: called from pftd_int61_handler with AL =
; payload[0] and CS=DS. Needs 4 bytes (packed date+time) out of the
; foreign receive buffer, via list_src_ds/list_src_si (set generically
; by pftd_int61_handler - see list.inc's dispatch_list header comment)
; - a fixed-size binary copy, not an ASCIIZ scan, since there's no
; terminator to look for.
dispatch_setdatetime:
        cmp     al, SETDATETIME_CMD
        jne     .not_setdatetime

        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    ds

        mov     ax, [cs:list_src_ds]
        mov     ds, ax
        mov     si, [cs:list_src_si]
        add     si, 3                   ; skip the 3-byte request header
        mov     ax, [si]                ; packed date
        mov     bx, [si+2]              ; packed time

        push    cs
        pop     ds                      ; DS=CS from here on - nothing
                                         ; below this line ever uses the
                                         ; foreign segment again

        ; Stash packed time to memory - every register gets reused by
        ; the date-unpacking arithmetic below, same "more live values
        ; than 8086 has registers for" situation list.inc's header
        ; documents for its own free-space scratch cells.
        mov     [setdatetime_packed_time], bx

        ; Unpack date (ax = packed date) into CX=year, DH=month, DL=day
        ; for AH=0x2B. Packed layout: bits 15-9=year-1980, 8-5=month,
        ; 4-0=day.
        mov     dl, al
        and     dl, 0x1F        ; day

        mov     bx, ax
        mov     cl, 5
        shr     bx, cl
        mov     dh, bl
        and     dh, 0x0F        ; month

        mov     cl, 4
        shr     bx, cl
        add     bx, 1980
        mov     cx, bx          ; year

        mov     byte [cs:critical_error_flag], 0
        mov     ah, 0x2B                ; Set Date
        int     0x21
        cmp     byte [cs:critical_error_flag], 0
        jne     .critical_fired
        cmp     al, 0xFF
        je      .invalid

        ; Unpack time (stashed earlier) into CH=hour, CL=minute,
        ; DH=second, DL=0 for AH=0x2D. Packed layout: bits
        ; 15-11=hour, 10-5=minute, 4-0=second/2.
        mov     ax, [setdatetime_packed_time]

        mov     dh, al
        and     dh, 0x1F
        add     dh, dh          ; seconds

        mov     cl, 5
        shr     ax, cl
        mov     bl, al
        and     bl, 0x3F        ; minutes

        mov     cl, 6
        shr     ax, cl
        mov     ch, al          ; hours
        mov     cl, bl          ; minutes

        xor     dl, dl          ; hundredths

        mov     byte [cs:critical_error_flag], 0
        mov     ah, 0x2D                ; Set Time
        int     0x21
        cmp     byte [cs:critical_error_flag], 0
        jne     .critical_fired
        cmp     al, 0xFF
        je      .invalid

        ; success
        mov     byte [datetime_response], 0x20
        mov     byte [datetime_response+1], 0
        jmp     .respond

.critical_fired:
        mov     byte [datetime_response], 0x10
        mov     byte [datetime_response+1], 0xFF
        jmp     .respond

.invalid:
        mov     byte [datetime_response], 0x10
        mov     byte [datetime_response+1], 4

.respond:
        mov     dx, datetime_response
        mov     cx, 2
        mov     ax, 0x3000              ; AH=0x30 AL=0 = transmit block
        int     0x61

        pop     ds
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
.not_setdatetime:
        ret

section .bss

; Shared by both dispatchers - GETDATETIME uses all 4 bytes as packed
; date(2B)+time(2B); SETDATETIME only ever writes/reads the first 2
; bytes as status(1B)+errcode(1B). Safe to share: the two dispatchers
; never run in the same int 0x61 hook invocation (payload[0] can only
; match one of GETDATETIME_CMD/SETDATETIME_CMD at a time).
datetime_response: resb 4

; dispatch_setdatetime's packed time, stashed while bx/cx/dx are reused
; for date unpacking - see that routine's comment.
setdatetime_packed_time: resw 1

section .text
