//.section .text
.global LoadR0
LoadR0:
  movw  r0, r16   
  ret 


.section .text
.global LoadZ
LoadZ:
  movw  r30, r16     
  ret 

  .global ReadELPM
  ReadELPM:
  elpm r16, Z
  ret 