# EDDI Compiler 1.0.0 #

The compiler of the EDDI programming language. 

## Building ##

A compiler supporting the new standard, C++11, is necessary to build the compiler. The compilation has been tested on GCC 4.6 and greater versions. You need Boost 1.47.0 installed on your computer to build this project. 

You juste have to use cmake to build the compiler : 

    $ git clone git://github.com/wichtounet/eddic.git
    $ cd eddic
    $ cmake .
    $ make

## Usage ##

You can compile an EDDI source file using the compiler easily. For example, with one of the provided sample : 

    $ cd eddic
    $ ./bin/eddic samples/assembly.eddi

That will create a "a.out" file in the current folder. You can then run this file as any other executable on your computer : 

    $ ./a.out

## Contributors ##

Read `AUTHORS`

## Release Notes ##

Read `ChangeLog`

## Contribute ##

The project is open for any kind of contribution : new features, hotfixes, tests, ...

If you want to contribute to this project, you can contact me by [email](baptiste.wicht@gmail.com) or via my [website](http://baptiste-wicht.com/).

If you want to support the development of this project, you [donate via Pledgie](http://pledgie.com/campaigns/16583). Thank you !

## More informations ##

You can find more informations about this project, on the [website](http://baptiste-wicht.com/).

## Troubleshooting ##

Please consider using [Github issues tracker](http://github.com/wichtounet/eddic/issues) to submit bug reports or feature requests. You can also contact me via my [website](http://baptiste-wicht.com/). 

## License ##

This project is distributed under the Boost Software License 1.0. Read `LICENSE` for details.
