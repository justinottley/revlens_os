#include <avif/avif.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  // --- Configuration ---
  int width = 640;
  int height = 480;
  int depth = 8; // 8 bits per channel (common for RGB)
  const char *output_file = "output.avif";

  // --- 1. Prepare Raw RGB Data (Example) ---
  //  Create some dummy RGB data:
  size_t rgb_size = width * height * 3; // 3 bytes per pixel (R, G, B)
  unsigned char *rgb_pixels = (unsigned char *)malloc(rgb_size);
  if (!rgb_pixels) {
    fprintf(stderr, "Memory allocation failed for RGB data.\n");
    return 1;
  }

  // Fill with some random data:
  for (size_t i = 0; i < rgb_size; ++i) {
    rgb_pixels[i] = (unsigned char)rand();
  }

  // --- 2. Create an avifImage ---
  avifImage *image = avifImageCreate(width, height, depth, AVIF_PIXEL_FORMAT_RGB);
    if (!image) {
        fprintf(stderr, "Failed to create avifImage.\n");
        free(rgb_pixels);
        return 1;
    }

  // --- 3. Set the Image Planes ---
  image->yuvFormat = AVIF_PIXEL_FORMAT_RGB; // Indicate the RGB format
  image->planes[AVIF_PLANE_Y] = rgb_pixels; // RGB is treated as a single plane
  image->rowBytes[AVIF_PLANE_Y] = width * 3; // Width in bytes

  // --- 4. Create an avifEncoder ---
  avifEncoder *encoder = avifEncoderCreate();
    if (!encoder) {
        fprintf(stderr, "Failed to create avifEncoder.\n");
        avifImageDestroy(image);
        free(rgb_pixels);
        return 1;
    }

  // --- 5. Encode the Image ---
    avifRWData output_avif = AVIF_RWDATA_EMPTY;
  avifResult encode_result = avifEncoderWrite(encoder, image, &output_avif);
  if (encode_result != AVIF_RESULT_OK) {
    fprintf(stderr, "AVIF encoding failed: %s\n", avifResultToString(encode_result));
      avifEncoderDestroy(encoder);
      avifImageDestroy(image);
      free(rgb_pixels);
    return 1;
  }

  // --- 6. Save to Disk ---
    FILE *fp = fopen(output_file, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open output file.\n");
        avifRWDataFree(&output_avif);
        avifEncoderDestroy(encoder);
        avifImageDestroy(image);
        free(rgb_pixels);
        return 1;
    }
    fwrite(output_avif.data, 1, output_avif.size, fp);
    fclose(fp);

  // --- 7. Clean Up ---
    avifRWDataFree(&output_avif);
    avifEncoderDestroy(encoder);
    avifImageDestroy(image);
  free(rgb_pixels);

  printf("Successfully encoded AVIF: %s\n", output_file);
  return 0;
}