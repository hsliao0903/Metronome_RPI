Name: Hsiang-Yuan Liao
UFID: 4353-5341
Email: hs.liao@ufl.edu


Please inform me if there is any problem in this project.
I have tested for several times, it works well.

[CAUTION] Please do not set the BPM too low, or it will have to wait for that long to act blink.

The green LED blink thread is implemented in thread sleep function.
So... if the BPM is set too low, the thread will sleep for that long and doing nothing,
even a new BPM is set.

For example, if the BPM is set to 1.
The green LED will blink once and stay off for 60 seconds.
If a new BPM is added during this 60 seconds, it still has to wait for the whole 60 seconds is up.
I didn't implement the way to wake up sleep "blink" thread.

BPM = 1, 60 seconds for green LED to blink or react to new BPM again.
BPM = 2, 30 seconds
BPM = 10, 6 seconds
