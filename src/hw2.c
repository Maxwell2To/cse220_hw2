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

int handle1Packet(unsigned int packets[], char *memory, int offset);


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
    uint32_t address = getAddress(packet[2]);
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


int readTlpRequest(unsigned int packets[], int offset, 
uint32_t *packetType, uint32_t *address, uint32_t *length, uint32_t *requesterID, uint32_t *tag, uint32_t *firstBE, uint32_t *lastBE) {

  *packetType = getPacketType(packets[offset + 0]);
  if (*packetType != 0) { // you know it's a read packet or not
    printf("No Output (invalid packet)\n");
    return -1;
  }

  //print_packet(packets);

  *address = getAddress(packets[offset + 2]);
  *length = getLength(packets[offset + 0]);
  *requesterID = getRequesterID(packets[offset + 1]); 
  *tag = getTag(packets[offset + 1]); 
  *firstBE = getFirstBE(packets[offset + 1]);
  *lastBE = getLastBE(packets[offset + 1]);
  return offset + 3;
}

///////////////////////////////////separte function to calculate byteCount and one to calulate lowerAddress
/////////////////////// call these functions in responseFiller

uint32_t findlowerAddress(uint32_t address){
  return (address & 0x7F);  ///////// 7F translates to last 7 bits
}

////////////////////////////////////// bytecount is (length * 4) + (remaining length * 4)  
//////////////////////////// address will need to be calculated
//////////////////////////// length needs to split if byteCount exceeds a limit


void responseFiller (
  uint32_t* completionResponse, int completionOffset, const char *memory, uint32_t address, uint32_t length, uint32_t requesterID, uint32_t tag, 
  uint32_t firstBE, uint32_t lastBE, uint32_t byteCount) {

  //printf("INSIDE RESPONSE FILLER: address is %d \n", address);

  uint32_t completerID = 0x00DC;
  uint32_t packetType = 0x4A000000;

  completionResponse[completionOffset + 0] |= packetType;
  completionResponse[completionOffset + 0] |= length;

  completionResponse[completionOffset + 1] |= (completerID << 16);
  completionResponse[completionOffset + 1] |= byteCount; /// will be calculated outside of this function

  completionResponse[completionOffset + 2] |= (requesterID << 16);
  completionResponse[completionOffset + 2] |= (tag << 8);
  completionResponse[completionOffset + 2] |= (findlowerAddress(address));

  int addressCount = address;   /////////////////// intialize to address which keeps going up

  for (uint32_t i = 0; i < length; i++) {
    int dataRowOffset = completionOffset + 3 + i;
    //printf("data row i is %d \n", i);

    if (i==0) {  /////////////////// if  you are one first 
      
      if (isBit0SetInBE(firstBE)) {      
        //printf("addressCount is %d and memory[addressCount] is %02X \n", addressCount, memory[addressCount]);
        completionResponse[dataRowOffset] |= (memory[addressCount] & 0x000000FF);
      }
      addressCount++;
      if (isBit1SetInBE(firstBE)) { //printf("addressCount is %d and memory[addressCount] is %02X \n", addressCount, memory[addressCount]);
        completionResponse[dataRowOffset] |= ((memory[addressCount] << 8) & 0x0000FF00);
        }
      addressCount++;
      if (isBit2SetInBE(firstBE)) { //printf("addressCount is %d and memory[addressCount] is %02X \n", addressCount, memory[addressCount]);
        completionResponse[dataRowOffset] |= ((memory[addressCount] << 16) & 0x00FF0000);
        }
      addressCount++;     
      if (isBit3SetInBE(firstBE)) { //printf("addressCount is %d and memory[addressCount] is %02X \n", addressCount, memory[addressCount]);
        completionResponse[dataRowOffset] |= ((memory[addressCount] << 24) & 0xFF000000);
        }
      addressCount++;
      if (length==1) { break; } 
    }

    else if (i == length-1) {
      if (isBit0SetInBE(lastBE)) {completionResponse[dataRowOffset] |= (memory[addressCount] & 0x000000FF);}
      addressCount++;
      if (isBit1SetInBE(lastBE)) {completionResponse[dataRowOffset] |= ((memory[addressCount] << 8) & 0x0000FF00);}
      addressCount++;
      if (isBit2SetInBE(lastBE)) {completionResponse[dataRowOffset] |= ((memory[addressCount] << 16) & 0x00FF0000);}
      addressCount++;     
      if (isBit3SetInBE(lastBE)) {completionResponse[dataRowOffset] |= ((memory[addressCount] << 24) & 0xFF000000);}
      addressCount++;
    }

    else {
      completionResponse[dataRowOffset] |= (memory[addressCount] & 0x000000FF);
      addressCount++;
      completionResponse[dataRowOffset] |= ((memory[addressCount] << 8) & 0x0000FF00);
      addressCount++;
      completionResponse[dataRowOffset] |= ((memory[addressCount] << 16) & 0x00FF0000);
      addressCount++;     
      completionResponse[dataRowOffset] |= ((memory[addressCount] << 24) & 0xFF000000);
      addressCount++;
    }
  }
} 

////////// if current address aka starting address + remaining length crosses over next 4k boundary then split
/////////// otherwise whatever is remaining length should fit within next 4k boundary
//////////// then just return rest of remaining length

uint32_t splitLength(uint32_t remainingLength, uint32_t currentAddress) {
    uint32_t startAddressBit14 = (currentAddress & 0x4000);
    uint32_t endingAddress = currentAddress + remainingLength * 4;
    uint32_t endingAddressBit14 = (endingAddress & 0x4000);

    uint32_t firstHalfSplit = (currentAddress % 0x4000);

    if (startAddressBit14 == endingAddressBit14){
        return remainingLength;
    }
    return (0x4000 - firstHalfSplit) / 4;
}

uint32_t calculateMemorySize(unsigned int packets[]) {
  int offset = 0;                
  int headerCount = 0;
  int lengthCount = 0;
  int splitCount = 0;

  while (offset != -1) {
    uint32_t packetType = 0;
    uint32_t address = 0;
    uint32_t length = 0;
    uint32_t requesterID = 0; 
    uint32_t tag = 0; 
    uint32_t firstBE = 0; 
    uint32_t lastBE = 0; 
    
    //printf("##############offset before is %d\n", offset);
    offset = readTlpRequest(packets, offset, &packetType, &address, &length, &requesterID, &tag, &firstBE, &lastBE);
    //printf("offset after is %d\n", offset);

    if (offset == -1)
      break;
    headerCount++;
    lengthCount += length;

    uint32_t remainingLength = length;
    uint32_t startingAddress = address;

    while (remainingLength > 0) {
      uint32_t responseDataLength = splitLength(remainingLength, startingAddress);
      remainingLength -= responseDataLength;
      
      if (0 != remainingLength)
        splitCount++;

      //printf("responseDataLength is %d and remaining length is %d \n", responseDataLength, remainingLength);
      //printf("startingAddress is 0X%x\n", startingAddress);

      startingAddress += responseDataLength * 4;
    } 
  }
  return ((headerCount * 3) + (splitCount * 3) + lengthCount) * sizeof(uint32_t);
}

unsigned int* create_completion(unsigned int packets[], const char *memory) {
  (void)memory;
  uint32_t* completionResponse = NULL;
  int offset = 0;
    
  int totalSize = calculateMemorySize(packets);  ///////////////////// allocate just enough memory here
  completionResponse = malloc(totalSize);  
  memset(completionResponse, 0, totalSize);
  //printf("totalSize of allocated memory is %d \n", totalSize);
  
  uint32_t completionOffset = 0;                  
  while (offset != -1) {
    uint32_t packetType = 0;
    uint32_t address = 0;
    uint32_t length = 0;
    uint32_t requesterID = 0; 
    uint32_t tag = 0; 
    uint32_t firstBE = 0; 
    uint32_t lastBE = 0; 
    //printf("##############offset before is %d\n", offset);
    offset = readTlpRequest(packets, offset, &packetType, &address, &length, &requesterID, &tag, &firstBE, &lastBE);
    //printf("offset after is %d\n", offset);
    if (offset == -1)
      break;
    //printf("address: %d  length: %d   requesterID: %d   tag: %d   firstBE: %d  lastBE: %d \n",
      //address, length, requesterID, tag, firstBE, lastBE);

    uint32_t remainingLength = length;
    uint32_t startingAddress = address;
    

//////////////////////////// address will need to be calculated
//////////////////////////// length needs to split if byteCount exceeds a limit
    while (remainingLength > 0) {
      uint32_t responseDataLength = splitLength(remainingLength, startingAddress);
      remainingLength -= responseDataLength;
      uint32_t byteCount = (responseDataLength * 4) + (remainingLength * 4);
      
      //printf("responseDataLength is %d and remaining length is %d \n", responseDataLength, remainingLength);
      //printf("bytecount is %d \n", byteCount);
      //printf("startingAddress is 0X%x\n", startingAddress);

      responseFiller (
        completionResponse, completionOffset, memory, startingAddress, responseDataLength, requesterID, tag, 
        firstBE, lastBE, byteCount);
      completionOffset += 3 + responseDataLength;
      startingAddress += responseDataLength * 4;
    } 
  }
	return completionResponse;
}



