 turtles-own
[ clock        ;; each firefly's clock
  threshold    ;; the clock tick at which a firefly stops its flash
  reset-level  ;; the clock tick a firefly will reset to when it is triggered by other flashing
  window       ;; a firefly can't reset its cycle if (clock <= window)
]

to setup
  clear-all
  set-sefault-shape turtles "circle"  
  crt number
    [ 
      set shape "butterfly"
      set clock random (round cycle-length)
      set threshold flash-length
      ifelse strategy = "delay"
      [ set reset-level threshold
        set window -1 ]
      [ set reset-level 0
        set window (threshold + 1) ]
      set size 2  ;; easier to see
      recolor ]
  reset-ticks
end

to go
  ask turtles [
    move
    increment-clock
    if ( (clock > window) and (clock >= threshold) )
      [ look ]
  ]
  ask turtles [
    recolor
  ]
  tick
end


to recolor ; turtle procedure
  ifelse (clock < threshold)
    [ st
      set color yellow ]
    [ set color gray - 2
      ifelse show-dark-fireflies?
        [ st ]
        [ ht ] ]
end

to move ; turtle procedure
end

to increment-clock ; turtle procedure
  set clock (clock + 1)
  if clock = cycle-length
    [ set clock 0 ]
end

to look ; turtle procedure
  if count turtles in-radius 1 with [color = yellow] >= flashes-to-reset
    [ set clock reset-level ]
end


; Copyright 1997 Uri Wilensky.
; See Info tab for full copyright and license.