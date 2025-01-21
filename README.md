# Quadtree Image Processing Project

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Project Structure](#project-structure)
- [Installation](#installation)
- [Usage](#usage)
- [Examples](#examples)
- [Makefile Details](#makefile-details)
- [Tasks Overview](#tasks-overview)

## Introduction

This project implements an image processing system using quadtree compression, decompression, and mirroring techniques. It processes images in the PPM format and provides an efficient way to compress and manipulate images by representing them as a quadtree structure.

## Features

- **Image Compression:** Convert a pixel matrix into a quadtree structure and save it as a binary file.
- **Image Decompression:** Reconstruct the image from the binary file using the quadtree representation.
- **Image Mirroring:** Perform horizontal and vertical mirroring operations on the image using the quadtree structure.

## Project Structure

```
.
├── Makefile                # Build instructions for the project
├── quadtree.c              # Main source file implementing the logic
├── README.md               # Documentation for the project
├── examples/               # Sample input and output files
 ── test_imgs/              # Test input files
```

## Installation

1. **Clone the Repository**
   ```bash
   git clone https://github.com/your-username/quadtree-image-processing.git
   cd quadtree-image-processing
   ```

2. **Build the Application**
   Use the provided Makefile to compile the project:
   ```bash
   make build
   ```

3. **Clean Up**
   To remove the compiled binaries:
   ```bash
   make clean
   ```

## Usage

### Compress an Image
To compress a PPM image:
```bash
./quadtree -c [compression_factor] [input.ppm] [output_file]
```

### Decompress an Image
To decompress a binary file back to a PPM image:
```bash
./quadtree -d [input_file] [output.ppm]
```

### Mirror an Image
To mirror an image along a specific axis:
```bash
./quadtree -m [axis] [compression_factor] [input.ppm] [output.ppm]
```
- `axis`: Specify `h` (horizontal) or `v` (vertical).

## Examples

Below are some examples of what the program can do:

### Original Image
![Original Image](examples/test0.ppm)

### Compressed Image
![Compressed Image](examples/compress0_0.out)

### Decompressed Image
![Decompressed Image](examples/decompress1.ppm)

### Horizontally Mirrored Image
![Horizontally Mirrored Image](examples/mirror0_h_0.ppm)

### Vertically Mirrored Image
![Vertically Mirrored Image](examples/mirror0_v_0.ppm)

## Makefile Details

- **Target: build**
  Compiles the `quadtree.c` file into an executable named `quadtree`.

- **Target: clean**
  Removes the compiled `quadtree` binary and other generated files.

Makefile example:
```makefile
CC = gcc
CFLAGS = -g -Wall -lm

build: quadtree

quadtree: quadtree.c
   $(CC) -o quadtree quadtree.c $(CFLAGS)

.PHONY : clean
clean:
   rm -f quadtree
```

## Tasks Overview

### Task 1: Image Compression
- **Description:** Converts a pixel matrix from a PPM image into a quadtree structure. The quadtree is built recursively, dividing the image into quadrants until the mean color difference is below the compression factor. The result is saved as a binary file.

### Task 2: Image Decompression
- **Description:** Reads the binary file containing the quadtree representation and reconstructs the original pixel matrix recursively. The result is written as a PPM image.

### Task 3: Image Mirroring
- **Description:** Performs horizontal or vertical mirroring of the image by adjusting the quadtree structure. The mirrored image is then reconstructed and saved as a PPM file.

