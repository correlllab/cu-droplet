## `printf`
I say it in the DropletAPI document as well, but it's important enough and common enough that I'll be redundant: anytime you use `printf` to format and print variables, you MUST make sure that the sizes of the types given in the format string match the sizes of the variables passed to `printf` for formatting. If you don't do this it can subtly break the stack, and thus break the flow of your program. A bit more detail about this is provided in the appropriate section of the [DropletAPI](documentation/DropletAPI.md).

Just because you didn't notice a bug when you first wrote a print statement does not mean that no bugs are present. This is one of the first things I check when helping people with their code, because of how often it is responsible for problems.

## Periodic Actions
Frequently, users might want to program a Droplet to perform some task -- ie, blink a light, send a message, take a measurement -- periodically. Depending on how small the desired period is, the best approach varies.
If the desired period is, say, less than 50ms, maybe 100ms, it's fine to simply perform the periodic task in `loop()`, and put the appropriate `delayMS` call in there.

For larger periods than this, that approach can be problematic, as any received messages and rnb measurements will be queued up until the `delayMS` call returns. Depending on the volume of messaging and rnb broadcasting, your queues might fill up. If the code which handles messages and rnb measurements takes a little while, then the time for those computations will effectively increase the period of your task.

There are two ways to handle this:
1. Use the scheduler. `schedulePeriodicTask` is basically made for this purpose. However, I have seen some strange erroneous behavior which seems to correlate to when the scheduled tasks make heavy use of interrupts themselves: stuff like ir messaging and rnb broadcasting. I'm not positive there's a bug there, though. If the periodic task is quick and simple, then this approach should work great.
2. Store the last time the periodic task was performed, and check how long it has been since that time in loop. When enough time has elapsed, perform the periodic task and update the stored time. Here is a simple example, which causes the Droplet to perform an rnb broadcast every 5 seconds:
```C
void loop(){
    static uint32_t lastBroadcast = 0;
    if( (getTime()-lastBroadcast) > 5000){
        broadcastRnbData();
        lastBroadcast = getTime();
    }
}
```
Observe that the TDMA-esque implementation with frames and slots used in many of the example programs (primarily, localization and anything which uses it) is just a (bit more advanced) version of this same technique.



## Coming Soon / WIP
