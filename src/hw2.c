#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "hw2.h"

#define PACKET_TYPE_BIT_MASK 0x7FFFFC00 //0x40000000
#define PACKET_WRITE_PACKET_BIT_MASK 0x40000000
//#define ADDRESS_BIT_MASK 0xFFFFFFFC // 0b 1111 1111 1111 1111 1111 1111 1111 1100 spec says last 2 bits should be 0
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
uint32_t getAddress(unsigned int pack) {return (pack & ADDRESS_BIT_MASK) /* >> 2*/;}
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

void print_packet(unsigned int packet[])
{
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

    // data chunk
    //int32_t field1 = packet[3] >> 
    //int32_t field2 = packet[0] >>

    printf("Data: ");
    if (isReadRequest == 0) {
      for (uint32_t i = 0; i < length; i++)
        printf("%d ", packet[3+i]);
    }
    printf("\n");
}


void store_values(unsigned int packets[], char *memory)
{
    (void)packets;
    (void)memory;
}

unsigned int* create_completion(unsigned int packets[], const char *memory)
{
    (void)packets;
    (void)memory;
	return NULL;
}
