#include <stdint.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
//for ntohs
#include <arpa/inet.h>

extern int errno;

int open_i2c(int bus, int device_id) {
  int file;
  char filename[20];

  snprintf(filename,19,"/dev/i2c-%d",bus);
  file = open(filename, O_RDWR);
  if(file<0) return file;
  fprintf(stderr,"filehandle is %d\n",file);

  fprintf(stderr,"setting slave id to 0x%x\n",device_id);

  if(ioctl(file, I2C_SLAVE, device_id) <0){
    fprintf(stderr,"Unable to set device slave to 0x%x", device_id);
    return -1;
  }

  return file;
}

int setup_mcp3425(int file) {
  char buf[16];
  //page 14 of datasheet.
  //PGA gain=1, 16-bit samplerate, one-shot conversion, no channel selector, not ready
  uint8_t initial_config=0x8;

  return write(file, &initial_config, 1);	//write configuration byte
}

int initiate_conversion(int file) {
  uint8_t config=0x88;	//set RDY bit
  write(file, &config, 1);
}

struct device_return {
  uint8_t config;
  uint16_t value;
};

struct device_return read_data(int file) {
  char buf[16];
  int rv;
  memset(buf,0,16);
  rv=read(file, &buf, 3);
  if(rv<3){
    fprintf(stderr,"read returned %d\n", rv);
    if(rv<0) fprintf(stderr,"system error %d\n",errno);
  }
  
  printf("device return: 0x%02x%02x%02x\n",buf[0],buf[1],buf[2]);

  struct device_return r;
  memset(&r, 0, sizeof(struct device_return));
  memcpy(&r.value, &buf[0], 2);
  r.value = ntohs(r.value);
  memcpy(&r.config, &buf[2], 1);
  return r;
}


int main(int argc, char *argv) {
  puts("Starting up...\n");
  
  int file=open_i2c(1, 0x68);
  if(file<0){
    fprintf(stderr,"Could not open i2c device:  errno %d", errno);
    exit(1);
  }

  int n=setup_mcp3425(file);

  if(n<0){
    fprintf(stderr,"Unable to set up device: %d\n",errno);
  }

  while(1){
    initiate_conversion(file);
    struct device_return r = read_data(file);
    fprintf(stdout, "Got data 0x%04x and config 0x%02x\n", r.value, r.config);
    usleep(1000000);
  } 
}
