# B Minor Compiler

This repository contains the source code for a B-Minor language compiler as described by Professor Douglas Thain in his book, [Introduction to Compilers and Language Design](https://www3.nd.edu/~dthain/compilerbook/). This is a toy compiler for a made-up language designed by Professor Thain which handles scanning using Flex, parsing using Bison, typechecking, and code generation in ARMv8 assembly.

The compiler was tested and verified using a Raspberry Pi 4 Model B with 2GB of RAM, and has all base functionality working.

# Usage Instructions

| Command | Description |
| ------- | ----------- |
| `make` | Make the compiler executable |
| `./bminor -scan FILENAME.bminor` | Scan/tokenize B-Minor code |
| `./bminor -parse FILENAME.bminor` | Parse B-Minor code into AST |
| `./bminor -print FILENAME.bminor` | Print back parsed B-Minor code |
| `./bminor -typecheck FILENAME.bminor` | Ensure proper type compatibility of B-Minor code |
| `./bminor -codegen FILENAME.bminor FILENAME.s` | Generate ARMv8 Assembly code |
| `./gcc -g FILENAME.s library.c -o PROGRAM` | Compile generated ARMv8 Assembly into an executable |

Note that each of the above commands is a prerequisite to the command on the next row. Meaning, that if for example you were to execute typechecking, the commands for scanning, parsing, and printing will be ran before typechecking can be ran.
