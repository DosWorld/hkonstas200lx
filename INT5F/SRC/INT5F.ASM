;*******************************************************************
;*                  INT5fh LOW-LEVEL GRAPHICS DRIVER               *
;*                              Vr.1.01                            *
;*                          Harry Konstas                          *
;*                        August 8, 1996                           *
;*                Assembler used: Eric Isaacson's A86              *
;*******************************************************************

cseg    segment byte
        assume  cs:cseg,ds:cseg,es:cseg,ss:cseg


        org     100h                    ; .COM driver

hook    equ     05fh                    ; Interrupt hook

VGAmode equ     11h                     ; VGA mode
CGAeven equ     0b800h                  ; CGA even field address
CGAodd  equ     0ba00h                  ; CGA odd field address
VGAadd  equ     0a000h                  ; VGA screen address

;Replacement rule equates:
FORCE_RULE      equ     0
AND_RULE        equ     1
OR_RULE         equ     2
XOR_RULE        equ     3
INV_FORCE       equ     4
INV_AND         equ     5
INV_OR          equ     6
INV_XOR         equ     7
TXT_RULE        equ     8               ; same as FORCE_RULE


start:  jmp     install

int5f:  cli
        jmp     inthandle

old_int_vector  label   dword
old_int_off     dw      ?
old_int_seg     dw      ?

ginfo           label   byte    ; graphics info table
CurMode         db      ?
DefMode         db      ?
DispWidth       dw      640
DispHeight      dw      200
PenX            dw      0
PenY            dw      0
LineStyle       dw      0ffffh
RepRule         db      8
PenColor        dw      1
ClipMinX        dw      0
ClipMinY        dw      0
ClipMaxX        dw      639
ClipMaxY        dw      199
OriginX         dw      0
OriginY         dw      0
FillMask        db      0ffh,0ffh,0ffh,0ffh,0ffh,0ffh,0ffh,0ffh
FontSeg         dw      ?
FontOff         dw      ?
FontHeight      dw      8
FontSpacing     db      8
FontWidth       db      8

; GETIMG/PUTIMG variables

ImgRule dw      ?
SrcX    dw      ?
SrcY    dw      ?
DstX    dw      ?
DstY    dw      ?
StartX  dw      ?
StartY  dw      ?
LimitX  dw      ?
LimitY  dw      ?
ImgW    dw      ?
ImgH    dw      ?
Xbytes  db      ?

; Other variables

RevVideo        db      0
ForceVGA        db      0

; putimg variables

planes  dw ?
pos1    dw ?
pos2    dw ?
mask1   db ?
mask2   db ?
vidseg  dw ?
shift   db ?
bpl     dw ?   ; bytes per line in source
bpl1    dw ?   ; bytes per line on screen


; Line plotting variables

x1              dw      ?
x2              dw      ?
y1              dw      ?
y2              dw      ?
Deltax          dw      ?
Deltay          dw      ?
cnt             dw      ?
acc             dw      ?
dirx            dw      ?
diry            dw      ?
LineRot         dw      ?
DirVal          dw      ?
FillCount       dw      ?


RomFonts        label   byte    ; FONTS in HFN format
include "fonts.asm"

;include "debug.8"               ; debugging utilities

;       *******************
;       * INT5Fh SERVICES *
;       *******************

inthandle:
        push    ds

        push    cs
        pop     ds

        push    bx
        push    cx
        push    si
        push    di

        cmp     ah,0
        jne     i1
        call    SetVideo        ; Set Video mode
        jmp     exirq

i1:     cmp     ah,1
        jne     i2
        call    SetFill         ; Set Fill pattern
        jmp     exirq

i2:     cmp     ah,2
        jne     i3
        call    GetGinfo        ; get graphics info
        jmp     exirq

i3:     cmp     ah,3
        jne     i4
        call    SetOrigin       ; set logical origin
        jmp     exirq

i4:     cmp     ah,4
        jne     i5
        call    SetClip         ; set clip region
        jmp     exirq

i5:     cmp     ah,5
        jne     i6
        call    Rectangle       ; Draw rectangle
        jmp     exirq

i6:     cmp     ah,6
        jne     i7
        call    Line            ; Draw line
        jmp     exirq

i7:     cmp     ah,7
        jne     i8
        call    PlotDot         ; Plot point
        jmp     exirq

i8:     cmp     ah,8
        jne     i9
        call    MovePen         ; Move pen to new coordinates
        jmp     exirq

i9:     cmp     ah,9
        jne     i10
        call    SetColor        ; Set pen color
        jmp     exirq

i10:    cmp     ah,0ah
        jne     i11
        call    SetRule         ; Set replacement rule
        jmp     exirq

i11:    cmp     ah,0bh
        jne     i12
        call    SetLine         ; Set line type
        jmp     exirq

i12:    cmp     ah,0ch
        jne     i13
        call    ReadDot         ; read point
        jmp     exirq

i13:    cmp     ah,0dh
        jne     i14
        call    GetImage        ; Get image block
        jmp     exirq

i14:    cmp     ah,0eh
        jne     i15
        call    PutImage        ; Put image block
        jmp     exirq

i15:    cmp     ah,0fh
        jne     i16
        call    WriteText       ; Write text string
        jmp     exirq

i16:    cmp     ah,10h
        jne     i17
        call    GetFont         ; Get font size
        jmp     exirq

i17:    cmp     ah,11h
        jne     i18
        call    SetFont         ; Set font
        jmp     exirq

i18:    cmp     ah,12h
        jne     i19
        call    Scroll          ; Scroll image
        jmp     exirq

i19:    cmp     ah,13h
        jne     i20
        call    SetGinfo        ; set graphics info
        jmp     exirq

i20:    cmp     ah,14h
        jne     exirq
        call    SetDefaults     ; set graphics defaults

exirq:  pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ds
        sti
        iret


;       *************************
;       * AH=0h: SET VIDEO MODE *   AL has video mode
;       *************************

SetVideo:
        push    ax
        mov     ah,0fh
        int     10h
        mov     DefMode,al      ; save previous mode
        pop     ax

        cmp     al,6            ; CGA mode?
        jne     svj3
        cmp     ForceVGA,1
        jne     svj3
        mov     al,VGAmode      ; command-line force to VGA

svj3:   cmp     al,20h          ; 95LX mode? (not supported)
        jne     svj1
        mov     al,6            ; 640x200 2-color
svj1:   mov     CurMode,al
        mov     ah,0
        int     10h
        call    SetDefaults     ; default settings

        mov     al,CurMode
        cmp     al,VGAmode
        jne     svj2
        mov     ClipMaxY,479    ; VGA mode
        mov     DispHeight,480
svj2:   ret



;       ************************
;       * AH=1h: SET FILL MASK * ES:DI= fill mask
;       ************************

SetFill:
        push    cx
        push    si
        push    di
        mov     cx,8
        lea     si,FillMask
sfl:    mov     al,es:[di]
        mov     cs:[si],al
        inc     di
        inc     si
        loop    sfl
        pop     di
        pop     si
        pop     cx
        ret


;       ****************************
;       * AH=2h: GET GRAPHICS INFO * ES:DI= 44-byte buffer
;       ****************************

GetGinfo:
        push    cx
        push    si
        cld
        mov     cx,44           ; transfer 44 bytes
        lea     si,ginfo
gil1:   lodsb                   ; DS segment
        stosb                   ; ES segment
        loop    gil1
        pop     si
        pop     cx
        ret


;       *****************************
;       * AH=3h: SET LOGICAL ORIGIN * CX=x DX=y
;       *****************************

SetOrigin:
        mov     OriginX,cx
        mov     OriginY,dx
        ret


;       **************************
;       * AH=4h: SET CLIP REGION * CX,DX=min x,y SI,DI=max x,y
;       **************************

SetClip:
        mov     ClipMinX,cx
        mov     ClipMinY,dx
        mov     ClipMaxX,si
        mov     ClipMaxY,di
        ret


;       *************************
;       * AH=5h: DRAW RECTANGLE * AL=fill flag CX=x DX=y (destination)
;       *************************

rctpln  DW -1
rctbits DW  1
rctwid  DW  ?
rcthei  DW  ?

Rectangle:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        cmp     al,0            ; outline only
        je      rcj1
        cmp     al,1            ; solid fill
        je      rcj2
        cmp     al,2            ; pattern fill
        je      rcj3

;------ OUTLINE ---------

rcj1:   mov     si,PenX
        mov     di,PenY
        mov     bx,dx
        mov     ax,cx

        mov     dx,di
        call    Line
        mov     dx,bx
        call    Line
        mov     cx,si
        call    Line
        mov     dx,di
        call    Line
        mov     PenX,ax
        mov     PenY,bx
        jmp     rcj4

;------ SOLID FILL -------

rcj2:
        sub     cx, PenX
        mov     rctwid, cx
        sub     dx, PenY
        mov     rcthei, dx
        mov     cx, PenX
        mov     dx, PenY
        push    ds
        pop     es
        mov     di, offset rctpln
        mov     al, RepRule
        cmp     PenColor, 0
        jne     rct1
        or      al, 4
rct1:   call    PutImage
        jmp     rcj4


;------ PATTERN FILL --------

rcj3:
        mov     ax,LineStyle
        push    ax                 ; save current line style

        mov     al,FillMask        ; use only 8 bits
        mov     ah,FillMask
        mov     LineStyle,ax

        mov     DirVal,0ffffh
        cmp     dx,PenY            ; upward fill?
        jb      rcj6
        mov     DirVal,1

rcj6:   mov     si,cx
        mov     di,dx
        mov     ax,PenX
        mov     dx,PenY
        push    dx                 ; save PenY

rcl2:   mov     PenX,ax
        mov     PenY,dx
        mov     cx,si
        call    Line
        inc     FillCount
        and     FillCount,7

        push    si
        lea     si,FillMask
        add     si,FillCount       ; calculate offset
        mov     bl,[si]            ; get pattern byte
        mov     bh,bl
        mov     LineStyle,bx
        pop     si

        cmp     dx,di
        je      rcj8
        add     dx,DirVal          ; next H-line (up/dn)
        jmp     rcl2

rcj8:   mov     PenX,ax
        pop     dx
        mov     PenY,dx
        pop     ax
        mov     LineStyle,ax
        jmp     rcj4

rcj4:   pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret


;       ********************
;       * AH=6h: DRAW LINE * CX=x DX=y (destination)
;       ********************

Line:   push    ax
        push    dx
        push    si
        push    di
        mov     ax,PenColor
        push    ax              ; Save color

        mov     ax,LineStyle
        mov     LineRot,ax

        mov     x2,cx
        mov     y2,dx
        mov     ax,PenX
        mov     x1,ax
        mov     ax,PenY
        mov     y1,ax

        mov     si,x1
        mov     di,y1
	xor	ax,ax
        mov     acc,ax
        mov     ax,x2
	sub	ax,si
	mov	dx,1
	jns	nosx
	neg	ax
	mov	dx,-1
nosx:   mov     dirx,dx
        mov     deltax,ax
        mov     ax,y2
	sub	ax,di
	mov	dx,1
	jns	nosy
	neg	ax
	mov	dx,-1
nosy:   mov     diry,dx
        mov     deltay,ax
        cmp     ax,deltax
	jge	ylp

        call    plotline
        mov     ax,deltax
        mov     cnt,ax
lp1:
        dec     cnt
        js      done2
        add     si,dirx
        mov     ax,deltay
        add     acc,ax
        mov     ax,acc
        cmp     ax,deltax
	jb	noincy
        mov     ax,deltax
        sub     acc,ax
        add     di,diry

noincy: call    plotline
        jmp     lp1
done2:  jmp     done

ylp:    call    plotline
        mov     ax,deltay
        mov     cnt,ax
lp2:
        dec     cnt
	js	done
        add     di,diry
        mov     ax,deltax
        add     acc,ax
        mov     ax,acc
        cmp     ax,deltay
	jb	noincx
        mov     ax,deltay
        sub     acc,ax
        add     si,dirx

noincx: call    plotline
        jmp     lp2

done:   mov     ax,x2
        mov     PenX,ax
        mov     ax,y2
        mov     PenY,ax

        pop     ax
        mov     PenColor,ax
        pop     di
        pop     si
        pop     dx
        pop     ax
        ret

plotline:
        push    cx
        push    dx
        mov     cx,LineRot      ; fill style rotation
        and     cx,1
        mov     PenColor,cx
        mov     cx,si
        mov     dx,di
        call    PlotDot
        ror     LineRot,1
        pop     dx
        pop     cx
        ret


;       *********************
;       * AH=7h: PLOT POINT * CX=x DX=y
;       *********************

PlotDot:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    ds              ; save data segment

        add     cx,OriginX      ; calculate logical origin
        add     dx,OriginY

        mov     ax,PenColor     ; get color
        mov     ah,al
        mov     al,RepRule
        mov     di,ax           ; save it in di
        jmp     pdj0

NoDraw: jmp     pdexit


; --- IMAGE PLOT (its own rep-rule (AL)  & color (AH) ) ---

ImgPlot:

        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    ds              ; save data segment
        mov     di,ax           ; save color + rule

pdj0:   cmp     cx,ClipMaxX     ; check clipping boundaries
        ja      NoDraw
        cmp     cx,ClipMinX
        jb      NoDraw
        cmp     dx,ClipMaxY
        ja      NoDraw
        cmp     dx,ClipMinY
        jb      NoDraw

        cmp     CurMode,VGAmode      ; VGA mode?
        jne     pdj1
        mov     ax,VGAadd
        push    ax
        pop     ds
        mov     ax,dx
        jmp     pdj3

pdj1:   mov     ax,CGAeven      ; even scan lines 0,2,4
        mov     bx,dx
        and     bx,1            ; even?
        je      pdj4
        mov     ax,CGAodd       ; odd scan lines address
pdj4:   push    ax
        pop     ds
        mov     ax,dx
        shr     ax,1            ; divide by 2 (odd/even fields)

pdj3:   mov     bx,ax           ; bx = ax
        shl     ax,6            ; multiply ax by 64
        shl     bx,4            ; multiply bx by 16
        add     ax,bx           ; get total (value * 80)
        mov     si,ax           ; SI = row byte

        mov     ax,cx
        shr     ax,3            ; divide by 8
        add     si,ax           ; add column

        mov     bh,cl
        mov     bl,0
        shr     bx,3            ; modulo 8 CL=bit
        shr     bl,5            ; decimal adjust

        mov     cl,bl
        mov     bl,128
        shr     bl,cl           ; BL = Xbit AND value

        mov     ax,di           ; get color & rule


; --- Check rule & Write Dot routine ---

chkrule:
        and     al,7            ; TXT_RULE = FORCE_RULE
        cmp     al,4            ; INVxxx rule?
        jb      noinv
        and     al,3            ; clear INV bit
        xor     ah,1            ; invert color

noinv:  mov     dl,ds:[si]      ; read dot
        cmp     byte ptr cs:RevVideo,1
        jne     noinv2
        xor     dl,255
noinv2: 
        cmp     al,FORCE_RULE
        je      force_dot
        cmp     al,OR_RULE
        je      or_dot
        cmp     al,XOR_RULE
        je      xor_dot
        cmp     al,AND_RULE
        je      and_dot

force_dot:
        cmp     ah,1            ; visible?
        je      or_dot          ; turn dot on
dotoff: xor     bl,255
        and     dl,bl           ; turn dot off
        jmp     writedot
xor_dot:
        cmp     ah,1            ; visible?
        jne     pdexit
        xor     dl,bl
        jmp     writedot
and_dot:
        cmp     ah,0            ; color off?
        je      dotoff          ; turn pixel off
        mov     bh,bl
        and     bh,bl           ; pixel already off?
        je      pdexit          ; leave it off
or_dot: or      dl,bl           ; turn pixel off

writedot:
        cmp     byte ptr cs:RevVideo,1
        jne     wrdt1
        xor     dl,255
wrdt1:  mov     ds:[si],dl

pdexit: pop     ds
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret


;       ****************************
;       * AH=8h: MOVE PEN POSITION * CX=x DX=y
;       ****************************

MovePen:
        mov     PenX,cx
        mov     PenY,dx
        ret


;       ************************
;       * AH=9h: SET PEN COLOR *  AL= color, 0=white, 1=black (LCD)
;       ************************

SetColor:
        mov     ah,0
        mov     PenColor,ax
        ret


;       ********************************
;       * AH=0ah: SET REPLACEMENT RULE * AL=rule (0,1,2,3,8)
;       ********************************

SetRule:
        mov     RepRule,al
        ret


;       **************************
;       * AH=0bh: SET LINE STYLE * CX= style
;       **************************

SetLine:
        mov     LineStyle,cx
        ret


;       **********************
;       * AH=0ch: READ POINT * CX=x DX=y  returns pixel color in AL
;       **********************

ReadDot:
        push    bx
        push    cx
        push    dx
        push    si
        push    ds

        mov     ax,0
        add     cx,OriginX      ; calculate logical origin
        add     dx,OriginY

        cmp     cx,ClipMaxX     ; check clipping boundaries
        ja      gohop
        cmp     cx,ClipMinX
        jb      rdpj1
        cmp     dx,ClipMaxY
gohop:  ja      rdpj1
        cmp     dx,ClipMinY
        jb      rdpj1

        cmp     CurMode,VGAmode      ; VGA mode?
        jne     rdj1
        mov     ax,VGAadd
        push    ax
        pop     ds
        mov     ax,dx
        jmp     rdj3

rdj1:   mov     ax,CGAeven      ; even scan lines 0,2,4
        mov     bx,dx
        and     bx,1            ; even?
        je      rdj4
        mov     ax,CGAodd       ; odd scan lines address
rdj4:   push    ax
        pop     ds
        mov     ax,dx
        shr     ax,1            ; divide by 2 (odd/even fields)

rdj3:   mov     bx,ax           ; bx = ax
        shl     ax,6            ; multiply ax by 64
        shl     bx,4            ; multiply bx by 16
        add     ax,bx           ; get total (value * 80)
        mov     si,ax           ; SI = row byte

        mov     ax,cx
        shr     ax,3            ; divide by 8
        add     si,ax           ; add column

        mov     bh,cl
        mov     bl,0
        shr     bx,3            ; modulo 8 CL=bit
        shr     bl,5            ; decimal adjust

        mov     cl,bl
        mov     bl,128
        shr     bl,cl           ; BL = Xbit AND value

        mov     ax,1
        mov     bh,ds:[si]      ; read dot
        and     bh,bl
        jne     rdpj1
        mov     ax,0

rdpj1:  pop     ds
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret


;       *********************
;       * AH=0dh: GET IMAGE * CX,DX - SI,BP ->ES:DI
;       *********************

GetImage:
        push    ax
        push    bx
        push    cx
        push    dx

        push    bp
        pop     ax
        mov     LimitY,ax
        mov     LimitX,si

        mov     StartX,cx
        mov     SrcX,cx
        mov     SrcY,dx

        mov     ax,0
        mov     DstX,ax
        mov     DstY,ax

        mov     ax,1
        mov     es:[di+0],ax    ; # of planes
        mov     es:[di+2],ax    ; bits per plane

        mov     ax,LimitX
        sub     ax,cx
        mov     es:[di+4],ax    ; Image Width
        mov     ImgW,ax

        mov     ax,LimitY
        sub     ax,dx
        inc     ax
        mov     es:[di+6],ax    ; Image height
        mov     ImgH,ax

        mov     ax,ImgW      ;((x2-x1+8)/8)
        mov     bl,8
        div     bl
        cmp     ah,0
        je      gimj1
        inc     al
gimj1:  mov     Xbytes,al    ; # of X bytes

        mov     ax,ImgH
        mov     bl,Xbytes
        mov     bh,0
        mul     bx
        mov     cx,ax           ; CX= buffer lenght

        push    di
        cld
        add     di,8            ; bypass IMG header
        mov     al,0
giml3:  stosb
        loop    giml3           ; clear image buffer
        pop     di

giml1:  mov     cx,SrcX
        mov     dx,SrcY
        call    ReadDot         ; AL returns the color
        xor     al,RevVideo
        call    PutBit          ; put it in buffer

        mov     ax,SrcX
        cmp     ax,LimitX
        je      giml2           ; no more horiz pixels

        inc     SrcX
        inc     DstX
        jmp     giml1

giml2:  mov     ax,StartX
        mov     SrcX,ax      ; reset X source pointer
        mov     ax,0
        mov     DstX,ax      ; reset X dest pointer
        mov     ax,SrcY
        cmp     ax,LimitY
        je      giml4

        inc     SrcY
        inc     DstY         ; next vert row
        jmp     giml1

giml4:  pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret


; ---- PUT (AL) BIT into X,Y location of ES:DI -----

PutBit:
        push    di

        mov     dh,0
        mov     dl,al           ; save color in DL

        add     di,8            ; bypass IMG 8-byte header

        mov     ax,DstY         ; find Y row in buffer
        mov     bl,XBytes
        mul     bl              ; DstY * XBytes
        add     di,ax           ; DI points to the row

        mov     ax,DstX         ; find X column in buffer
        mov     bx,8
        div     bl              ; SrcX/8: AL= X byte AH=X bit

        mov     cl,ah
        mov     bl,128
        shr     bl,cl           ; BL = Xbit AND value

        mov     ah,0            ; AL = X offset
        add     di,ax           ; X offset
        cmp     dl,1            ; check color
        jne     pbt2            ; leave bit to 0

        mov     al,es:[di]
        or      al,bl           ; set the bit
        mov     es:[di],al

pbt2:   pop     di
        ret

; ***************************
; write byte to screen
; ES:DI points to screen
; AH is mask (1 = put)
; AL is data

PutByte:
        cmp RevVideo, 0
        jz  pb0
        xor byte ptr es:[di], 255
        
pb0:    mov  cx, ImgRule
        mov  ch, cl
        and ch, 4          ; CH is not 0 -> invert
        and cl, 3
        cmp cl, 0
        jne pb2
        and ch, ch
        jnz pb1
        and al, ah
        not ah
        and byte ptr es:[di], ah
        or byte ptr es:[di], al
        jmp pbend
        
pb1:    and al, ah
        not al
        or byte ptr es:[di], ah
        and byte ptr es:[di], al
        jmp pbend
        
pb2:    cmp cl, 1
        jne pb3
        and ch, ch
        jz  pb2a
        not al
pb2a:   not ah
        or  al, ah
        and byte ptr es:[di], al
        jmp pbend

pb3:    cmp cl, 2
        jne pb4
        and al, ah
        or byte ptr es:[di], al
        jmp pbend

pb4:    and al, ah
        xor byte ptr es:[di], al

pbend:  cmp RevVideo, 0
        jz  pbend2
        xor byte ptr es:[di], 255
pbend2: ret


;       *********************
;       * AH=0eh: PUT IMAGE * ES:DI -> CX,DX   AL= rep-rule
;       *********************

PutImage:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     ah,0
        mov     ImgRule,ax

        mov     DstX,cx      ; set X,Y dest. pointers
        mov     DstY,dx

        add     dx,es:[di+6]    ; compute Y limit
        mov     LimitY,dx

        mov     ax,es:[di+0]
        mov     planes, ax      ; number of planes (-1 is abused for
                                ; solid fill

        mov     ax,es:[di+4]
        mov     ImgW,ax      ; image width
        inc     ImgW
        add     ax, 7
        mov     cl, 3
        shr     ax, cl
        mov     bpl, ax         ; number of bytes per line in memory

        mov     dx, DstX
        mov     cl, 3
        shr     dx, cl
        mov     ax, DstX
        add     ax, ImgW
        dec     ax
        mov     cl, 3
        shr     ax, cl
        sub     ax, dx
        inc     ax
        mov     bpl1, ax        ; number of bytes per line on screen

pil1:   add     di, 8           ; find beginning of data ES:DI
        mov     si, di          ; ES:SI points to source data
        
        mov     cx, DstX        ; set up mask for first byte
        and     cl, 7
        mov     shift, cl
        mov     al, 255
        shr     al, cl
        mov     mask1, al
        mov     cx, DstX        ; set up mask for last byte
        add     cx, ImgW
        and     cl, 7
        mov     al, 255
        and     cl, cl
        jz      pxl2
        shr     al, cl
        not     al
pxl2:   mov     mask2, al 

        mov     cx, DstY        ; CX is y

plp1:                           ; start line loop
        mov     al,CurMode
        cmp     al,VGAmode
        je      pkrx
        
        mov     vidseg, CGAeven
        test    cl, 1           ; odd line?
        jz      pim1
        mov     vidseg, CGAodd  ; vidseg now points to video segment
        jmp     pim1
pkrx:   mov     vidseg, VGAadd
        push    cx
        jmp     pim1a

pim1:   push    cx              ; save line number
        shr     cx, 1
pim1a:  mov     ax, 80
        mul     cx
        mov     di, ax          ; DI is now screen address at x=0

        mov     ax, DstX
        mov     cl, 3
        shr     ax, cl
        add     di, ax          ; DI is now first byte on screen
        mov     pos1, di
        mov     ax, di
        add     ax, bpl1
        dec     ax
        mov     pos2, ax

plp2:                           ; start interline loop

        mov     al, es:[si]       ; AL is data byte
        cmp     planes, -1
        jne     ppl1
        mov     al, 255          ; fill all
ppl1:   mov     cl, shift
        and     cl, cl
        jz      pim2            ; so we need to shift?
        dec     si
        mov     ah, es:[si]       ; AH is previous data byte
        inc     si
        cmp     planes, -1
        jne     ppl2
        mov     ah, 255          ; fill all
ppl2:   shr     ax, cl          ; AL is now combined byte
        
pim2:   mov     ah, 255         ; AH is mask
        cmp     di, pos1        ; apply masks for 1st and last byte
        jne     pim3
        and     ah, mask1
pim3:   cmp     di, pos2
        jne     pim4
        and     ah, mask2
pim4:   push    es              ; save ES
        push    vidseg
        pop     es              ; set up ES for video

        pop     cx              ; get line number
        push    cx
        cmp     cx, ClipMinY
        jb      pim4a
        call    Putbyte         ; write byte to screen
pim4a:  pop     es              ; retrieve old ES
        
        inc     si              ; next byte
        inc     di
        
        cmp     di, pos2
        jbe     plp2            ; loop if not done with line

        mov     ax, bpl
        cmp     ax, bpl1
        je      pil2a
        dec     si              ; fix address if needed        

pil2a:  pop     cx              ; retrieve y
        inc     cx
        cmp     cx, ClipMaxY
        ja      pil3
        cmp     cx, LimitY
        jae     pil3
        jmp     plp1           ; process next line

pil3:   pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret


;       **********************
;       * AH=0fh: WRITE TEXT * ES:DI -> CX,DX  (AL=rot)
;       **********************

WriteText:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di

wtl1:   mov     si,FontOff      ; Font offset
        add     si,10           ; bypass HFN header

        mov     ax,FontHeight
        cmp     FontWidth,8
        je      wtj1
        shl     ax,1            ; x2 if 16-bit wide
wtj1:   add     ax,4            ; bypass special img header
        mov     bl,es:[di]      ; get ASCII char in AX
        cmp     bl,0            ; end of string?
        je      wtj2

        push    dx              ; wasted in mult
        mov     bh,0
        mul     bx              ; multiply by ASCII code
        add     si,ax           ; add offset to SI
        sub     si,4            ; point 2 int's behind
        pop     dx

        push    di
        push    es

        mov     ax,FontSeg
        push    ax
        pop     es
        mov     di,si           ; ES:DI points to char image
        mov     ax,PenColor
        mov     ah,al
        mov     al,RepRule
        cmp     ah,0
        jne     wtj3
        or      al,4            ; INVERT + FORCE if PEN is white
wtj3:   call    PutImage        ; draw character at CX,DX

        pop     es
        pop     di

        inc     di
        mov     ah,0
        mov     al,FontSpacing
        add     cx,ax
        jmp     wtl1

wtj2:   pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret


;       ****************************
;       * AH=10h: GET FONT POINTER * CX = font size
;       ****************************

GetFont:
        cmp     cx,0808h                ; small font 8x8
        jne     gfj1
        mov     FontHeight,8
        mov     FontWidth,8
        mov     FontSpacing,8
        lea     ax,SmallFont
        jmp     gfj3

gfj1:   cmp     cx,0a0bh                ; medium font 10x11
        jne     gfj2
        mov     FontHeight,11
        mov     FontWidth,10
        mov     FontSpacing,10

        mov     ax,8                    ; So Palinit doesn't complain
        jmp     gfj3

gfj2:   cmp     cx,100ch                ; large font  16x12
        jne     gfj3
        mov     ax,10h
        mov     FontHeight,12
        mov     FontWidth,16
        mov     FontSpacing,16
        lea     ax,LargeFont

gfj3:   push    cs
        pop     dx                      ; DX: font segment
        ret


;       ********************
;       * AH=11h: SET FONT *  ES=seg DI=offset
;       ********************

SetFont:
        push    ax
        push    di

        push    es
        pop     ax

        cmp     di,8            ; PalInit workaround
        jne     sfj1
        lea     di,MediumFont

sfj1:   mov     FontSeg,ax
        mov     FontOff,di

        pop     di
        pop     ax
        ret


;       ***********************
;       * AH=12h: SCROLL AREA *  From Pen to CX,DX AL=dir SI=fill DI=dist
;       ***********************

Scroll:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        mov     bx,PenColor
        push    bx              ; save color

        mov     LimitX,cx
        mov     LimitY,dx

        mov     bx,si           ; save fill in BX
        cmp     al,0
        jne     scrj1
        jmp     sleft

scrj1:  cmp     al,1
        jne     scrj2
        jmp     sright

scrj2:  cmp     al,2
        jne     scrj3
        jmp     sup

scrj3:  cmp     al,3
        jne     scrj4
        jmp     sdown

scrj4:  pop     bx              ; restore color
        mov     PenColor,bx
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ---- scroll left ----

sleft:  mov     ax,PenX
        add     ax,di           ; add distance
        mov     StartX,ax       ; X start pos
        mov     si,ax
        mov     dx,PenY

slej1:  mov     cx,si
        call    ReadDot         ; read pixel
        xor     al,RevVideo
        mov     PenColor,ax     ; save color
        sub     cx,di           ; new X destination
        call    PlotDot         ; write pixel
        cmp     si,LimitX
        je      slej2           ; no more H pixels
        inc     si
        jmp     slej1

slej2:  mov     PenColor,bx     ; fill color
        mov     cx,si
        mov     si,1            ; 1st pixel is already filled
slej5:  call    PlotDot
        cmp     si,di           ; finished filling?
        je      slej4
        inc     si
        dec     cx
        jmp     slej5

slej4:  mov     si,StartX
        cmp     dx,LimitY
        je      slej3
        inc     dx
        jmp     slej1

slej3:  jmp     scrj4


; ---- scroll right ----

sright: mov     ax,LimitX
        sub     ax,di           ; substract distance
        mov     StartX,ax       ; X start pos
        mov     si,ax
        mov     dx,PenY

srij1:  mov     cx,si
        call    ReadDot         ; read pixel
        xor     al,RevVideo
        mov     PenColor,ax     ; save color
        add     cx,di           ; new X destination
        call    PlotDot         ; write pixel
        cmp     si,PenX
        je      srij2           ; no more H pixels
        dec     si
        jmp     srij1

srij2:  mov     PenColor,bx     ; fill color
        mov     cx,si
        mov     si,1
srij5:  call    PlotDot
        cmp     si,di           ; finished filling?
        je      srij4
        inc     si
        inc     cx
        jmp     srij5

srij4:  mov     si,StartX
        cmp     dx,LimitY
        je      srij3
        inc     dx
        jmp     srij1

srij3:  jmp     scrj4


; ---- scroll up ----

sup:    mov     ax,PenY
        add     ax,di           ; add distance
        mov     StartY,ax       ; Y start pos
        mov     si,ax
        mov     cx,PenX

supj1:  mov     dx,si
        call    ReadDot         ; read pixel
        xor     al,RevVideo
        mov     PenColor,ax     ; save color
        sub     dx,di           ; new Y destination
        call    PlotDot         ; write pixel
        cmp     si,LimitY
        je      supj2           ; no more V pixels
        inc     si
        jmp     supj1

supj2:  mov     PenColor,bx     ; fill color
        mov     dx,si
        mov     si,1
supj5:  call    PlotDot
        cmp     si,di           ; finished filling?
        je      supj4
        inc     si
        dec     dx
        jmp     supj5

supj4:  mov     si,StartY
        cmp     cx,LimitX
        je      supj3
        inc     cx
        jmp     supj1

supj3:  jmp     scrj4


; ---- scroll down ----

sdown:  mov     ax,LimitY
        sub     ax,di           ; substract distance
        mov     StartY,ax       ; Y start pos
        mov     si,ax
        mov     cx,PenX

sdoj1:  mov     dx,si
        call    ReadDot         ; read pixel
        xor     al,RevVideo
        mov     PenColor,ax     ; save color
        add     dx,di           ; new Y destination
        call    PlotDot         ; write pixel
        cmp     si,PenY
        je      sdoj2           ; no more V pixels
        dec     si
        jmp     sdoj1

sdoj2:  mov     PenColor,bx     ; fill color
        mov     dx,si
        mov     si,1
sdoj5:  call    PlotDot
        cmp     si,di           ; finished filling?
        je      sdoj4
        inc     si
        inc     dx
        jmp     sdoj5

sdoj4:  mov     si,StartY
        cmp     cx,LimitX
        je      sdoj3
        inc     cx
        jmp     sdoj1

sdoj3:  jmp     scrj4


;       *****************************
;       * AH=13h: SET GRAPHICS INFO * ES:DI= 44-byte buffer
;       *****************************

SetGinfo:
        push    cx
        push    si
        mov     cx,44           ; transfer 44 bytes
        lea     si,cs:ginfo
sil1:   mov     al,es:[di]
        mov     cs:[si],al
        inc     si
        inc     di
        loop    sil1
        pop     si
        pop     cx
        ret


;       ************************
;       * AH=14h: SET DEFAULTS *
;       ************************

SetDefaults:
        push    ax
        push    cx
        push    si
        mov     ax,0ffffh
        mov     LineStyle,ax
        mov     al,8
        mov     RepRule,al
        mov     ax,1
        mov     PenColor,ax

        mov     FontHeight,8
        mov     FontWidth,8
        mov     FontSpacing,8

        mov     ClipMinX,0
        mov     ClipMinY,0
        mov     ClipMaxX,639
        mov     ClipMaxY,199

        mov     DispWidth,640
        mov     DispHeight,200

        lea     ax,SmallFont
        mov     FontOff,ax
        push    cs
        pop     ax
        mov     FontSeg,ax

        push    cs
        pop     ds
        push    cs
        pop     es

        cld
        mov     cx,8
        mov     al,0ffh
        lea     di,FillMask
sdl1:   stosb
        loop    sdl1

        pop     si
        pop     cx
        pop     ax
        ret


Signature:
        db      "HK"

keep    label   byte            ; end of TSR code block


        ;********************
        ;* TSR INSTALLATION *
        ;********************

install:
        call    cmdtail         ; process command line

        mov     es,ds:2ch
        mov     ah,49h
        int     21h
        lea     dx,msg1
        mov     ah,9
        int     21h

        mov     ah,35h
        mov     al,hook
        int     21h
        cmp     bx,offset int5f
        jne     br1

        lea     dx,msg8
        mov     ah,9
        int     21h
        mov     ax,4c00h
        int     21h                     ; quit

br1:
        mov     old_int_off,bx
        mov     old_int_seg,es

        mov     ah,25h
        mov     al,hook
        mov     dx,offset int5f
        int     21h

        lea     dx,msg2
        cmp     cs:ForceVGA,1
        jne     br3
        lea     dx,msg5
br3:    mov     ah,9
        int     21h                     ; installed

        mov     dx,offset keep+1
        int     27h                     ; stay resident


;       ***********************************
;       * COMMAND TAIL ANALYSIS & PROCESS *
;       ***********************************

cmdtail:
        cmp     byte ptr[80h],1         ; any command line parameter?
        ja      cmd1
        jmp     opt4                    ; no arguments, give help

cmd1:   mov     si,81h
cmdtlp: lodsb
        cmp     al,"/"
        je      optdec
        cmp     al,13                   ; end of line?
        je      eol
        cmp     al,32                   ; space ?
        je      cmdtlp
eol:    ret

optdec:
        lodsb
        call    ucase                   ; convert to upper case

        cmp     al,"V"                  ; VGA mode
        jne     opt1
        mov     ForceVGA,1
        ret

opt1:   cmp     al,"R"                  ; Inverse VGA
        jne     opt2
        mov     ForceVGA,1
        mov     RevVideo,1
        ret

opt2:   cmp     al,"C"                  ; CGA mode
        jne     opt3
        mov     ForceVGA,0
        ret

opt3:   cmp     al,"?"
        je      opt4
        cmp     al,"H"
        je      opt4
        jmp     opt5
opt4:   lea     dx,msg1
        mov     ah,9
        int     21h
        lea     dx,msg7                 ; help screen
        jmp     opt10                   ; quit

opt5:   cmp     al,"I"                  ; inverse CGA
        jne     opt6
        mov     ForceVGA,0
        mov     ah,1
        mov     RevVideo,ah
        ret

opt6:   cmp     al,"U"
        jne     opt9
        jmp     Uninstall

opt9:   lea     dx,msg6                 ; invalid option
opt10:  mov     ah,9
        int     21h     
        mov     ax,4c00h
        int     21h                     ; quit

; --- Convert to upper case ---

ucase:  cmp     al,61h
        jb      udone
        cmp     al,7ah
        ja      udone
        and     al,5fh                  ; clear 5th bit
udone:  ret


;       ****************************
;       * UNINSTALL INT5F SERVICES *
;       ****************************

Uninstall:

        mov     ax,355fh
        int     21h                     ; get INT5f vector
        cmp     WORD PTR es:[Signature],"KH"
        jnz     fail                    ; can't find TSR

        mov     al,3
        mov     ah,0
        int     10h                     ; restore video mode

        push    ds
        lds     dx,ES:[old_int_vector]
        mov     ax,255fh
        int     21h                     ; reload old vector
        pop     ds

        push    es
        mov     es,es:[02CH]            ; PSP env. segment
        mov     ah,49h
        int     21h
        pop     es
        mov     ah,49h                  ; free memory
        int     21h

        lea     dx,msg4
        mov     ah,9
        int     21h                     ; uninstall message
        mov     ax,4c00h
        int     21h                     ; quit

fail:   lea     dx,msg3
        mov     ah,9
        int     21h                     ; uninstall failed message
        mov     ax,4c00h
        int     21h                     ; quit




msg1    db      10,"HP200LX INT5fh CGA Graphics driver for PAL applications.",13,10
        db      "Vr.1.01 (08/97). Freeware by Harry Konstas. ",13,10,10,"$"
msg2    db      "INT5fh Driver installed in CGA mode.",13,10,"$"
msg3    db      7,"WARNING: INT5fh Driver cannot be removed. Not installed?",13,10,"$"
msg4    db      "INT5fh driver removed from memory.",13,10,"$"
msg5    db      "INT5fh Driver installed in VGA mode.",13,10,"$"
msg6    db      7,"INT5F: Invalid command line switch.",13,10,"$"
msg8    db      7,"INT5F: Driver already installed.",13,10,"$"

msg7    db      "USAGE: int5f [switches]  ex: int5f /v",13,10,10
        db      " Command line switches:",13,10
        db      "  /v = Install in VGA mode.",13,10
        db      "  /c = Install in CGA mode (default).",13,10
        db      "  /r = Install in reverse VGA mode.",13,10
        db      "  /i = Install in inverse CGA mode.",13,10
        db      "  /u = Uninstall INT5F driver from memory.",13,10

        db      13,10,10,"$"

        cseg    ends
        end     start

