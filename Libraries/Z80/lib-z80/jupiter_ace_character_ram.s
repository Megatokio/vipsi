


#target bin
; #target was defined to compile the file "jupiter_ace_characterset.bin"
; the .bin file should be #inserted instead of #including this file.



#code CHARACTER_SET,0,$400

; -------------------
; THE 'CHARACTER SET'
; -------------------

	ds	32*8

; $20 - Character: ' '          CHR$(32)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000

; $21 - Character: '!'          CHR$(33)

	db 0
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00000000

; $22 - Character: '"'          CHR$(34)

	db 0
	DEFB    %00100100
	DEFB    %00100100
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000

; $23 - Character: '#'          CHR$(35)

	db 0
	DEFB    %00100100
	DEFB    %01111110
	DEFB    %00100100
	DEFB    %00100100
	DEFB    %01111110
	DEFB    %00100100
	DEFB    %00000000

; $24 - Character: '$'          CHR$(36)

	db 0
	DEFB    %00001000
	DEFB    %00111110
	DEFB    %00101000
	DEFB    %00111110
	DEFB    %00001010
	DEFB    %00111110
	DEFB    %00001000

; $25 - Character: '%'          CHR$(37)

	db 0
	DEFB    %01100010
	DEFB    %01100100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00100110
	DEFB    %01000110
	DEFB    %00000000

; $26 - Character: '&'          CHR$(38)

	db 0
	DEFB    %00010000
	DEFB    %00101000
	DEFB    %00010000
	DEFB    %00101010
	DEFB    %01000100
	DEFB    %00111010
	DEFB    %00000000

; $27 - Character: '''          CHR$(39)

	db 0
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000

; $28 - Character: '('          CHR$(40)

	db 0
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00000100
	DEFB    %00000000

; $29 - Character: ')'          CHR$(42)

	db 0
	DEFB    %00100000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00100000
	DEFB    %00000000

; $2A - Character: '*'          CHR$(42)

	db 0
	DEFB    %00000000
	DEFB    %00010100
	DEFB    %00001000
	DEFB    %00111110
	DEFB    %00001000
	DEFB    %00010100
	DEFB    %00000000

; $2B - Character: '+'          CHR$(43)

	db 0
	DEFB    %00000000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00111110
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00000000

; $2C - Character: ','          CHR$(44)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00010000

; $2D - Character: '-'          CHR$(45)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00111110
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000

; $2E - Character: '.'          CHR$(46)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00011000
	DEFB    %00011000
	DEFB    %00000000

; $2F - Character: '/'          CHR$(47)

	db 0
	DEFB    %00000000
	DEFB    %00000010
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00100000
	DEFB    %00000000

; $30 - Character: '0'          CHR$(48)

	db 0
	DEFB    %00111100
	DEFB    %01000110
	DEFB    %01001010
	DEFB    %01010010
	DEFB    %01100010
	DEFB    %00111100
	DEFB    %00000000

; $31 - Character: '1'          CHR$(49)

	db 0
	DEFB    %00011000
	DEFB    %00101000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00111110
	DEFB    %00000000

; $32 - Character: '2'          CHR$(50)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %00000010
	DEFB    %00111100
	DEFB    %01000000
	DEFB    %01111110
	DEFB    %00000000

; $33 - Character: '3'          CHR$(51)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %00001100
	DEFB    %00000010
	DEFB    %01000010
	DEFB    %00111100
	DEFB    %00000000

; $34 - Character: '4'          CHR$(52)

	db 0
	DEFB    %00001000
	DEFB    %00011000
	DEFB    %00101000
	DEFB    %01001000
	DEFB    %01111110
	DEFB    %00001000
	DEFB    %00000000

; $35 - Character: '5'          CHR$(53)

	db 0
	DEFB    %01111110
	DEFB    %01000000
	DEFB    %01111100
	DEFB    %00000010
	DEFB    %01000010
	DEFB    %00111100
	DEFB    %00000000

; $36 - Character: '6'          CHR$(54)

	db 0
	DEFB    %00111100
	DEFB    %01000000
	DEFB    %01111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111100
	DEFB    %00000000

; $37 - Character: '7'          CHR$(55)

	db 0
	DEFB    %01111110
	DEFB    %00000010
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00000000

; $38 - Character: '8'          CHR$(56)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111100
	DEFB    %00000000

; $39 - Character: '9'          CHR$(57)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111110
	DEFB    %00000010
	DEFB    %00111100
	DEFB    %00000000

; $3A - Character: ':'          CHR$(58)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00000000

; $3B - Character: ';'          CHR$(59)

	db 0
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00100000

; $3C - Character: '<'          CHR$(60)

	db 0
	DEFB    %00000000
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00001000
	DEFB    %00000100
	DEFB    %00000000

; $3D - Character: '='          CHR$(61)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00111110
	DEFB    %00000000
	DEFB    %00111110
	DEFB    %00000000
	DEFB    %00000000

; $3E - Character: '>'          CHR$(62)

	db 0
	DEFB    %00000000
	DEFB    %00010000
	DEFB    %00001000
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00000000

; $3F - Character: '?'          CHR$(63)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00000000
	DEFB    %00001000
	db 0

; $40 - Character: '@'          CHR$(64)

	db 0
	DEFB    %00111100
	DEFB    %01001010
	DEFB    %01010110
	DEFB    %01011110
	DEFB    %01000000
	DEFB    %00111100
	db 0

; $41 - Character: 'A'          CHR$(65)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01111110
	DEFB    %01000010
	DEFB    %01000010
	db 0

; $42 - Character: 'B'          CHR$(66)

	db 0
	DEFB    %01111100
	DEFB    %01000010
	DEFB    %01111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01111100
	db 0

; $43 - Character: 'C'          CHR$(67)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01000010
	DEFB    %00111100
	db 0

; $44 - Character: 'D'          CHR$(68)

	db 0
	DEFB    %01111000
	DEFB    %01000100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000100
	DEFB    %01111000
	db 0

; $45 - Character: 'E'          CHR$(69)

	db 0
	DEFB    %01111110
	DEFB    %01000000
	DEFB    %01111100
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01111110
	db 0

; $46 - Character: 'F'          CHR$(70)

	db 0
	DEFB    %01111110
	DEFB    %01000000
	DEFB    %01111100
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01000000
	db 0

; $47 - Character: 'G'          CHR$(71)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000000
	DEFB    %01001110
	DEFB    %01000010
	DEFB    %00111100
	db 0

; $48 - Character: 'H'          CHR$(72)

	db 0
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01111110
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	db 0

; $49 - Character: 'I'          CHR$(73)

	db 0
	DEFB    %00111110
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00111110
	db 0

; $4A - Character: 'J'          CHR$(74)

	db 0
	DEFB    %00000010
	DEFB    %00000010
	DEFB    %00000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111100
	db 0

; $4B - Character: 'K'          CHR$(75)

	db 0
	DEFB    %01000100
	DEFB    %01001000
	DEFB    %01110000
	DEFB    %01001000
	DEFB    %01000100
	DEFB    %01000010
	db 0

; $4C - Character: 'L'          CHR$(76)

	db 0
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01111110
	db 0

; $4D - Character: 'M'          CHR$(77)

	db 0
	DEFB    %01000010
	DEFB    %01100110
	DEFB    %01011010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	db 0

; $4E - Character: 'N'          CHR$(78)

	db 0
	DEFB    %01000010
	DEFB    %01100010
	DEFB    %01010010
	DEFB    %01001010
	DEFB    %01000110
	DEFB    %01000010
	db 0

; $4F - Character: 'O'          CHR$(79)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111100
	db 0

; $50 - Character: 'P'          CHR$(80)

	db 0
	DEFB    %01111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01111100
	DEFB    %01000000
	DEFB    %01000000
	db 0

; $51 - Character: 'Q'          CHR$(81)

	db 0
	DEFB    %00111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01010010
	DEFB    %01001010
	DEFB    %00111100
	db 0

; $52 - Character: 'R'          CHR$(82)

	db 0
	DEFB    %01111100
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01111100
	DEFB    %01000100
	DEFB    %01000010
	db 0

; $53 - Character: 'S'          CHR$(83)

	db 0
	DEFB    %00111100
	DEFB    %01000000
	DEFB    %00111100
	DEFB    %00000010
	DEFB    %01000010
	DEFB    %00111100
	db 0

; $54 - Character: 'T'          CHR$(84)

	db 0
	DEFB    %11111110
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	db 0

; $55 - Character: 'U'          CHR$(85)

	db 0
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00111110
	db 0

; $56 - Character: 'V'          CHR$(86)

	db 0
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %00100100
	DEFB    %00011000
	db 0

; $57 - Character: 'W'          CHR$(87)

	db 0
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01000010
	DEFB    %01011010
	DEFB    %00100100
	db 0

; $58 - Character: 'X'          CHR$(88)

	db 0
	DEFB    %01000010
	DEFB    %00100100
	DEFB    %00011000
	DEFB    %00011000
	DEFB    %00100100
	DEFB    %01000010
	db 0

; $59 - Character: 'Y'          CHR$(89)

	db 0
	DEFB    %10000010
	DEFB    %01000100
	DEFB    %00101000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	db 0

; $5A - Character: 'Z'          CHR$(90)

	db 0
	DEFB    %01111110
	DEFB    %00000100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00100000
	DEFB    %01111110
	db 0

; $5B - Character: '['          CHR$(91)

	db 0
	DEFB    %00001110
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001110
	db 0

; $5C - Character: '\'          CHR$(92)

	db 0
	DEFB    %00000000
	DEFB    %01000000
	DEFB    %00100000
	DEFB    %00010000
	DEFB    %00001000
	DEFB    %00000100
	db 0

; $5D - Character: ']'          CHR$(93)

	db 0
	DEFB    %01110000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %01110000
	db 0

; $5E - Character: '^'          CHR$(94)

	db 0
	DEFB    %00010000
	DEFB    %00111000
	DEFB    %01010100
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	db 0

; $5F - Character: '_'          CHR$(95)

	db 0
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %11111111

; $60 - Character:  £           CHR$(96)

	db 0
	DEFB    %00011100
	DEFB    %00100010
	DEFB    %01111000
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %01111110
	DEFB    %00000000

; $61 - Character: 'a'          CHR$(97)

	db 0
	DEFB    %00000000
	DEFB    %00111000
	DEFB    %00000100
	DEFB    %00111100
	DEFB    %01000100
	DEFB    %00111110
	DEFB    %00000000

; $62 - Character: 'b'          CHR$(98)

	db 0
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00111100
	DEFB    %00100010
	DEFB    %00100010
	DEFB    %00111100
	DEFB    %00000000

; $63 - Character: 'c'          CHR$(99)

	db 0
	DEFB    %00000000
	DEFB    %00011100
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00011100
	DEFB    %00000000

; $64 - Character: 'd'          CHR$(100)

	db 0
	DEFB    %00000100
	DEFB    %00000100
	DEFB    %00111100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111110
	DEFB    %00000000

; $65 - Character: 'e'          CHR$(101)

	db 0
	DEFB    %00000000
	DEFB    %00111000
	DEFB    %01000100
	DEFB    %01111000
	DEFB    %01000000
	DEFB    %00111100
	DEFB    %00000000

; $66 - Character: 'f'          CHR$(102)

	db 0
	DEFB    %00001100
	DEFB    %00010000
	DEFB    %00011000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00000000

; $67 - Character: 'g'          CHR$(103)

	db 0
	DEFB    %00000000
	DEFB    %00111100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111100
	DEFB    %00000100
	DEFB    %00111000

; $68 - Character: 'h'          CHR$(104)

	db 0
	DEFB    %01000000
	DEFB    %01000000
	DEFB    %01111000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00000000

; $69 - Character: 'i'          CHR$(105)

	db 0
	DEFB    %00010000
	DEFB    %00000000
	DEFB    %00110000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00111000
	DEFB    %00000000

; $6A - Character: 'j'          CHR$(106)

	db 0
	DEFB    %00000100
	DEFB    %00000000
	DEFB    %00000100
	DEFB    %00000100
	DEFB    %00000100
	DEFB    %00100100
	DEFB    %00011000

; $6B - Character: 'k'          CHR$(107)

	db 0
	DEFB    %00100000
	DEFB    %00101000
	DEFB    %00110000
	DEFB    %00110000
	DEFB    %00101000
	DEFB    %00100100
	DEFB    %00000000

; $6C - Character: 'l'          CHR$(108)

	db 0
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00001100
	DEFB    %00000000

; $6D - Character: 'm'          CHR$(109)

	db 0
	DEFB    %00000000
	DEFB    %01101000
	DEFB    %01010100
	DEFB    %01010100
	DEFB    %01010100
	DEFB    %01010100
	DEFB    %00000000

; $6E - Character: 'n'          CHR$(110)

	db 0
	DEFB    %00000000
	DEFB    %01111000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00000000

; $6F - Character: 'o'          CHR$(111)

	db 0
	DEFB    %00000000
	DEFB    %00111000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111000
	DEFB    %00000000

; $70 - Character: 'p'          CHR$(112)

	db 0
	DEFB    %00000000
	DEFB    %01111000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01111000
	DEFB    %01000000
	DEFB    %01000000

; $71 - Character: 'q'          CHR$(113)

	db 0
	DEFB    %00000000
	DEFB    %00111100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111100
	DEFB    %00000100
	DEFB    %00000110

; $72 - Character: 'r'          CHR$(114)

	db 0
	DEFB    %00000000
	DEFB    %00011100
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00100000
	DEFB    %00000000

; $73 - Character: 's'          CHR$(115)

	db 0
	DEFB    %00000000
	DEFB    %00111000
	DEFB    %01000000
	DEFB    %00111000
	DEFB    %00000100
	DEFB    %01111000
	DEFB    %00000000

; $74 - Character: 't'          CHR$(116)

	db 0
	DEFB    %00010000
	DEFB    %00111000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00010000
	DEFB    %00001100
	DEFB    %00000000

; $75 - Character: 'u'          CHR$(117)

	db 0
	DEFB    %00000000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111100
	DEFB    %00000000

; $76 - Character: 'v'          CHR$(118)

	db 0
	DEFB    %00000000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00101000
	DEFB    %00101000
	DEFB    %00010000
	DEFB    %00000000

; $77 - Character: 'w'          CHR$(119)

	db 0
	DEFB    %00000000
	DEFB    %01000100
	DEFB    %01010100
	DEFB    %01010100
	DEFB    %01010100
	DEFB    %00101000
	DEFB    %00000000

; $78 - Character: 'x'          CHR$(120)

	db 0
	DEFB    %00000000
	DEFB    %01000100
	DEFB    %00101000
	DEFB    %00010000
	DEFB    %00101000
	DEFB    %01000100
	DEFB    %00000000

; $79 - Character: 'y'          CHR$(121)

	db 0
	DEFB    %00000000
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %01000100
	DEFB    %00111100
	DEFB    %00000100
	DEFB    %00111000

; $7A - Character: 'z'          CHR$(122)

	db 0
	DEFB    %00000000
	DEFB    %01111100
	DEFB    %00001000
	DEFB    %00010000
	DEFB    %00100000
	DEFB    %01111100
	DEFB    %00000000

; $7B - Character: '{'          CHR$(123)

	db 0
	DEFB    %00001110
	DEFB    %00001000
	DEFB    %00110000
	DEFB    %00110000
	DEFB    %00001000
	DEFB    %00001110
	DEFB    %00000000

; $7C - Character: '|'          CHR$(124)

	db 0
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00001000
	DEFB    %00000000

; $7D - Character: '}'          CHR$(125)

	db 0
	DEFB    %01110000
	DEFB    %00010000
	DEFB    %00001100
	DEFB    %00001100
	DEFB    %00010000
	DEFB    %01110000
	DEFB    %00000000

; $7E - Character: '~'          CHR$(126)

	db 0
	DEFB    %00110010
	DEFB    %01001100
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000
	DEFB    %00000000

; $7F - Character:  ©           CHR$(127)

	DEFB    %00111100
	DEFB    %01000010
	DEFB    %10011001
	DEFB    %10100001
	DEFB    %10100001
	DEFB    %10011001
	DEFB    %01000010
	DEFB    %00111100


#end




