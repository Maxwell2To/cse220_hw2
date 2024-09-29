#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <stdint.h>

#include "hw2.h"

#define PACKET_TYPE_BIT_MASK 0x40000000
#define ADDRESS_BIT_MASK 0xFFFFFFFC
#define LENGTH_BIT_MASK 0x03FF
#define REQUESTER_ID_BIT_MASK 0XFFFF0000
#define TAG_BIT_MASK      0xFF00
#define LAST_BE_BIT_MASK  0x00F0
#define FIRST_BE_BIT_MASK 0x000F

uint32_t getPacketType(unsigned int pack) {return pack & PACKET_TYPE_BIT_MASK;}
uint32_t getAddress(unsigned int pack) {return pack & ADDRESS_BIT_MASK;}
uint32_t getLength(unsigned int pack) {return pack & LENGTH_BIT_MASK;}
uint32_t getRequesterID(unsigned int pack) {return (pack & REQUESTER_ID_BIT_MASK) >> 16;}
uint32_t getTag(unsigned int pack) {return (pack & TAG_BIT_MASK) >> 8;}
uint32_t getLastBE(unsigned int pack) {return (pack & LAST_BE_BIT_MASK) >> 4;}
uint32_t getFirstBE(unsigned int pack) {return pack & FIRST_BE_BIT_MASK;}


void print_packet(unsigned int packet[])
{
    (void) packet;

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

    if (isReadRequest == 0) {
      printf("Data: ");
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
