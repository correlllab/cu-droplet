This version of the Droplet's electronics includes fixes to minor problems with the first batch of Audio Droplets, ordered in March 2015.

Those fixes mean that this board is changed slightly, and it's always possible that those changes introduced new bugs in turn.

OUTSTANDING ISSUES:
    Does the Supercap really need to be 1.5F? Almost certainly not, but we haven't experimented with smaller ones, yet.
    It's nice for the Droplet legs to be thicker (and thus sturdier) than normal pins, but it's hard to find thick-but-long
        connectors like this. The pins in the BOM are flattened on the very tip, with the result that it's possible for them
        them to short two adjacent strips together in rare circumstances. This is bad! Depending on the pin alternatives you 
        can find, this may be a good thing to change.

Thus, we can't guarantee that the board, built as presented in this files, will work perfectly. But it's the best/closest-to-working-perfectly that we have.

Note also that this BoM was when we were intending to replace motor 0 with a speaker (using the motor driver as a kludgy audio driver). 

If you want Droplets with three motors instead of a speaker, order one more motor per board. 
You'll also want to make a few simple changes to the code if you go this route. 
Please contact me for assistance once you've gotten this far. (john.klingner@colorado.edu)

Good luck!

-John Klingner

PLANNED CHANGES:
    Replace coin vibration motors with surface-mounted motors. This will render the bottom shells unnecessary and substantially reduce costs.
