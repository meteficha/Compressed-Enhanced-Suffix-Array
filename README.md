Code disponibilized by:
Daniel Saad and Mauricio Ayala-Rincón

danielsaad88@gmail.com
ayala@unb.br




COMPILATION
===========

In order to compile the source code, just do:

>  $ make

This will generate 3 executables:

1. index - Our index.

2. control - A program that uses libdivsufsort for comparison with our index.

3. textGenerator - A program responsible to generate well formed files.

INPUT FILES
===========

A well formed file, which is a text file that has the last symbol
lexicographically smaller than the others.  For example:

> actagaccat$


EXECUTION
=========

Here we show how to run the programs.

1. index
  To create the index one must execute:

>  $ ./index -create <inputFile> <indexFile>

  To search for ocurrences, one must do:

>  $ ./index -search <indexFile> <inputFile> <patternFile>

2. control

  Same semantic from 1).

3. textGenerator

  To generate a DNA well formed text, one must use:

>  $ textGenerator <textSize>  >  dnaText.txt
