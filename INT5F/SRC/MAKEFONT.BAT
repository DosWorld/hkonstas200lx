REM Batch file to create FONTS.ASM

makedb small.hfn >small.asm
makedb medium.hfn >medium.asm
makedb large.hfn >large.asm

copy small.asm + medium.asm + large.asm fonts.asm

ECHO Don't forget to edit FONTS.ASM and insert the font labels!

