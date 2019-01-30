
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

section .rodata

; Kernel Boot Log Font
global font
font:
    incbin "src/font/vgafont.bin"
