#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

char* loadPng(const char* filename, int* width, int* height) {
  unsigned char* image = NULL;
  int error = lodepng_decode32_file(&image, width, height, filename);
  if (error) {
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  return (image);
}

void writePng(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
  unsigned error;
  unsigned char *png;
  size_t pngsize;
  LodePNGState state;
  lodepng_state_init(&state);
  error = lodepng_encode(&png, &pngsize, image, width, height, &state);
  if (!error)
    lodepng_save_file(png, pngsize, filename);
  if (error)
    printf("error %u: %s\n", error, lodepng_error_text(error));
  lodepng_state_cleanup(&state);
  free(png);
}

void preparation(unsigned char* od, int ih, int iw) {
  for (int i = 0; i < ih; i++) {
    for (int j = 0; j < iw; j++) {
      if (od[iw * i + j] < 110) {
        od[iw * i + j] = 0;
      }
      if (od[iw * i + j] > 195) {
        od[iw * i + j] = 255;
      }
    }
  }
  return;
}

void fGauss(unsigned char *od, unsigned char *dc, int ih, int iw) {
  for (int i = 2; i < ih - 1; i++) {
    for (int j = 2; j < iw - 1; j++) {
      dc[iw * i * 4 + j * 4] = 0.12 * od[iw * i * 4 + j * 4] + 0.12 * od[iw * (i + 1) * 4 + j * 4] + 0.12 * od[iw * (i - 1) * 4 + j * 4] + 0.12 * od[iw * i * 4 + (j - 1) * 4] + 0.12 * od[iw * 4 * i + (j + 1) * 4] + 0.12 * od[iw * (i - 1) * 4 + (j - 1) * 4] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4] + 0.12 * od[iw * (i - 1) * 4 + (j + 1) * 4] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4];
      dc[iw * 4 * i + j * 4 + 1] = 0.12 * od[iw * i * 4 + j * 4 + 1] + 0.12 * od[iw * (i + 1) * 4 + j * 4 + 1] + 0.12 * od[iw * (i - 1) * 4 + j * 4 + 1] + 0.12 * od[iw * i * 4 + (j - 1) * 4 + 1] + 0.12 * od[iw * 4 * i + (j + 1) * 4 + 1] + 0.12 * od[iw * (i - 1) * 4 + (j - 1) * 4 + 1] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 1] + 0.12 * od[iw * (i - 1) * 4 + (j + 1) * 4 + 1] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 1];
      dc[iw * 4 * i + j * 4 + 2] = 0.12 * od[iw * i * 4 + j * 4 + 2] + 0.12 * od[iw * (i + 1) * 4 + j * 4 + 2] + 0.12 * od[iw * (i - 1) * 4 + j * 4 + 2] + 0.12 * od[iw * i * 4 + (j - 1) * 4 + 2] + 0.12 * od[iw * 4 * i + (j + 1) * 4 + 2] + 0.12 * od[iw * (i - 1) * 4 + (j - 1) * 4 + 2] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 2] + 0.12 * od[iw * (i - 1) * 4 + (j + 1) * 4 + 2] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 2];
      dc[iw * 4 * i + j * 4 + 3] = 0.12 * od[iw * i * 4 + j * 4 + 3] + 0.12 * od[iw * (i + 1) * 4 + j * 4 + 3] + 0.12 * od[iw * (i - 1) * 4 + j * 4 + 3] + 0.12 * od[iw * i * 4 + (j - 1) * 4 + 3] + 0.12 * od[iw * 4 * i + (j + 1) * 4 + 3] + 0.12 * od[iw * (i - 1) * 4 + (j - 1) * 4 + 3] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 3] + 0.12 * od[iw * (i - 1) * 4 + (j + 1) * 4 + 3] + 0.12 * od[iw * (i + 1) * 4 + (j - 1) * 4 + 3];
    }
  }
  return;
}

void colouring(unsigned char* dc, unsigned char *mcod, int ih, int iw, int n) {
  for (int i = 1; i < iw * ih; i++) {
    mcod[i * n] =  dc[i] + 0.3 * dc[i - 1];
    mcod[i * n + 1] = 110 + dc[i - 1];
    mcod[i * n + 2] = 255 + dc[i + 1];
    mcod[i * n + 3] = 250;
  }
  return;
}

int main() {
  char *inputPath = "AAAAA.png";
  int iw, ih, n = 4;
  int k = 0;
  char *idata = loadPng(inputPath, &iw, &ih);
  if (idata == NULL) {
    printf("ERROR: can't read file %s\n", inputPath);
    return 1;
  }
  unsigned char * data = (unsigned char *) malloc (ih * iw * sizeof(unsigned char));
  unsigned char * copy = (unsigned char *) malloc (ih * iw * sizeof(unsigned char));
  unsigned char * mc = (unsigned char *) malloc (ih * iw * n * sizeof(unsigned char));
  for (int i = 0; i < ih * iw; i = i + 1) {
    data[k] = 0.299 * idata[i * 4] + 0.587 * idata[4 * i + 1] + 0.114 * idata[i * 4 + 2];
    k++;
  }
  preparation(data, ih, iw);
  fGauss(data, copy, ih, iw);
  colouring(copy, mc, ih, iw, n); 
  char *outputPath = "A_A.png";
  writePng(outputPath, mc, iw, ih);
  printf("end*_*");
  return 0;
}
