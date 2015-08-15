I've made a few changes to rtl_ais. There is a hacked version of the library on the net that gives a few extras.
. For the compiling there is now cmake. (Go to the build directory. do #cmake .. and then make.)
. added also rtl_test. this will allow to find the ppm. (rtl_test -p)
. added an option (-w) to enter a bandwidth. default is as narrow as possible.
. for linux the kernelmodule is now auto deattached. So no more blacklisting.
. the library is now compiled into the program. So only one executable file.

TODO
There is also a hack to increase the HF gain. would be nice.
Split the aisdecoder in a few seperated threads. I think this will increase the number of received sentences.
