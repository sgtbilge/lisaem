/**************************************************************************************\
*                   A part of the Apple Lisa 2 Emulator Project                        *
*                                                                                      *
*                    Copyright (C) MMXXVI  Ray A. Arachelian                           *
*                            All Rights Reserved                                       *
*                                                                                      *
*                    Recalculate and fix checksums on DC42 images                      *
*                                                                                      *
\**************************************************************************************/

#include <libdc42.h>

int main(int argc, char *argv[])
{
  int ret;
  DC42ImageType F;

  puts(    "  ---------------------------------------------------------------------------");
  puts(    "    DC42 Fix Checksum                          http://lisaem.sunder.net");
  puts(    "  ---------------------------------------------------------------------------");
  puts(    "          Copyright (C) MMXXVI, Ray A. Arachelian, All Rights Reserved.");
  puts(    "              Released under the GNU Public License, Version 2.0");
  puts(    "    There is absolutely no warranty for this program. Use at your own risk.  ");
  puts(    "  ---------------------------------------------------------------------------\n");
  puts(    "              Recalculate and fix checksums on a DC42 disk image.");

  if (argc != 2)
  {
    puts("");
    puts("  This program recalculates the data and tag checksums for a DC42 disk image");
    puts("  and updates the image header with the correct values. This is useful for");
    puts("  fixing images that show checksum errors when mounting.\n");
    puts("  The image file will be modified in place.");
    puts("");
    puts("  Usage:  dc42-fix-checksum  {disk image}");
    puts("  i.e.   ./dc42-fix-checksum boot-disk.dc42");
    puts("");
    exit(0);
  }

  printf("Opening: %s\n", argv[1]);
  ret = dc42_auto_open(&F, argv[1], "wb");
  if (ret) {
    fprintf(stderr, "Error: Could not open image %s\n", argv[1]);
    fprintf(stderr, "Reason: %s (code: %d)\n", F.errormsg, ret);
    exit(1);
  }

  printf("Image opened successfully:\n");
  printf("  Name:         %s\n", dc42_get_volname(&F));
  printf("  Size:         %u blocks (%u bytes)\n", F.numblocks, F.numblocks * 512);
  printf("  Tag size:     %u bytes\n", F.tagsize);
  printf("\n");

  // Check current checksums
  int checksum_status = dc42_check_checksums(&F);
  if (checksum_status == 0) {
    printf("Checksums are already correct - no changes needed.\n");
    dc42_close_image(&F);
    exit(0);
  }

  printf("Current checksum status:\n");
  if (checksum_status & 1) printf("  Tag checksum:  INVALID\n");
  else                     printf("  Tag checksum:  OK\n");
  if (checksum_status & 2) printf("  Data checksum: INVALID\n");
  else                     printf("  Data checksum: OK\n");
  printf("\n");

  printf("Recalculating checksums...\n");
  ret = dc42_recalc_checksums(&F);
  if (ret) {
    fprintf(stderr, "Error: Failed to recalculate checksums (code: %d)\n", ret);
    dc42_close_image(&F);
    exit(1);
  }

  printf("Closing and saving image...\n");
  ret = dc42_close_image(&F);
  if (ret) {
    fprintf(stderr, "Error: Failed to close image (code: %d)\n", ret);
    exit(1);
  }

  printf("\n");
  printf("SUCCESS: Checksums have been recalculated and saved.\n");
  printf("The image should now mount without checksum warnings.\n");

  return 0;
}
