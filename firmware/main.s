	.file	"main.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.section	.text.spi,"ax",@progbits
	.type	spi, @function
spi:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	movw r30,r24
	movw r26,r22
	movw r20,r24
	subi r20,-4
	sbci r21,-1
.L5:
	ld r22,Z+
	ldi r18,lo8(8)
	ldi r19,0
	ldi r24,lo8(-128)
	ldi r25,0
.L4:
	mov r23,r24
	and r23,r22
	breq .L2
	sbi 0x18,0
.L2:
	lds r23,sck_period
/* #APP */
 ;  354 "main.c" 1
	        mov   __tmp_reg__,r23    
spd29:  rjmp  .                 
        rjmp  .                 
        nop                     
        dec   __tmp_reg__       
        brne  spd29             

 ;  0 "" 2
/* #NOAPP */
	sbi 0x18,2
	lds r23,sck_period
/* #APP */
 ;  354 "main.c" 1
	        mov   __tmp_reg__,r23    
spd34:  rjmp  .                 
        rjmp  .                 
        nop                     
        dec   __tmp_reg__       
        brne  spd34             

 ;  0 "" 2
/* #NOAPP */
	lsl r25
	sbic 0x16,1
	subi r25,lo8(-(1))
.L3:
	cbi 0x18,0
	cbi 0x18,2
	lsr r24
	subi r18,1
	sbc r19,__zero_reg__
	brne .L4
	st X+,r25
	cp r30,r20
	cpc r31,r21
	brne .L5
/* epilogue start */
	ret
	.size	spi, .-spi
	.section	.text.spi_rw,"ax",@progbits
	.type	spi_rw, @function
spi_rw:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	lds r24,address
	lds r25,address+1
	movw r18,r24
	subi r18,-1
	sbci r19,-1
	sts address+1,r19
	sts address,r18
	lds r18,cmd0
	sbrs r18,7
	rjmp .L16
	lsl r24
	rol r25
.L16:
	sbrc r24,0
	ori r18,lo8(8)
.L19:
	sts cmd,r18
	mov r18,r25
	lsr r18
	sts cmd+1,r18
	lsr r25
	ror r24
	sts cmd+2,r24
	ldi r22,lo8(res)
	ldi r23,hi8(res)
	ldi r24,lo8(cmd)
	ldi r25,hi8(cmd)
	rjmp spi
	.size	spi_rw, .-spi_rw
	.section	.text._delay_us,"ax",@progbits
.global	_delay_us
	.type	_delay_us, @function
_delay_us:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
/* #APP */
 ;  191 "main.c" 1
	                                                                       
;       Delay 1 to 65536 microseconds (pass 0 for 65536 microseconds)  
                                                                       
;       Assume we enter with 5 cycles already used:                    
;         Microsecond parameter load (e.g. 2 x ldi) has taken 2 cycles 
;         Call has taken 3 cycles (4 if more than 2k words distant)    
                                                                       
;       Want to reach dus10 on a multiple of 16.5 + 12.5 including     
;       call time but excluding return time.                           
                                                                       
;       E.g. cycles taken inc setup, call and return will be:          
;actual 16,   33, 49,   66, 82,   99, 115    ...                       
;ideal  16.5, 33, 49.5, 66, 82.5, 99, 115.5  ...                       
                                                                       
        rjmp  dus8            ; 2. 7  Enter loop allowing for call     
                              ;       and returnn overhead             
                                                                       
;       One microsecond takes 16.5 cycles, so use a loop of alternate  
;       16 and 17 cycle delays.                                        
                                                                       
                              ;          13  46  79                    
dus2:   ldi   r18,4           ; +                                      
dus4:   dec   r18             ;  > 12.                                 
        brne  dus4            ; +        25  58  91                    
        sbiw  r24,1           ; 2.       27  60  93                    
        brcs  dus10           ; If complete reaches dus10 at 29 + n*33 
                              ;          28  61  94                    
        ldi   r18,4           ; +                                      
dus6:   dec   r18             ;  > 12.                                 
        brne  dus6            ; +        40  73  106                   
dus8:   rjmp  .               ; 2.    9  42  75  108                   
        sbiw  r24,1           ; 2.   11  44  77  110                   
        brcc  dus2            ; If more delay required                 
                                                                       
dus10:                        ;      12  45  78  111                   
                                                                       
;       Return takes 4 cycles        16  49  82  115                   

 ;  0 "" 2
/* #NOAPP */
	ret
	.size	_delay_us, .-_delay_us
	.section	.text._delay_ms,"ax",@progbits
.global	_delay_ms
	.type	_delay_ms, @function
_delay_ms:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
/* #APP */
 ;  233 "main.c" 1
	                                                                       
;       Delay 1 to 65536 milliseconds (pass 0 for 65536 milliseconds)  
                                                                       
        movw  r26,r24         ; Loop count in r27:r26                  
                                                                       
dms2:   rjmp  .               ; 2. 2                                   
        rjmp  .               ; 2. 4                                   
        rjmp  .               ; 2. 6                                   
        rjmp  .               ; 2. 8                                   
        rjmp  .               ; 2. 10                                  
        rjmp  .               ; 2. 12                                  
        ldi   r24,lo8(999)    ;                                        
        ldi   r25,hi8(999)    ;                                        
        rcall _delay_us       ;    16495                               
        nop                   ; 1. 16496                               
        sbiw  r26,1           ; 2. 16498                               
        brcc  dms2            ; if more, 16500                         

 ;  0 "" 2
/* #NOAPP */
	ret
	.size	_delay_ms, .-_delay_ms
	.section	.text.usbFunctionRead,"ax",@progbits
.global	usbFunctionRead
	.type	usbFunctionRead, @function
usbFunctionRead:
	push r14
	push r15
	push r17
	push r28
	push r29
	push __zero_reg__
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 1 */
/* stack size = 6 */
.L__stack_usage = 6
	mov r17,r24
	movw r14,r24
.L23:
	mov r24,r14
	sub r24,r17
	cp r24,r22
	brsh .L25
	std Y+1,r22
	rcall spi_rw
	lds r24,res+3
	movw r30,r14
	st Z+,r24
	movw r14,r30
	ldd r22,Y+1
	rjmp .L23
.L25:
	mov r24,r22
/* epilogue start */
	pop __tmp_reg__
	pop r29
	pop r28
	pop r17
	pop r15
	pop r14
	ret
	.size	usbFunctionRead, .-usbFunctionRead
	.section	.text.usbFunctionWrite,"ax",@progbits
.global	usbFunctionWrite
	.type	usbFunctionWrite, @function
usbFunctionWrite:
	push r13
	push r14
	push r15
	push r16
	push r17
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 7 */
.L__stack_usage = 7
	mov r28,r22
	lds r18,dwState
	cpse r18,__zero_reg__
	rjmp .L27
	mov r29,r24
	movw r16,r24
	ldi r19,lo8(96)
	mov r13,r19
	rjmp .L28
.L27:
	lds r18,dwIn
	lds r20,dwLen
	ldi r23,0
	add r22,r18
	adc r23,__zero_reg__
	ldi r21,0
	ldi r18,lo8(1)
	cp r22,r20
	cpc r23,r21
	brge .L41
	ldi r18,0
	rjmp .L30
.L41:
	lds r28,dwLen
	lds r19,dwIn
	sub r28,r19
	ldi r19,lo8(20)
	sts jobState,r19
.L30:
	mov r19,r24
	movw r26,r24
.L31:
	mov r24,r26
	sub r24,r19
	cp r24,r28
	brsh .L42
	lds r30,dwIn
	ldi r24,lo8(1)
	add r24,r30
	sts dwIn,r24
	ldi r31,0
	ld r24,X+
	subi r30,lo8(-(dwBuf))
	sbci r31,hi8(-(dwBuf))
	st Z,r24
	rjmp .L31
.L42:
	mov r24,r18
	rjmp .L33
.L35:
	lds r24,sck_period
	ldi r25,0
	ldi r18,5
	1:
	lsl r24
	rol r25
	dec r18
	brne 1b
	add r14,r24
	adc r15,r25
.L34:
	lds r24,timeout
	lds r25,timeout+1
	cp r14,r24
	cpc r15,r25
	brsh .L28
	ldi r22,lo8(res)
	ldi r23,hi8(res)
	ldi r24,lo8(cmd)
	ldi r25,hi8(cmd)
	rcall spi
	lds r24,res+3
	lds r25,cmd+3
	cpse r24,r25
	rjmp .L35
	lds r25,poll1
	cp r24,r25
	breq .L35
	lds r25,poll2
	cp r24,r25
	breq .L35
.L28:
	mov r24,r16
	sub r24,r29
	cp r24,r28
	brsh .L43
	movw r30,r16
	ld r24,Z+
	movw r16,r30
	sts cmd+3,r24
	rcall spi_rw
	lds r24,cmd
	eor r24,r13
	sts cmd,r24
	mov r14,__zero_reg__
	mov r15,__zero_reg__
	rjmp .L34
.L43:
	ldi r24,lo8(1)
.L33:
/* epilogue start */
	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	ret
	.size	usbFunctionWrite, .-usbFunctionWrite
	.section	.text.ws2812_sendarray_mask,"ax",@progbits
.global	ws2812_sendarray_mask
	.type	ws2812_sendarray_mask, @function
ws2812_sendarray_mask:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	in r18,0x18
	mov r19,r20
	com r19
	and r18,r19
	in r19,0x18
	or r20,r19
	add r22,r24
	adc r23,r25
.L45:
	cp r24,r22
	cpc r25,r23
	breq .L47
	movw r30,r24
	ld r19,Z+
	movw r24,r30
/* #APP */
 ;  515 "main.c" 1
	                                                
         ldi   r21,8     ; 0                     
loop303:  out   24,r20    ; 1                     
         lsl   r19       ; 2                     
         dec   r21       ; 3                     
                                                
         rjmp  .+0      ; 5                     
                                                
         brcs  .+2      ; 6l / 7h               
         out   24,r18    ; 7l / -                
                                                
         rjmp  .+0      ; 9                     
                                                
         nop            ; 10                    
         out   24,r18    ; 11                    
         breq  end303    ; 12      nt. 13 taken  
                                                
         rjmp  .+0      ; 14                    
         rjmp  .+0      ; 16                    
         rjmp  .+0      ; 18                    
         rjmp  loop303   ; 20                    
end303:                                          

 ;  0 "" 2
/* #NOAPP */
	rjmp .L45
.L47:
/* epilogue start */
	ret
	.size	ws2812_sendarray_mask, .-ws2812_sendarray_mask
	.section	.text.usbFunctionSetup,"ax",@progbits
.global	usbFunctionSetup
	.type	usbFunctionSetup, @function
usbFunctionSetup:
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	movw r28,r24
	ldd r18,Y+1
	cpse r18,__zero_reg__
	rjmp .L49
	ldi r24,lo8(33)
	std Y+1,r24
	sts usbMsgPtr+1,r29
	sts usbMsgPtr,r28
	ldi r25,lo8(8)
	rjmp .L50
.L49:
	cpi r18,lo8(1)
	brne .L51
	in r24,0x16
	rjmp .L127
.L51:
	ldd r22,Y+2
	cpi r18,lo8(2)
	brne .L52
	out 0x18,r22
	rjmp .L126
.L52:
	mov r25,r22
	mov r19,r22
	andi r19,lo8(7)
	ldi r20,lo8(1)
	ldi r21,0
	mov r0,r19
	rjmp 2f
	1:
	lsl r20
	rol r21
	2:
	dec r0
	brpl 1b
	cpi r18,lo8(3)
	brne .+2
	rjmp .L139
	cpi r18,lo8(4)
	brne .L54
.L138:
	in r24,0x18
	or r20,r24
.L132:
	out 0x18,r20
	rjmp .L126
.L54:
	cpi r18,lo8(5)
	brne .L55
	sts sck_period,r22
	cbi 0x17,1
	in r24,0x17
	ori r24,lo8(37)
	out 0x17,r24
	in r24,0x18
	andi r24,lo8(-40)
	out 0x18,r24
	rjmp .L126
.L55:
	cpi r18,lo8(6)
	brne .L56
	cbi 0x17,0
	cbi 0x18,0
	cbi 0x17,1
	cbi 0x18,1
	cbi 0x17,2
	cbi 0x18,2
	cbi 0x17,5
	cbi 0x18,5
	rjmp .L126
.L56:
	cpi r18,lo8(7)
	brne .L57
	movw r22,r28
	movw r24,r28
	adiw r24,2
	rcall spi
	sts usbMsgPtr+1,r29
	sts usbMsgPtr,r28
	ldi r25,lo8(4)
	rjmp .L50
.L57:
	cpi r18,lo8(8)
	brne .L58
	sts poll1,r22
	ldd r24,Y+3
	sts poll2,r24
	rjmp .L126
.L58:
	ldd r30,Y+4
	ldd r31,Y+5
	sts address+1,r31
	sts address,r30
	cpi r18,lo8(9)
	brne .L59
	ldi r24,lo8(32)
	rjmp .L130
.L59:
	cpi r18,lo8(11)
	brne .L60
	ldi r24,lo8(-96)
.L130:
	sts cmd0,r24
	rjmp .L129
.L60:
	ldd r30,Y+2
	ldd r31,Y+3
	sts timeout+1,r31
	sts timeout,r30
	cpi r18,lo8(10)
	brne .L61
	ldi r24,lo8(64)
	rjmp .L130
.L61:
	cpi r18,lo8(12)
	brne .L62
	ldi r24,lo8(-64)
	rjmp .L130
.L62:
	cpi r18,lo8(13)
	brne .L63
	in r24,0x17
	com r20
	and r20,r24
	rjmp .L137
.L63:
	cpi r18,lo8(14)
	brne .L64
	in r24,0x17
	or r20,r24
.L137:
	out 0x17,r20
	rjmp .L126
.L64:
	cpi r18,lo8(15)
	brne .L65
	cpse r22,__zero_reg__
	rjmp .L66
	lds r24,adcSetting
	out 0x7,r24
	sbi 0x14,5
	rjmp .L67
.L66:
	cpi r22,lo8(1)
	brne .L68
	lds r24,adcSetting
	ori r24,lo8(1)
	out 0x7,r24
	sbi 0x14,2
	rjmp .L67
.L68:
	cpi r22,lo8(2)
	brne .L67
	ldi r24,lo8(-113)
	out 0x7,r24
.L67:
	sbi 0x6,6
.L69:
	sbic 0x6,6
	rjmp .L69
	in r24,0x4
	st Y,r24
	in r24,0x5
	std Y+1,r24
	sts usbMsgPtr+1,r29
	sts usbMsgPtr,r28
	out 0x14,__zero_reg__
	ldi r25,lo8(2)
	rjmp .L50
.L65:
	cpi r18,lo8(16)
	brne .L70
	sbi 0x17,0
	sbi 0x17,1
	in r24,0x2a
	ori r24,lo8(-96)
	out 0x2a,r24
	in r24,0x2a
	ori r24,lo8(3)
	out 0x2a,r24
	in r24,0x33
	ori r24,lo8(5)
	rjmp .L134
.L70:
	cpi r18,lo8(17)
	brne .L71
	out 0x29,r22
	ldd r24,Y+4
	out 0x28,r24
	rjmp .L126
.L71:
	cpi r18,lo8(18)
	brne .+2
	rjmp .L138
	cpi r18,lo8(19)
	brne .L73
.L139:
	in r24,0x18
	com r20
	and r20,r24
	rjmp .L132
.L73:
	cpi r18,lo8(20)
	brne .L74
	in r24,0x16
	ldi r25,0
	and r20,r24
	and r21,r25
	rjmp 2f
	1:
	asr r21
	ror r20
	2:
	dec r19
	brpl 1b
	st Y,r20
	rjmp .L128
.L74:
	cpi r18,lo8(22)
	brne .L75
	cpse r22,__zero_reg__
	rjmp .L76
	in r24,0x33
	andi r24,lo8(-5)
	out 0x33,r24
	in r24,0x33
	andi r24,lo8(-3)
	out 0x33,r24
	in r24,0x33
	ori r24,lo8(1)
	out 0x33,r24
	rjmp .L50
.L76:
	cpi r22,lo8(1)
	brne .L77
	in r24,0x33
	andi r24,lo8(-5)
	out 0x33,r24
	in r24,0x33
	ori r24,lo8(2)
	rjmp .L131
.L77:
	cpi r22,lo8(2)
	brne .L78
	in r24,0x33
	andi r24,lo8(-5)
	out 0x33,r24
	in r24,0x33
	ori r24,lo8(2)
	rjmp .L140
.L78:
	cpi r22,lo8(3)
	brne .L79
	in r24,0x33
	ori r24,lo8(4)
	out 0x33,r24
	in r24,0x33
	andi r24,lo8(-3)
.L131:
	out 0x33,r24
	in r24,0x33
	andi r24,lo8(-2)
.L134:
	out 0x33,r24
	rjmp .L126
.L79:
	cpi r22,lo8(4)
	breq .+2
	rjmp .L126
	in r24,0x33
	ori r24,lo8(4)
	out 0x33,r24
	in r24,0x33
	andi r24,lo8(-3)
.L140:
	out 0x33,r24
	in r24,0x33
	ori r24,lo8(1)
	rjmp .L134
.L75:
	cpi r18,lo8(31)
	brne .L81
	ldd r24,Y+3
	ldi r25,0
	mov r25,r24
	clr r24
	add r24,r22
	adc r25,__zero_reg__
	sts SPI_DELAY+1,r25
	sts SPI_DELAY,r24
	rjmp .L126
.L81:
	cpi r18,lo8(32)
	brne .L82
	out 0x2a,__zero_reg__
	out 0x33,__zero_reg__
	rjmp .L126
.L82:
	cpi r18,lo8(33)
	brne .L83
	sts rxBuffer,r22
	ldi r24,lo8(1)
	rjmp .L135
.L83:
	cpi r18,lo8(34)
	brne .L84
	ldi r24,lo8(19)
.L127:
	st Y,r24
.L128:
	sts usbMsgPtr+1,r29
	sts usbMsgPtr,r28
	ldi r25,lo8(1)
	rjmp .L50
.L84:
	cpi r18,lo8(35)
	brne .L85
	sbi 0x6,7
	in r25,0x6
	ldd r24,Y+2
	or r24,r25
	out 0x6,r24
	ldd r25,Y+3
	cpi r25,lo8(1)
	breq .L86
	brlo .L87
	cpi r25,lo8(2)
	breq .+2
	rjmp .L126
	lds r24,adcSetting
	ori r24,lo8(-112)
	rjmp .L133
.L87:
	sts adcSetting,__zero_reg__
	rjmp .L50
.L86:
	lds r24,adcSetting
	ori r24,lo8(-128)
.L133:
	sts adcSetting,r24
	rjmp .L126
.L85:
	cpi r18,lo8(40)
	brne .L89
	ldi r24,lo8(sendBuffer)
	ldi r25,hi8(sendBuffer)
	sts usbMsgPtr+1,r25
	sts usbMsgPtr,r24
	lds r25,sendBuffer+8
	rjmp .L50
.L89:
	cpi r18,lo8(41)
	brne .L90
	ldi r24,lo8(2)
	rjmp .L135
.L90:
	cpi r18,lo8(42)
	brne .L91
	ldi r24,lo8(3)
	rjmp .L136
.L91:
	cpi r18,lo8(43)
	brne .L92
	ldi r24,lo8(4)
	rjmp .L135
.L92:
	cpi r18,lo8(44)
	brne .L93
	ldi r24,lo8(8)
	rjmp .L135
.L93:
	cpi r18,lo8(45)
	brne .L94
	ldi r24,lo8(9)
.L136:
	sts jobState,r24
	sts rxBuffer,r22
	rjmp .L126
.L94:
	cpi r18,lo8(46)
	brne .L95
	ldi r24,lo8(11)
	sts jobState,r24
	sts rxBuffer,r22
	ldd r24,Y+3
	sts rxBuffer+1,r24
	ldd r24,Y+4
	sts rxBuffer+2,r24
	rjmp .L126
.L95:
	cpi r18,lo8(47)
	brne .L96
	cpi r22,lo8(1)
	brne .L97
	sbi 0x17,0
	sbi 0x17,1
	sbi 0x17,2
	sts softPWM,r22
	rjmp .L126
.L97:
	sts softPWM,__zero_reg__
	rjmp .L126
.L96:
	cpi r18,lo8(48)
	brne .L98
	sts compare0,r22
	ldd r24,Y+3
	sts compare1,r24
	ldd r24,Y+4
	sts compare2,r24
	rjmp .L126
.L98:
	cpi r18,lo8(49)
	brne .L99
	ldi r23,0
	sts I2C_DELAY+1,r23
	sts I2C_DELAY,r22
	rjmp .L126
.L99:
	cpi r18,lo8(50)
	brne .L100
	ldi r24,lo8(5)
	rjmp .L135
.L100:
	cpi r18,lo8(51)
	brne .L101
	sts rxBuffer,r22
	ldi r24,lo8(6)
.L135:
	sts jobState,r24
	rjmp .L126
.L101:
	cpi r18,lo8(54)
	brne .L102
	sbrs r22,5
	rjmp .L103
	lds r30,ws2812_ptr
	cpi r30,lo8(-64)
	brsh .L103
	ldd r24,Y+3
	mov r26,r30
	ldi r27,0
	subi r26,lo8(-(ws2812_grb))
	sbci r27,hi8(-(ws2812_grb))
	st X,r24
	ldd r24,Y+4
	ldi r26,lo8(1)
	add r26,r30
	ldi r27,0
	subi r26,lo8(-(ws2812_grb))
	sbci r27,hi8(-(ws2812_grb))
	st X,r24
	ldi r24,lo8(3)
	add r24,r30
	sts ws2812_ptr,r24
	ldd r24,Y+5
	subi r30,lo8(-(2))
	ldi r31,0
	subi r30,lo8(-(ws2812_grb))
	sbci r31,hi8(-(ws2812_grb))
	st Z,r24
.L103:
	ldd r24,Y+2
	sbrs r24,4
	rjmp .L126
	lds r24,ws2812_ptr
	tst r24
	brne .+2
	rjmp .L126
	ldi r24,lo8(17)
	sts jobState,r24
	in r24,0x17
	or r24,r20
	out 0x17,r24
	sts ws2812_mask,r20
	rjmp .L126
.L102:
	cpi r18,lo8(55)
	brne .L105
	lds r24,EE_addr
	lds r25,EE_addr+1
	rcall eeprom_write_byte
	lds r24,EE_addr
	lds r25,EE_addr+1
	ldd r22,Y+3
	adiw r24,1
	rcall eeprom_write_byte
	lds r24,EE_addr
	lds r25,EE_addr+1
	ldd r22,Y+4
	adiw r24,2
	rcall eeprom_write_byte
	rjmp .L126
.L105:
	cpi r18,lo8(60)
	brne .L106
	ld r24,Y
	sbrs r24,7
	rjmp .L107
	lds r24,dwState
	cpse r24,__zero_reg__
	rjmp .L126
	ldi r24,lo8(dwBuf)
	ldi r25,hi8(dwBuf)
	sts usbMsgPtr+1,r25
	sts usbMsgPtr,r24
	lds r25,dwLen
	rjmp .L50
.L107:
	sts dwState,r22
	lds r25,dwState
	cpse r25,__zero_reg__
	rjmp .L108
	ldi r24,lo8(20)
	sts jobState,r24
	rjmp .L50
.L108:
	lds r24,dwState
	cpi r24,lo8(4)
	brlo .+2
	rjmp .L126
	ldd r24,Y+6
	sts dwLen,r24
	lds r24,dwLen
	cpi r24,lo8(-127)
	brlo .L109
	ldi r24,lo8(-128)
	sts dwLen,r24
.L109:
	sts dwIn,__zero_reg__
.L129:
	ldi r25,lo8(-1)
	rjmp .L50
.L106:
	mov r24,r18
	andi r24,lo8(-16)
	cpi r24,lo8(-32)
	brne .L110
	ldi r24,lo8(10)
	sts jobState,r24
	mov r24,r18
	andi r24,lo8(7)
	sts rxBuffer,r24
	andi r18,lo8(8)
	sts rxBuffer+1,r18
	ldi r18,0
.L111:
	lds r24,rxBuffer
	cp r18,r24
	brsh .L126
	mov r24,r18
	ldi r25,0
	adiw r24,2
	movw r30,r28
	add r30,r24
	adc r31,r25
	ld r19,Z
	movw r30,r24
	subi r30,lo8(-(rxBuffer))
	sbci r31,hi8(-(rxBuffer))
	st Z,r19
	subi r18,lo8(-(1))
	rjmp .L111
.L110:
	cpi r24,lo8(-16)
	brne .L126
	ldi r24,lo8(7)
	sts jobState,r24
	mov r24,r18
	andi r24,lo8(8)
	sts rxBuffer,r24
	andi r18,lo8(7)
	sts rxBuffer+1,r18
	ldi r18,0
.L113:
	lds r24,rxBuffer+1
	cp r18,r24
	brsh .L126
	mov r24,r18
	ldi r25,0
	adiw r24,2
	movw r30,r28
	add r30,r24
	adc r31,r25
	ld r19,Z
	movw r30,r24
	subi r30,lo8(-(rxBuffer))
	sbci r31,hi8(-(rxBuffer))
	st Z,r19
	subi r18,lo8(-(1))
	rjmp .L113
.L126:
	ldi r25,0
.L50:
	mov r24,r25
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	usbFunctionSetup, .-usbFunctionSetup
	.section	.text.usbEventResetReady,"ax",@progbits
.global	usbEventResetReady
	.type	usbEventResetReady, @function
usbEventResetReady:
	push r13
	push r14
	push r15
	push r16
	push r17
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 7 */
.L__stack_usage = 7
	ldi r16,lo8(8)
	ldi r17,0
	ldi r28,0
	ldi r29,lo8(-128)
.L143:
	mov r13,r29
	add r13,r28
	out 0x31,r13
	rcall usbMeasureFrameLength
	movw r14,r24
	ldi r24,52
	cp r14,r24
	ldi r24,9
	cpc r15,r24
	brge .L142
	mov r28,r13
.L142:
	lsr r29
	subi r16,1
	sbc r17,__zero_reg__
	brne .L143
	ldi r24,lo8(-1)
	add r24,r28
	out 0x31,r24
	mov r16,r28
	ldi r17,0
	subi r16,-1
	sbci r17,-1
.L144:
	in r18,0x31
	ldi r19,0
	cp r16,r18
	cpc r17,r19
	brlt .L149
	rcall usbMeasureFrameLength
	subi r24,52
	sbci r25,9
	sbrs r25,7
	rjmp .L145
	neg r25
	neg r24
	sbc r25,__zero_reg__
.L145:
	cp r24,r14
	cpc r25,r15
	brge .L146
	in r28,0x31
	movw r14,r24
.L146:
	in r24,0x31
	subi r24,lo8(-(1))
	out 0x31,r24
	rjmp .L144
.L149:
	out 0x31,r28
	in r22,0x31
	ldi r24,0
	ldi r25,0
/* epilogue start */
	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	rjmp eeprom_write_byte
	.size	usbEventResetReady, .-usbEventResetReady
	.section	.text.I2C_WriteBit,"ax",@progbits
.global	I2C_WriteBit
	.type	I2C_WriteBit, @function
I2C_WriteBit:
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	mov r29,r24
	tst r24
	breq .L151
	cbi 0x17,0
	rjmp .L152
.L151:
	sbi 0x17,0
.L152:
	ldi r28,0
.L153:
	mov r18,r28
	ldi r19,0
	lds r24,I2C_DELAY
	lds r25,I2C_DELAY+1
	cp r18,r24
	cpc r19,r25
	brsh .L165
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L153
.L165:
	cbi 0x17,2
	ldi r28,0
.L155:
	mov r18,r28
	ldi r19,0
	lds r24,I2C_DELAY
	lds r25,I2C_DELAY+1
	cp r18,r24
	cpc r19,r25
	brsh .L166
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L155
.L166:
	sbi 0x17,2
	ldi r28,0
.L157:
	mov r18,r28
	ldi r19,0
	lds r24,I2C_DELAY
	lds r25,I2C_DELAY+1
	cp r18,r24
	cpc r19,r25
	brsh .L167
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L157
.L167:
	cpse r29,__zero_reg__
	sbi 0x17,0
.L159:
	ldi r28,0
.L160:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L168
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L160
.L168:
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	I2C_WriteBit, .-I2C_WriteBit
	.section	.text.I2C_ReadBit,"ax",@progbits
.global	I2C_ReadBit
	.type	I2C_ReadBit, @function
I2C_ReadBit:
	push r16
	push r17
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 4 */
.L__stack_usage = 4
	cbi 0x17,0
	ldi r28,0
.L170:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L178
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L170
.L178:
	cbi 0x17,2
	ldi r28,0
.L172:
	lds r16,I2C_DELAY
	lds r17,I2C_DELAY+1
	mov r24,r28
	ldi r25,0
	cp r24,r16
	cpc r25,r17
	brsh .L179
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L172
.L179:
	in r29,0x16
	ldi r28,0
.L174:
	mov r24,r28
	ldi r25,0
	cp r24,r16
	cpc r25,r17
	brsh .L180
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L174
.L180:
	sbi 0x17,2
	ldi r28,0
.L176:
	mov r18,r28
	ldi r19,0
	lds r24,I2C_DELAY
	lds r25,I2C_DELAY+1
	cp r18,r24
	cpc r19,r25
	brsh .L181
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L176
.L181:
	mov r24,r29
	andi r24,lo8(1)
/* epilogue start */
	pop r29
	pop r28
	pop r17
	pop r16
	ret
	.size	I2C_ReadBit, .-I2C_ReadBit
	.section	.text.I2C_Init,"ax",@progbits
.global	I2C_Init
	.type	I2C_Init, @function
I2C_Init:
	push r28
/* prologue: function */
/* frame size = 0 */
/* stack size = 1 */
.L__stack_usage = 1
	in r24,0x18
	andi r24,lo8(-6)
	out 0x18,r24
	cbi 0x17,2
	cbi 0x17,0
	ldi r28,0
.L183:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L185
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L183
.L185:
/* epilogue start */
	pop r28
	ret
	.size	I2C_Init, .-I2C_Init
	.section	.text.I2C_Start,"ax",@progbits
.global	I2C_Start
	.type	I2C_Start, @function
I2C_Start:
	push r28
/* prologue: function */
/* frame size = 0 */
/* stack size = 1 */
.L__stack_usage = 1
	in r24,0x17
	andi r24,lo8(-6)
	out 0x17,r24
	ldi r28,0
.L187:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L193
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L187
.L193:
	sbi 0x17,0
	ldi r28,0
.L189:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L194
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L189
.L194:
	sbi 0x17,2
	ldi r28,0
.L191:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L195
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L191
.L195:
/* epilogue start */
	pop r28
	ret
	.size	I2C_Start, .-I2C_Start
	.section	.text.I2C_Stop,"ax",@progbits
.global	I2C_Stop
	.type	I2C_Stop, @function
I2C_Stop:
	push r28
/* prologue: function */
/* frame size = 0 */
/* stack size = 1 */
.L__stack_usage = 1
	sbi 0x17,0
	ldi r28,0
.L197:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L205
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L197
.L205:
	sbi 0x17,2
	ldi r28,0
.L199:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L206
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L199
.L206:
	cbi 0x17,2
	ldi r28,0
.L201:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L207
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L201
.L207:
	cbi 0x17,0
	ldi r28,0
.L203:
	mov r24,r28
	ldi r25,0
	lds r18,I2C_DELAY
	lds r19,I2C_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L208
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r28,lo8(-(1))
	rjmp .L203
.L208:
/* epilogue start */
	pop r28
	ret
	.size	I2C_Stop, .-I2C_Stop
	.section	.text.I2C_Write,"ax",@progbits
.global	I2C_Write
	.type	I2C_Write, @function
I2C_Write:
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	mov r29,r24
	ldi r28,lo8(8)
.L210:
	mov r24,r29
	andi r24,lo8(-128)
	rcall I2C_WriteBit
	lsl r29
	subi r28,lo8(-(-1))
	brne .L210
/* epilogue start */
	pop r29
	pop r28
	rjmp I2C_ReadBit
	.size	I2C_Write, .-I2C_Write
	.section	.text.I2C_Read,"ax",@progbits
.global	I2C_Read
	.type	I2C_Read, @function
I2C_Read:
	push r15
	push r16
	push r17
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 5 */
.L__stack_usage = 5
	mov r15,r24
	ldi r29,lo8(8)
	ldi r28,0
.L213:
	lsl r28
	rcall I2C_ReadBit
	or r28,r24
	subi r29,lo8(-(-1))
	brne .L213
.L214:
	lds r16,I2C_DELAY
	lds r17,I2C_DELAY+1
	mov r24,r29
	ldi r25,0
	cp r24,r16
	cpc r25,r17
	brsh .L222
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r29,lo8(-(1))
	rjmp .L214
.L222:
	tst r15
	breq .L216
	ldi r24,0
	rjmp .L221
.L216:
	ldi r24,lo8(1)
.L221:
	rcall I2C_WriteBit
	ldi r29,0
.L218:
	mov r24,r29
	ldi r25,0
	cp r24,r16
	cpc r25,r17
	brsh .L223
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	subi r29,lo8(-(1))
	rjmp .L218
.L223:
	mov r24,r28
/* epilogue start */
	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	ret
	.size	I2C_Read, .-I2C_Read
	.section	.text.dwCaptureWidths,"ax",@progbits
.global	dwCaptureWidths
	.type	dwCaptureWidths, @function
dwCaptureWidths:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
/* #APP */
 ;  1299 "main.c" 1
	                                                                                    
;       Measure pulse widths and store in dwBuf                                     
                                                                                    
        ldi   r26,lo8(dwBuf)  ; X register addresses dwBuf                          
        ldi   r27,hi8(dwBuf)                                                        
        clr   r24             ; Counter for number of measured pulses               
                                                                                    
        cli                   ; Disable interrupts while measuring pulse widths     
        sbi   0x18,5          ; End break                                           
        cbi   0x17,5          ; Set DDRB 5 (reset/dwire) direction input            
                                                                                    
;       Cycle loop - track a high pulse followed by a low pulse                     
                                                                                    
dcw2:   clr   r30             ; 1.                                                  
        clr   r31             ; 1.                                                  
                                                                                    
;       Loop while pin high                                                         
                                                                                    
dwc4:   adiw  r30,1           ; 2.                                                  
        brcs  dwc8            ; 1/2. If no change in 65536 loops (~24ms)            
                                                                                    
        sbic  0x16,5          ; 1/2. Skip if Pin PB5 clear (space, zero)            
        rjmp  dwc4            ; 2.                                                  
                                                                                    
        cpi   r24,64          ; 1.   Limit measurement to space available in dwBuf. 
                              ;      Logically this test would be more              
                              ;      sensible at the start of the main dcw2         
                              ;      loop. It is here instead using 2 cycles at     
                              ;      the end of a low pulse in order to balance     
                              ;      the 2 cycles taken by the 'rjmp dcw2' at       
                              ;      the end of the high pulse.                     
                                                                                    
        brsh  dwc8            ; 1.   If dwBuf full                                  
                                                                                    
        st    x+,r30          ; 2.   Record high pulse time                         
        st    x+,r31          ; 2.                                                  
        inc   r24             ; 1.                                                  
                                                                                    
        clr   r30             ; 1.                                                  
        clr   r31             ; 1.                                                  
                                                                                    
;       Loop while pin low                                                          
                                                                                    
dwc6:   adiw  r30,1           ; 2.                                                  
        brcs  dwc8            ; 1/2. If no change in 65536 loops (~24ms)            
                                                                                    
        sbis  0x16,5          ; 1/2. Skip if Pin PB5 set (mark, one)                
        rjmp  dwc6            ; 2.                                                  
                                                                                    
        st    x+,r30          ; 2.   Record low pulse time                          
        st    x+,r31          ; 2.                                                  
        inc   r24             ; 1.                                                  
                                                                                    
        rjmp  dcw2            ; 2.   Back to start of cycle loop                    
                                                                                    
                                                                                    
;       Measurement complete                                                        
                                                                                    
dwc8:   sei                   ; Re-enable interrupts                                
        add   r24,r24         ; Convert word count to byte count                    
        sts   dwLen,r24                                                             

 ;  0 "" 2
/* #NOAPP */
	ret
	.size	dwCaptureWidths, .-dwCaptureWidths
	.section	.text.dwSendBytes,"ax",@progbits
.global	dwSendBytes
	.type	dwSendBytes, @function
dwSendBytes:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
/* #APP */
 ;  1383 "main.c" 1
	                                                                    
;       Transmit dwLen bytes from dwBuf                             
;                                                                   
;       Register usage:                                             
;                                                                   
;       r21     - Remaining bit count to be sent in this byte       
;       r22     - current byte being transmitted (shifted)          
;       r23     - remaining byte count to be transmitted            
;       r25:r24 - bit time as iteration count                       
;       r27:r26 - current buffer address (x)                        
;       r31:r30 - bit time counter                                  
                                                                    
;       Start with dwire pin as output in idle state                
                                                                    
        sbi   0x18,5          ; Make sure line is idle (high)       
        sbi   0x17,5          ; DDRB pin5 is output                 
                                                                    
;       Preload registers                                           
                                                                    
        lds   r24,dwBitTime                                         
        lds   r25,dwBitTime+1                                       
        ldi   r26,lo8(dwBuf)  ; X register addresses dwBuf          
        ldi   r27,hi8(dwBuf)                                        
                                                                    
        lds   r23,dwLen                                             
        tst   r23                                                   
        breq  dws12           ; If no bytes to transmit             
                                                                    
;       Disable interrupts during transmission                      
                                                                    
        cli                                                         
                                                                    
;       Loop for each byte                                          
                                                                    
;       Send start bit (space / 0)                                  
                                                                    
dws2:   movw  r30,r24         ;      Load wait count to r31:r30     
        cbi   0x18,5          ; 1.   Set dwire port low             
                                                                    
        ld    r22,x+          ; 2.   Next byte to send              
        ldi   r21,8           ; 1.   Bit count                      
                                                                    
dws4:   sbiw  r30,1           ; 2.   Decrement wait count           
        brne  dws4            ; 1/2. While more waiting required    
                                                                    
;       Loop for each bit                                           
                                                                    
;       Each bit takes a total of 4*dwBitTime + 8 cycles.           
                                                                    
dws6:   sbrc  r22,0           ; 1/2. Skip if sending zero           
        sbi   0x18,5          ; 1.   Set dwire port high            
        sbrs  r22,0           ; 1/2. Skip if sending one            
        cbi   0x18,5          ; 1.   Set dwire port low             
        movw  r30,r24         ; 1.   Load wait count to r31:r30     
                                                                    
;       5 cycles from dws6 to here.                                 
                                                                    
dws8:   sbiw  r30,1           ; 2.   Decrement wait count           
        brne  dws8            ; 1/2. While more waiting required    
                                                                    
;       4*dwBitTime+4 cycles from dws6 to here.                     
                                                                    
        lsr   r22             ; 1.   Shift next bit to bit 0        
        dec   r21             ; 1.   Count transmitted bit          
        brne  dws6            ; 1/2. While more bits to transmit    
                                                                    
;       Send stop bit (mark / 1)                                    
                                                                    
        movw  r30,r24         ; 1.   Load wait count to r31:r30     
        sbi   0x18,5          ; 1.   Set dwire port high            
        adiw  r30,1           ; 2.   Extra iteration                
                                                                    
dws10:  sbiw  r30,1           ; 2.   Decrement wait count           
        brne  dws10           ; 1/2. While more waiting required    
                                                                    
        dec   r23                                                   
        brne  dws2            ; While more bytes to transmit        
                                                                    
dws12:  sei                   ; Reenable interrupts                 
        cbi   0x17,5          ; DDRB pin5 is input                  
                                                                    

 ;  0 "" 2
/* #NOAPP */
	ret
	.size	dwSendBytes, .-dwSendBytes
	.section	.text.dwReadBytes,"ax",@progbits
.global	dwReadBytes
	.type	dwReadBytes, @function
dwReadBytes:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
/* #APP */
 ;  1472 "main.c" 1
	                                                                    
;       Register usage:                                             
;                                                                   
;       r21     - Remaining bit count to be read in this byte       
;       r22     - current byte being received (shifted)             
;       r23     - total bytes received so far                       
;       r25:r24 - bit time as iteration count                       
;       r27:r26 - current buffer address (x)                        
;       r31:r30 - bit time counter                                  
                                                                    
;       Preload registers                                           
                                                                    
        clr   r23                                                   
        lds   r24,dwBitTime                                         
        lds   r25,dwBitTime+1                                       
        ldi   r26,lo8(dwBuf)  ; X register addresses dwBuf          
        ldi   r27,hi8(dwBuf)                                        
                                                                    
;       Disable interrupts during reception                         
                                                                    
        cli                                                         
        cbi   0x17,5          ; DDRB pin5 is input                  
                                                                    
;       Wait up to 65536*6 cycles = 23ms for start bit              
                                                                    
dwr2:   clr   r30                                                   
        clr   r31                                                   
                                                                    
dwr4:   sbiw  r30,1           ; 2.   Check for timeout              
        breq  dwr14           ; 1/2. If no start bit encountered    
                                                                    
        sbic  0x16,5          ; 1/2. Skip if Pin PB5 clear          
        rjmp  dwr4            ; 2.   While no start bit             
                                                                    
;       Wait through half of start bit                              
                                                                    
        movw  r30,r24         ; 1                                   
        lsr   r31             ; 1                                   
        ror   r30             ; 1                                   
                                                                    
dwr6:   sbiw  r30,1           ; 2.                                  
        brne  dwr6            ; 1/2.                                
                                                                    
;       We should be half way through the start bit, check this.    
;       If line not still low, assume it was a glitch and go        
;       back to waiting for a start bit.                            
                                                                    
        sbic  0x16,5          ; 1/2. Skip if Pin PB5 clear          
        rjmp  dwr2            ; 2.   If not a real start bit        
                                                                    
;       Loop for 8 bits sampling the middle of each pulse.          
                                                                    
        clr   r22                                                   
        ldi   r21,8                                                 
                                                                    
;       Each iteration takes 4*(r25:r24) + 8 cycles.                
                                                                    
dwr8:   movw  r30,r24         ; 1.                                  
                                                                    
dwr10:  sbiw  r30,1           ; 2.                                  
        brne  dwr10           ; 1/2.                                
                                                                    
;       4*(r25:r24) cycles to here                                  
                                                                    
;       Sample one bit to top of r22                                
                                                                    
        lsr   r22             ; 1.                                  
        sbic  0x16,5          ; 1/2.                                
        sbr   r22,128         ; 1.   Sets top bit of r22            
        rjmp  .               ; 2.   2 cycle noop                   
        dec   r21             ; 1.                                  
        brne  dwr8            ; 1/2. While more bits to sample      
                                                                    
;       Add sampled byte to buffer                                  
                                                                    
        st    x+,r22                                                
        inc   r23                                                   
                                                                    
;       Wait for line to go idle                                    
                                                                    
        clr   r30                                                   
        clr   r31                                                   
                                                                    
dwr12:  sbiw  r30,1           ; 2.   Check for timeout              
        breq  dwr14           ; 1/2. If line stuck low              
                                                                    
        sbis  0x16,5          ; 1/2. Skip if Pin PB5 set            
        rjmp  dwr12           ; 2.   While no start bit             
                                                                    
;       Check for buffer full and loop back to read next byte       
                                                                    
        cpi   r23,128                                               
        brlo  dwr2            ; While buffer not full               
                                                                    
;       Read complete                                               
                                                                    
dwr14:  sei                   ; Re-enable interrupts                
        sts   dwLen,r23                                             

 ;  0 "" 2
/* #NOAPP */
	ret
	.size	dwReadBytes, .-dwReadBytes
	.section	.text.startup.main,"ax",@progbits
.global	main
	.type	main, @function
main:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	ldi r24,lo8(32)
	out 0x17,r24
	out 0x18,r24
	ldi r24,0
	ldi r25,0
	rcall eeprom_read_byte
	cpi r24,lo8(-1)
	breq .L228
	out 0x31,r24
.L228:
	lds r16,EE_addr
	lds r17,EE_addr+1
	movw r24,r16
	rcall eeprom_read_byte
	mov r14,r24
	movw r24,r16
	adiw r24,1
	rcall eeprom_read_byte
	mov r15,r24
	movw r24,r16
	adiw r24,2
	rcall eeprom_read_byte
	mov r25,r24
	ldi r24,lo8(-48)
	add r24,r14
	cpi r24,lo8(10)
	brlo .+2
	rjmp .L229
	ldi r24,lo8(-48)
	add r24,r15
	cpi r24,lo8(10)
	brlo .+2
	rjmp .L229
	ldi r24,lo8(-48)
	add r24,r25
	cpi r24,lo8(10)
	brlo .+2
	rjmp .L229
.L290:
	lds r16,EE_addr
	lds r17,EE_addr+1
	movw r24,r16
	rcall eeprom_read_byte
	ldi r25,0
	sts usbDescriptorStringSerialNumber+2+1,r25
	sts usbDescriptorStringSerialNumber+2,r24
	movw r24,r16
	adiw r24,1
	rcall eeprom_read_byte
	ldi r25,0
	sts usbDescriptorStringSerialNumber+4+1,r25
	sts usbDescriptorStringSerialNumber+4,r24
	movw r24,r16
	adiw r24,2
	rcall eeprom_read_byte
	ldi r25,0
	sts usbDescriptorStringSerialNumber+6+1,r25
	sts usbDescriptorStringSerialNumber+6,r24
	sbi 0x17,3
	ldi r17,lo8(20)
.L230:
	ldi r24,lo8(15)
	ldi r25,0
	rcall _delay_ms
	subi r17,lo8(-(-1))
	brne .L230
	cbi 0x17,3
	ldi r24,lo8(5)
	ldi r25,0
	sts I2C_DELAY+1,r25
	sts I2C_DELAY,r24
	sts SPI_DELAY+1,__zero_reg__
	sts SPI_DELAY,__zero_reg__
	ldi r25,lo8(14)
	ldi r24,lo8(24)
/* #APP */
 ;  456 "d:\projects\avr\avr8-gnu-toolchain\avr\include\avr\wdt.h" 1
	in __tmp_reg__,__SREG__
	cli
	wdr
	out 33, r24
	out __SREG__,__tmp_reg__
	out 33, r25
 	
 ;  0 "" 2
/* #NOAPP */
	sts cmp0,__zero_reg__
	sts cmp1,__zero_reg__
	sts cmp2,__zero_reg__
	sts compare0,__zero_reg__
	sts compare1,__zero_reg__
	sts compare2,__zero_reg__
	sts counter,__zero_reg__
	sts jobState,__zero_reg__
	rcall usbInit
/* #APP */
 ;  1614 "main.c" 1
	sei
 ;  0 "" 2
/* #NOAPP */
	cbi 0x17,0
	cbi 0x18,0
	cbi 0x17,1
	cbi 0x18,1
	cbi 0x17,2
	cbi 0x18,2
	cbi 0x17,5
	cbi 0x18,5
	ldi r17,lo8(26)
	ldi r16,lo8(64)
	ldi r18,lo8(27)
	mov r15,r18
	ldi r19,lo8(-128)
	mov r14,r19
.L281:
/* #APP */
 ;  1627 "main.c" 1
	wdr
 ;  0 "" 2
/* #NOAPP */
	rcall usbPoll
	lds r30,jobState
	mov r24,r30
	ldi r25,0
	cpi r24,21
	cpc r25,__zero_reg__
	brlo .+2
	rjmp .L231
	movw r30,r24
	subi r30,lo8(-(gs(.L233)))
	sbci r31,hi8(-(gs(.L233)))
	ijmp
	.section	.progmem.gcc_sw_table.main,"ax",@progbits
	.p2align	1
.L233:
	rjmp .L232
	rjmp .L234
	rjmp .L235
	rjmp .L236
	rjmp .L237
	rjmp .L238
	rjmp .L239
	rjmp .L240
	rjmp .L241
	rjmp .L242
	rjmp .L291
	rjmp .L244
	rjmp .L231
	rjmp .L231
	rjmp .L231
	rjmp .L231
	rjmp .L231
	rjmp .L245
	rjmp .L231
	rjmp .L231
	rjmp .L246
	.section	.text.startup.main
.L234:
	sbi 0x17,0
	cbi 0x17,1
	sbi 0x17,2
	sbi 0x18,2
	sts sendBuffer,__zero_reg__
	lds r24,rxBuffer
	sts t,r24
	sts q,r14
.L247:
	lds r24,q
	tst r24
	brne .+2
	rjmp .L326
	cbi 0x18,0
	mov r13,__zero_reg__
.L248:
	mov r24,r13
	ldi r25,0
	lds r18,SPI_DELAY
	lds r19,SPI_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L328
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	inc r13
	rjmp .L248
.L328:
	lds r25,q
	lds r24,t
	and r24,r25
	breq .L250
	sbi 0x18,0
.L250:
	cbi 0x18,2
	lds r24,sendBuffer
	lsl r24
	sts sendBuffer,r24
	in r25,0x16
	lds r18,sendBuffer
	bst r25,1
	clr r24
	bld r24,0
	add r24,r18
	sts sendBuffer,r24
	mov r13,__zero_reg__
.L251:
	mov r24,r13
	ldi r25,0
	lds r18,SPI_DELAY
	lds r19,SPI_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L329
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	inc r13
	rjmp .L251
.L329:
	sbi 0x18,2
	lds r24,q
	lsr r24
	sts q,r24
	rjmp .L247
.L235:
	sbi 0x17,2
	cbi 0x18,2
	ldi r24,lo8(-32)
	ldi r25,lo8(1)
	rcall _delay_us
	in r13,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	sbi 0x18,2
	ldi r24,lo8(70)
	ldi r25,0
	rcall _delay_us
	cbi 0x17,2
	in r24,0x16
	bst r24,2
	clr r24
	bld r24,0
	mov r28,r24
	ldi r18,0
	mov r29,r18
	movw r24,r28
	ldi r20,1
	eor r24,r20
	sts sendBuffer,r24
	out __SREG__,r13
	ldi r24,lo8(-102)
	ldi r25,lo8(1)
	rcall _delay_us
	sbi 0x17,2
	sbi 0x18,2
.L326:
	ldi r24,lo8(1)
	rjmp .L324
.L236:
	lds r24,rxBuffer
	sts q,r24
	sbi 0x17,2
	ldi r25,lo8(8)
	mov r13,r25
.L256:
	in r12,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	cbi 0x18,2
	lds r24,q
	sbrs r24,0
	rjmp .L254
	ldi r24,lo8(6)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(64)
	ldi r25,0
	rjmp .L321
.L254:
	ldi r24,lo8(60)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(10)
	ldi r25,0
.L321:
	rcall _delay_us
	lds r24,q
	lsr r24
	sts q,r24
	out __SREG__,r12
	dec r13
	cpse r13,__zero_reg__
	rjmp .L256
	rjmp .L231
.L237:
	sts sendBuffer,__zero_reg__
	ldi r24,lo8(8)
	mov r13,r24
.L258:
	lds r24,sendBuffer
	lsr r24
	sts sendBuffer,r24
	sbi 0x17,2
	in r12,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	cbi 0x18,2
	ldi r24,lo8(6)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(10)
	ldi r25,0
	rcall _delay_us
	cbi 0x17,2
	in r24,0x16
	bst r24,2
	clr r24
	bld r24,0
	sts q,r24
	out __SREG__,r12
	ldi r24,lo8(55)
	ldi r25,0
	rcall _delay_us
	lds r24,q
	tst r24
	breq .L257
	lds r24,sendBuffer
	ori r24,lo8(-128)
	sts sendBuffer,r24
.L257:
	dec r13
	cpse r13,__zero_reg__
	rjmp .L258
	rjmp .L326
.L238:
	sbi 0x17,2
	in r13,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	cbi 0x18,2
	ldi r24,lo8(6)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(10)
	ldi r25,0
	rcall _delay_us
	cbi 0x17,2
	in r24,0x16
	bst r24,2
	clr r24
	bld r24,0
	sts sendBuffer,r24
	out __SREG__,r13
	rjmp .L326
.L239:
	sbi 0x17,2
	in r13,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	cbi 0x18,2
	lds r24,rxBuffer
	sbrs r24,0
	rjmp .L259
	ldi r24,lo8(6)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(64)
	ldi r25,0
	rjmp .L322
.L259:
	ldi r24,lo8(60)
	ldi r25,0
	rcall _delay_us
	sbi 0x18,2
	ldi r24,lo8(10)
	ldi r25,0
.L322:
	rcall _delay_us
.L325:
	out __SREG__,r13
	rjmp .L231
.L240:
	sbi 0x17,1
	cbi 0x17,0
	sbi 0x17,2
	cbi 0x18,2
	sts t,r15
	out 0xd,r17
	lds r24,rxBuffer
	cpse r24,__zero_reg__
	cbi 0x18,5
.L261:
	sts q,__zero_reg__
.L262:
	lds r30,q
	lds r24,rxBuffer+1
	cp r30,r24
	brsh .L330
	ldi r31,0
	subi r30,lo8(-(rxBuffer+2))
	sbci r31,hi8(-(rxBuffer+2))
	ld r24,Z
	out 0xf,r24
	out 0xe,r16
.L265:
	lds r24,t
	out 0xd,r24
	mov r13,__zero_reg__
.L263:
	mov r24,r13
	ldi r25,0
	lds r18,SPI_DELAY
	lds r19,SPI_DELAY+1
	cp r24,r18
	cpc r25,r19
	brsh .L331
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_us
	inc r13
	rjmp .L263
.L331:
	sbis 0xe,6
	rjmp .L265
	lds r24,q
	mov r30,r24
	ldi r31,0
	in r25,0xf
	subi r30,lo8(-(sendBuffer))
	sbci r31,hi8(-(sendBuffer))
	st Z,r25
	subi r24,lo8(-(1))
	sts q,r24
	rjmp .L262
.L330:
	lds r24,rxBuffer
	cpse r24,__zero_reg__
	sbi 0x18,5
.L267:
	lds r24,q
	rjmp .L324
.L241:
	rcall I2C_Init
	rjmp .L231
.L242:
	ldi r24,0
	ldi r25,0
	rcall I2C_Start
	lds r24,rxBuffer
	rcall I2C_Write
	sts q,r24
	rjmp .L327
.L291:
	mov r13,__zero_reg__
.L243:
	lds r24,rxBuffer
	cp r13,r24
	brsh .L332
	mov r30,r13
	ldi r31,0
	subi r30,lo8(-(rxBuffer+2))
	sbci r31,hi8(-(rxBuffer+2))
	ld r24,Z
	rcall I2C_Write
	sts q,r24
	inc r13
	rjmp .L243
.L332:
	lds r24,rxBuffer+1
	cpse r24,__zero_reg__
	rcall I2C_Stop
.L269:
	lds r24,q
.L327:
	sts sendBuffer,r24
	rjmp .L326
.L244:
	lds r24,rxBuffer
	cpse r24,__zero_reg__
	rjmp .L292
	mov r13,__zero_reg__
	rjmp .L271
.L292:
	mov r11,__zero_reg__
.L270:
	mov r18,r11
	ldi r19,0
	lds r24,rxBuffer+1
	ldi r25,0
	sbiw r24,1
	movw r20,r18
	subi r20,lo8(-(sendBuffer))
	sbci r21,hi8(-(sendBuffer))
	movw r12,r20
	cp r18,r24
	cpc r19,r25
	brge .L333
	ldi r24,lo8(1)
	rcall I2C_Read
	movw r30,r12
	st Z,r24
	inc r11
	rjmp .L270
.L333:
	ldi r24,0
	rcall I2C_Read
	movw r30,r12
	st Z,r24
	rjmp .L273
.L271:
	lds r24,rxBuffer+1
	cp r13,r24
	brsh .L273
	mov r10,r13
	mov r11,__zero_reg__
	ldi r24,lo8(1)
	rcall I2C_Read
	movw r30,r10
	subi r30,lo8(-(sendBuffer))
	sbci r31,hi8(-(sendBuffer))
	st Z,r24
	inc r13
	rjmp .L271
.L273:
	lds r24,rxBuffer+2
	cpse r24,__zero_reg__
	rcall I2C_Stop
.L275:
	lds r24,rxBuffer+1
.L324:
	sts sendBuffer+8,r24
	rjmp .L231
.L245:
	ldi r24,lo8(1)
	ldi r25,0
	rcall _delay_ms
	in r13,__SREG__
/* #APP */
 ;  50 "d:\projects\avr\avr8-gnu-toolchain\avr\include\util\atomic.h" 1
	cli
 ;  0 "" 2
/* #NOAPP */
	lds r22,ws2812_ptr
	ldi r23,0
	lds r20,ws2812_mask
	ldi r24,lo8(ws2812_grb)
	ldi r25,hi8(ws2812_grb)
	rcall ws2812_sendarray_mask
	sts ws2812_ptr,__zero_reg__
	rjmp .L325
.L246:
	lds r24,dwState
	cpi r24,lo8(1)
	breq .L277
	brlo .L278
	cpi r24,lo8(2)
	breq .L279
	cpi r24,lo8(3)
	brne .L276
	ldi r24,lo8(2)
	ldi r25,0
	rcall _delay_ms
	rcall dwSendBytes
	rcall dwReadBytes
	rjmp .L276
.L278:
	cbi 0x18,5
	sbi 0x17,5
	ldi r24,lo8(100)
	ldi r25,0
	rcall _delay_ms
	rjmp .L323
.L277:
	lds r24,dwBuf
	sts dwBitTime,r24
	lds r24,dwBuf+1
	sts dwBitTime+1,r24
	rjmp .L276
.L279:
	ldi r24,lo8(2)
	ldi r25,0
	rcall _delay_ms
	rcall dwSendBytes
.L323:
	rcall dwCaptureWidths
.L276:
	sts dwState,__zero_reg__
.L231:
	sts jobState,__zero_reg__
.L232:
	lds r24,softPWM
	tst r24
	brne .+2
	rjmp .L281
	lds r24,counter
	cpse r24,__zero_reg__
	rjmp .L282
	lds r24,compare0
	sts cmp0,r24
	lds r24,compare1
	sts cmp1,r24
	lds r24,compare2
	sts cmp2,r24
	rjmp .L283
.L282:
	lds r25,cmp0
	cp r25,r24
	brsh .L284
	cbi 0x18,0
	rjmp .L285
.L284:
	sbi 0x18,0
.L285:
	lds r25,counter
	lds r24,cmp1
	cp r24,r25
	brsh .L286
	cbi 0x18,1
	rjmp .L287
.L286:
	sbi 0x18,1
.L287:
	lds r25,counter
	lds r24,cmp2
	cp r24,r25
	brsh .L288
	cbi 0x18,2
	rjmp .L283
.L288:
	sbi 0x18,2
.L283:
	lds r24,counter
	subi r24,lo8(-(1))
	sts counter,r24
	rjmp .L281
.L229:
	ldi r22,lo8(53)
	movw r24,r16
	rcall eeprom_write_byte
	lds r24,EE_addr
	lds r25,EE_addr+1
	ldi r22,lo8(49)
	adiw r24,1
	rcall eeprom_write_byte
	lds r24,EE_addr
	lds r25,EE_addr+1
	ldi r22,lo8(50)
	adiw r24,2
	rcall eeprom_write_byte
	rjmp .L290
	.size	main, .-main
	.comm	dwBitTime,2,1
	.comm	dwState,1,1
	.comm	dwIn,1,1
	.comm	dwLen,1,1
	.comm	dwBuf,128,1
	.section	.bss.ws2812_ptr,"aw",@nobits
	.type	ws2812_ptr, @object
	.size	ws2812_ptr, 1
ws2812_ptr:
	.zero	1
	.section	.bss.ws2812_mask,"aw",@nobits
	.type	ws2812_mask, @object
	.size	ws2812_mask, 1
ws2812_mask:
	.zero	1
	.section	.bss.ws2812_grb,"aw",@nobits
	.type	ws2812_grb, @object
	.size	ws2812_grb, 192
ws2812_grb:
	.zero	192
.global	jobState
	.section	.bss.jobState,"aw",@nobits
	.type	jobState, @object
	.size	jobState, 1
jobState:
	.zero	1
	.section	.bss.adcSetting,"aw",@nobits
	.type	adcSetting, @object
	.size	adcSetting, 1
adcSetting:
	.zero	1
	.section	.bss.compare2,"aw",@nobits
	.type	compare2, @object
	.size	compare2, 1
compare2:
	.zero	1
	.section	.bss.compare1,"aw",@nobits
	.type	compare1, @object
	.size	compare1, 1
compare1:
	.zero	1
	.section	.bss.compare0,"aw",@nobits
	.type	compare0, @object
	.size	compare0, 1
compare0:
	.zero	1
	.section	.bss.cmp2,"aw",@nobits
	.type	cmp2, @object
	.size	cmp2, 1
cmp2:
	.zero	1
	.section	.bss.cmp1,"aw",@nobits
	.type	cmp1, @object
	.size	cmp1, 1
cmp1:
	.zero	1
	.section	.bss.cmp0,"aw",@nobits
	.type	cmp0, @object
	.size	cmp0, 1
cmp0:
	.zero	1
	.section	.bss.softPWM,"aw",@nobits
	.type	softPWM, @object
	.size	softPWM, 1
softPWM:
	.zero	1
	.section	.bss.counter,"aw",@nobits
	.type	counter, @object
	.size	counter, 1
counter:
	.zero	1
	.section	.bss.q,"aw",@nobits
	.type	q, @object
	.size	q, 1
q:
	.zero	1
	.section	.bss.t,"aw",@nobits
	.type	t, @object
	.size	t, 1
t:
	.zero	1
	.comm	rxBuffer,8,1
	.comm	sendBuffer,9,1
	.section	.bss.I2C_DELAY,"aw",@nobits
	.type	I2C_DELAY, @object
	.size	I2C_DELAY, 2
I2C_DELAY:
	.zero	2
	.section	.data.SPI_DELAY,"aw",@progbits
	.type	SPI_DELAY, @object
	.size	SPI_DELAY, 2
SPI_DELAY:
	.word	10
	.section	.bss.res,"aw",@nobits
	.type	res, @object
	.size	res, 4
res:
	.zero	4
	.section	.bss.cmd,"aw",@nobits
	.type	cmd, @object
	.size	cmd, 4
cmd:
	.zero	4
	.section	.bss.cmd0,"aw",@nobits
	.type	cmd0, @object
	.size	cmd0, 1
cmd0:
	.zero	1
	.section	.bss.timeout,"aw",@nobits
	.type	timeout, @object
	.size	timeout, 2
timeout:
	.zero	2
	.section	.bss.address,"aw",@nobits
	.type	address, @object
	.size	address, 2
address:
	.zero	2
	.section	.bss.poll2,"aw",@nobits
	.type	poll2, @object
	.size	poll2, 1
poll2:
	.zero	1
	.section	.bss.poll1,"aw",@nobits
	.type	poll1, @object
	.size	poll1, 1
poll1:
	.zero	1
	.section	.data.sck_period,"aw",@progbits
	.type	sck_period, @object
	.size	sck_period, 1
sck_period:
	.byte	10
.global	usbDescriptorStringSerialNumber
	.section	.data.usbDescriptorStringSerialNumber,"aw",@progbits
	.type	usbDescriptorStringSerialNumber, @object
	.size	usbDescriptorStringSerialNumber, 8
usbDescriptorStringSerialNumber:
	.word	776
	.word	53
	.word	49
	.word	50
.global	EE_addr
	.section	.data.EE_addr,"aw",@progbits
	.type	EE_addr, @object
	.size	EE_addr, 2
EE_addr:
	.word	32
.global	LITTLE_WIRE_VERSION
	.section	.rodata.LITTLE_WIRE_VERSION,"a",@progbits
	.type	LITTLE_WIRE_VERSION, @object
	.size	LITTLE_WIRE_VERSION, 1
LITTLE_WIRE_VERSION:
	.byte	19
	.ident	"GCC: (AVR_8_bit_GNU_Toolchain_3.5.4_1709) 4.9.2"
.global __do_copy_data
.global __do_clear_bss
