#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "hw2.h"

#define PACKET_TYPE_BIT_MASK 0x7FFFFC00 //0x40000000
#define PACKET_WRITE_PACKET_BIT_MASK 0x40000000
#define ADDRESS_BIT_MASK 0xFFFFFFFF // 0b 1111 1111 1111 1111 1111 1111 1111 1111
#define LENGTH_BIT_MASK 0x03FF
#define REQUESTER_ID_BIT_MASK 0XFFFF0000
#define TAG_BIT_MASK      0xFF00
#define LAST_BE_BIT_MASK  0x00F0
#define FIRST_BE_BIT_MASK 0x000F

int isInvalidPacket(unsigned int pack) {
    if ((pack & PACKET_TYPE_BIT_MASK) == 0) // you know it's a read packet
      return 1;
    if ((pack & PACKET_TYPE_BIT_MASK) == PACKET_WRITE_PACKET_BIT_MASK) // you know it's a write packet
      return 1;
  return 0; // invalid
}
uint32_t getPacketType(unsigned int pack) {return pack & PACKET_TYPE_BIT_MASK;}
uint32_t getAddress(unsigned int pack) {return (pack & ADDRESS_BIT_MASK) /* >> 2*/;}   /////////////////////////One of these address functions is fucked up
uint32_t getLength(unsigned int pack) {return pack & LENGTH_BIT_MASK;}
uint32_t getRequesterID(unsigned int pack) {return (pack & REQUESTER_ID_BIT_MASK) >> 16;}
uint32_t getTag(unsigned int pack) {return (pack & TAG_BIT_MASK) >> 8;}
uint32_t getLastBE(unsigned int pack) {return (pack & LAST_BE_BIT_MASK) >> 4;}
uint32_t getFirstBE(unsigned int pack) {return pack & FIRST_BE_BIT_MASK;}


#define BUF_SIZE 33
char *int2bin(int a, char *buffer, int buf_size) {
    memset(buffer, 0, BUF_SIZE);
    buffer += (buf_size - 1);
    for (int i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';
        a >>= 1;
    }
    return buffer;
}

void print_packet(unsigned int packet[]) {
  /*
   char buffer[BUF_SIZE];
    int2bin(packet[0], buffer, BUF_SIZE - 1);
    printf("#### packet 0 = %s\n", buffer);
        int2bin(packet[1], buffer, BUF_SIZE - 1);
    printf("#### packet 1 = %s\n", buffer);
    int2bin(packet[2], buffer, BUF_SIZE - 1);
    printf("#### packet 2 = %s\n", buffer);
*/
  if (isInvalidPacket(packet[0]) == 0){
    printf("No Output (invalid packet)\n");
    return;
  }

    uint32_t packetType = getPacketType(packet[0]);
    uint32_t address = getAddress(packet[2]);;
    uint32_t length = getLength(packet[0]);
    uint32_t requesterID = getRequesterID(packet[1]); 
    uint32_t tag = getTag(packet[1]); 
    uint32_t lastBE = getLastBE(packet[1]); 
    uint32_t firstBE = getFirstBE(packet[1]); 

    int isReadRequest = 0; 
    if (packetType & PACKET_TYPE_BIT_MASK)
      printf("Packet Type: Write\n");
    else {
      printf("Packet Type: Read\n");
      isReadRequest = 1;
    }
    printf("Address: %d\n", address);
    printf("Length: %d\n", length);
    printf("Requester ID: %d\n", requesterID);
    printf("Tag: %d\n", tag);
    printf("Last BE: %d\n", lastBE);
    printf("1st BE: %d\n", firstBE);

    printf("Data: ");
    if (isReadRequest == 0) {
      for (uint32_t i = 0; i < length; i++)
        printf("%d ", packet[3+i]);
    }
    printf("\n");
}


uint32_t beValueToMask(uint32_t beValue) {
  uint32_t mask = 0x0000;
  if (beValue & 0x0001)
    mask |= 0x000000FF;
  if (beValue & 0x0002)
    mask |= 0x0000FF00;
  if (beValue & 0x0004)
    mask |= 0x00FF0000;
  if (beValue & 0x0008)
    mask |= 0xFF000000;
  return mask; 
}


/*
STEPS:
Take BE and convert to binary, which determines the ranges you need to shift

example: 
12 = 0b1100
So now, you will take the first 4 digits of the hex data according to the positions of the 11
0x01020304
memory[1342]=0x02
memory[1343]=0x01

Notice how it skipped memory[1340] and memory[1341]

Put 2 digits at a time in the memory (right to left)

For data that is in between, just store the whole thing


*/

int isBit0SetInBE(uint32_t beValue){
  return beValue & 0x0001;
}
int isBit1SetInBE(uint32_t beValue){
  return beValue & 0x0002;
}
int isBit2SetInBE(uint32_t beValue){
  return beValue & 0x0004;
}
int isBit3SetInBE(uint32_t beValue){
  return beValue & 0x0008;
}

void store_values(unsigned int packets[], char *memory) {

  int offset = 0;   //////// packet row offset

  while (offset != -1){
    offset = handle1Packet(packets, memory, offset);
  }

}

int handle1Packet(unsigned int packets[], char *memory, int offset) {
  if ((packets[offset + 0] & PACKET_WRITE_PACKET_BIT_MASK) == 0){      //////////////////valid packet check for write packet, returns -1 if invalid
    return -1;
  }

  int addressCount = getAddress(packets[offset + 2]);  ///////////////////
  int length = getLength(packets[offset + 0]);
  uint32_t firstBE = getFirstBE(packets[offset + 1]);
  uint32_t lastBE = getLastBE(packets[offset + 1]);
 
  if (addressCount > 1000000) 
    return offset + 3 + length;  ///////// packet was too big 

  for (int i = 0; i < length; i++) {
    int dataRowOffset = offset + 3 + i;

    if (i==0) {
      if (isBit0SetInBE(firstBE)) { 
        memory[addressCount] = (char) (packets[dataRowOffset] & 0x000000FF);
        }
      addressCount++;
      if (isBit1SetInBE(firstBE)) { 
        memory[addressCount] = (char) (packets[dataRowOffset] >> 8 & 0x000000FF);
        }
      addressCount++;
      if (isBit2SetInBE(firstBE)) { 
        memory[addressCount] = (char) (packets[dataRowOffset] >> 16 & 0x000000FF);
        }
      addressCount++;     
      if (isBit3SetInBE(firstBE)) { 
        memory[addressCount] = (char) (packets[dataRowOffset] >> 24 & 0x000000FF);
        }
      addressCount++;
      if (length==1) { break; }
    }

    else if (i == length-1) {
        // use last BE to write last buffer
      if (isBit0SetInBE(lastBE)) { memory[addressCount] = (char) (packets[dataRowOffset] & 0xFF);
      }
      addressCount++;
      if (isBit1SetInBE(lastBE)) { memory[addressCount] = (char) (packets[dataRowOffset] >> 8 & 0xFF);
      }
      addressCount++;
      if (isBit2SetInBE(lastBE)) { memory[addressCount] = (char) (packets[dataRowOffset] >> 16 & 0xFF);
      }
      addressCount++;     
      if (isBit3SetInBE(lastBE)) { memory[addressCount] = (char) (packets[dataRowOffset] >> 24 & 0xFF);
      }
      addressCount++;
    } 
    
    else {
      memory[addressCount] = (char) (packets[dataRowOffset] & 0xFF);
      addressCount++;
      memory[addressCount] = (char) (packets[dataRowOffset] >> 8 & 0xFF);
      addressCount++;
      memory[addressCount] = (char) (packets[dataRowOffset] >> 16 & 0xFF);
      addressCount++;     
      memory[addressCount] = (char) (packets[dataRowOffset] >> 24 & 0xFF);
      addressCount++;
    }
  }
  return offset + 3 + length;  //// packet was valid and thus handled, now it returns the new offset

}


unsigned int* create_completion(unsigned int packets[], const char *memory) {
    (void)packets;
    (void)memory;
	return NULL;
}
