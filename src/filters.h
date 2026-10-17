#ifndef DISPLAY_H
#define DISPLAY_H

void displayImage(volatile unsigned short* imageIn,
                  volatile unsigned short* ImageOut);

void mirrorImage(volatile unsigned short* imageIn,
                 volatile unsigned short* ImageOut);

void sobelImage(volatile unsigned short* imageIn,
                volatile unsigned short* ImageOut);

void invertImage(volatile unsigned short* imageIn,
                 volatile unsigned short* ImageOut);

void kaleidoImage(volatile unsigned short* imageIn,
                  volatile unsigned short* imageOut);

// VGA
void swapVID();
void swapVGA();

unsigned short grayToRgb565(unsigned char gray);

#endif