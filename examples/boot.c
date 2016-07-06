// See LICENSE for license details.

#include <stdio.h>
#include "diskio.h"
#include "ff.h"
#include "uart.h"
#include "elf.h"
#include "memory.h"
#include "spi.h"

FATFS FatFs;   /* Work area (file system object) for logical drive */

// max size of file image is 16M
#define MAX_FILE_SIZE 0x1000000

// size of DDR RAM (128M for NEXYS4-DDR) 
#define DDR_SIZE 0x8000000

// 4K size read burst
#define SD_READ_SIZE 4096

int main (void)
{
  FIL fil;                /* File object */
  FRESULT fr;             /* FatFs return code */
  uint8_t *boot_file_buf = (uint8_t *)(get_ddr_base()) + DDR_SIZE - MAX_FILE_SIZE; // at the end of DDR space
  uint8_t *memory_base = (uint8_t *)(get_ddr_base());

  uart_init();

  printf("lowRISC boot program\n=====================================\n");

  /* Register work area to the default drive */
  if(f_mount(&FatFs, "", 1)) {
    printf("Fail to mount SD driver!\n");
    return 1;
  }

  /* Open a file */
  printf("Read boot.bin into buffer(16MB)\n");
  fr = f_open(&fil, "boot.bin", FA_READ);
  if (fr) {
    printf("Failed to open boot!\n");
    return (int)fr;
  }

  /* Read file into memory */
  uint8_t *buf = boot_file_buf;
  uint32_t br;                  /* Read count */
  do {
    fr = f_read(&fil, buf, SD_READ_SIZE, &br);  /* Read a chunk of source file */
    buf += br;
  } while(!(fr || br == 0));

  printf("Read %0x bytes.\n", fil.fsize);

  /* read elf */
  printf("Load boot.bin to DDR memory\n");
  if(br = load_elf(memory_base, boot_file_buf, fil.fsize))
    printf("elf read failed with code %0d", br);

  /* Close the file */
  if(f_close(&fil)) {
    printf("fail to close file!");
    return 1;
  }
  if(f_mount(NULL, "", 1)) {         /* unmount it */
    printf("fail to umount disk!");
    return 1;
  }

  spi_disable();

  /* jump to the loaded program */
  printf("Jump to boot.bin\n");
  asm volatile ("jr %0" : : "r" (get_ddr_base()));

}
