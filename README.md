# EDDI Compiler #

The compiler of the EDDI programming language. 

## Building ##

To build the compiler, you need to install the [eddi-commons](http://github.com/wichtounet/eddi-commons/) library. 

Then, you juste have to use cmake to build the compiler : 

    $ git clone git://github.com/wichtounet/eddic.git
    $ cd eddic
    $ cmake .
    $ make

## Usage ##

You can compile an EDDI source file using the compiler easily. For example, with one of the provided sample : 

    $ cd eddic
    $ ./bin/eddic samples/simple.eddi

That will create a simple.v file in the samples folder. You can then run this file using the EDDI virtual machine ([eddivm](http://github.com/wichtounet/eddivm/)). 

## Troubleshooting ##

Please consider using [Github issues tracker](http://github.com/wichtounet/eddic/issues) to submit bug reports or feature requests. You can also contact me via my [website](http://baptiste-wicht.com/). 

## License ##

This project is distributed under the Boost Software License 1.0. See `LICENSE_1_0.txt` for details.
