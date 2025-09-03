# RTno2Proxy

## how to use with commandline mode
* WARNING Arduino UNO and other AVR Arduino resets every time in connection sequence in MacOS.
* I recommed you to use "interactive mode" for those device reset is launched in file open sequence.



## how to use with interactive mode

```
$ ./rtno2 /dev/cu.usbmodem101 57600 interactive
> getstate
INACTIVE
> activate
ACTIVE
> execute
RESULT::OK
> getprofile
RTnoProfile(inports=[], outports=[Port(TimedLongSeq, stick),Port(TimedLongSeq, button),])
> print button
RESULT::OK([1,1,1,1])
> execute
RESULT::OK
> print button
RESULT::OK([0,1,1,1])
> deactivate
INACTIVE
> exit
```

### 