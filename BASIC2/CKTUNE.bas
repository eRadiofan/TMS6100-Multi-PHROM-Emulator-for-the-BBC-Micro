   10 REM>CKTUNE
   20 MODE 7
   30 PRINT"Utility for setting TMS5220 clock"
   40 PRINT
   50 A%=&EA:Y%=&FF:X%=0:R%=USR(&FFF4)
   60 IF (R% AND &FF00) > 0 PRINT"This progam does not work":PRINT"on a co-processor":END
   70 R%=?&FE40 AND &80
   80 IF R%>0 PRINT"TMS 5220 not fitted":END
   90 PRINT"Adjust to 160 KHz"
  100 PRINT
  110
  120 PROCspSETUP
  130 REM Units are milliseconds
  140 nFrames=33
  150 tExpect=25*nFrames
  160 adj=-1
  170 @%=&0002010A
  180
  190 REPEAT
  200   TIME=0:CALL SPEAK
  210   T=FNgetinterval+adj
  220   REM PRINT ;T;" ms = ";
  230   PRINT ;160*tExpect/T;" KHz"
  240 UNTIL 0
  250
  260 DEF FNgetinterval
  270 us=realus?1*256+?realus
  280 =?realcs*10+us/1000.0
  290
  300 DEF PROCspSETUP
  310 DIM SPEECH% 55
  320 I=0:REPEAT
  330   READ Y$:Y=EVAL("&"+Y$)
  340   IF I=0 AND Y<>&8D END
  350   SPEECH%?I=Y:I=I+1
  360 UNTIL Y=0
  370
  380 tc1l=&FE44
  390 tc1h=&FE45
  400 ifr=&FE4D
  410
  420 DIM CODE% 290
  430 DIM realcs 1
  440 DIM realus 2
  450
  460 FOR N=0 TO 2 STEP 2
  470   P%=CODE%
  480   [OPT N
  490
  500   .starttimer SEI
  510   LDY &283:LDA &291,Y
  520   STA realcs
  530   .retr
  540   LDY tc1h:LDA tc1l
  550   CPY tc1h:BNE retr
  560   STA realus
  570   STY realus+1
  580   CLI:RTS
  590
  600   .stoptimer SEI
  610   .retry LDY tc1h
  620   LDA tc1l
  630   CPY tc1h:BNE retry
  640   STA &70:LDA realus
  650   SEC:SBC &70:STA realus
  660   STY &70:LDA realus+1
  670   SBC &70:STA realus+1
  680   BPL bypd
  690   LDA realus:CLC
  700   ADC #&0F:STA realus
  710   LDA realus+1
  720   ADC #&27:STA realus+1
  730   INC realcs
  740   .bypd
  750   LDY &283:LDA &291,Y
  760   SEC:SBC realcs
  770   STA realcs
  780   CLI:RTS
  790
  800   .delay LDX #10 \ 1ms
  810   .WL1 DEX
  820   BNE WL1
  830   RTS
  840
  850   .send8 LDA #&9F
  860   LDY SPEECH%,X
  870   JSR &FFF4:INX
  880   CPX #(I-1):BEQ s8xit
  890   LDY SPEECH%,X
  900   JSR &FFF4:INX
  910   CPX #(I-1):BEQ s8xit
  920   LDY SPEECH%,X
  930   JSR &FFF4:INX
  940   CPX #(I-1):BEQ s8xit
  950   LDY SPEECH%,X
  960   JSR &FFF4:INX
  970   CPX #(I-1):BEQ s8xit
  980   LDY SPEECH%,X
  990   JSR &FFF4:INX
 1000   CPX #(I-1):BEQ s8xit
 1010   LDY SPEECH%,X
 1020   JSR &FFF4:INX
 1030   CPX #(I-1):BEQ s8xit
 1040   LDY SPEECH%,X
 1050   JSR &FFF4:INX
 1060   CPX #(I-1):BEQ s8xit
 1070   LDY SPEECH%,X
 1080   JSR &FFF4:INX
 1090   .s8xit RTS
 1100
 1110   .SPEAK LDA #&9E:JSR &FFF4
 1120   TYA:BMI SPEAK
 1130   LDY #&60:LDA #&9F:JSR &FFF4
 1140   JSR delay \ X = 0
 1150   LDY SPEECH%,X
 1160   JSR &FFF4:INX
 1170   JSR send8
 1180   STX &70:LDA #&9E
 1190   .spl1 JSR &FFF4
 1200   TYA:BPL spl1 \ Wait for TS flag
 1210   JSR starttimer
 1220   .smore LDX &70:JSR send8
 1230   CPX #(I-1)
 1240   BEQ spwait
 1250   STX &70
 1260   .spl2 JSR delay
 1270   LDA#&9E:JSR&FFF4
 1280   TYA:AND #&40:BEQ spl2
 1290   BNE smore
 1300   .spwait: JSR delay
 1310   LDA #&9E:JSR &FFF4
 1320   TYA:BMI spwait \ Wait for EOS
 1330   JSR stoptimer
 1340   RTS
 1350   ]:NEXT:REM PRINT (P%-CODE%)
 1360 A%=&9F:Y%=&FF:CALL &FFF4
 1370 Y%=&70:CALL &FFF4
 1380 Y%=0:CALL &FFF4
 1390 ENDPROC
 1400
 1410 DATA 8D,F2,DE,DD,93,74
 1420 DATA AA,53,9D,EA,54,A7,3A,D5,A9,4E,75
 1430 DATA AA,53,9D,EA,54,A7,3A,D5,A9,4E,75
 1440 DATA AA,53,9D,EA,54,A7,3A,D5,A9,4E,75
 1450 DATA AA,53,9D,EA,54,A7,3A,D5,A9,4E,75
 1460 DATA AA,53,9D,FA,0
