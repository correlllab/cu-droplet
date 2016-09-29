//.section .text
.global LoadR0
LoadR0:
  movw  r0, r16   ; Load R17:R16 into R1:R0. 
  ret 


.section .text
.global LoadZ
LoadZ:
  movw  r30, r16    ; Load R17:R16 into Z. 
  ret 

  .global ReadELPM
  ReadELPM:
  elpm r16, Z
  ret 