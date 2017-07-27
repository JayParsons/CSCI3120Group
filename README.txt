------Before starting------
This version of server works only on Mac machine,
or under the compiler in LLVM(possibly), for using
this version of semaphore and pthread.
------Before starting------

-------Possible bugs-------
Segmentation fault: it is possible that the 
program run into a segmentation fault in its
first run. 
Reasons: unknown.
Solution: Rerun the program without recompiling

File name unreadable: there is a low probab-
ility that the server cannot read the correct
file name and save in a mojibake.
Reasons: unknown, but might be a pthread issue but
unable to verify. 
Solution: not affecting the program
-------Possible bugs-------

Steps to run the program:
<server side>
make sws
./sws 38080 <algorithm> <number of threads>

<client side>
python2 hydra.py < TestSuite/test#.in > test#.out


------Acknowledge------
This project(coding part) is done under some
consultations from Computer Science Learning
Center tutors. The consulting part including
design logic, thread creation.
------Acknowledge------
