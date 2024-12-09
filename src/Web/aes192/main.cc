
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "AES_192_CBC.h"

int main1();
int main2();

int main()
{
    main1();
    main2();
}


int main1()
{
    AES_CTX ctx;
    uint8_t key[AES_KEY_SIZE+1];  // 24
    uint8_t iv[AES_BLOCK_SIZE+1]; // 16
    uint8_t data[AES_BLOCK_SIZE * 3]; // Example data with three blocks

    printf("AES_KEY_SIZE=%d AES_BLOCK_SIZE=%d:\n", AES_KEY_SIZE, AES_BLOCK_SIZE );
    
    // Initialization and usage example
    memset(key, 0, AES_KEY_SIZE+1);
    //memcpy(key, "This is aes key!", AES_KEY_SIZE);
    strcpy((char*)key, "This is aes key!");

    // memcpy(iv, "This is an IV123", AES_BLOCK_SIZE);
    memset(iv, 0, AES_BLOCK_SIZE+1);
    strcpy((char*)iv, "This is an IV123");


    memset(data, 0, AES_BLOCK_SIZE * 3);
    strcpy( (char*)data, "This is test msg123456");
    
    printf("Original:  ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
    
    AES_EncryptInit(&ctx, key, iv);
    
    for (unsigned int offset = 0; offset < AES_BLOCK_SIZE * 3; offset += AES_BLOCK_SIZE) {
        AES_Encrypt(&ctx, data + offset, data + offset);
    }
    
    printf("Encrypted: ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
    
    AES_DecryptInit(&ctx, key, iv);
    
    for (unsigned int offset = 0; offset < AES_BLOCK_SIZE * 3; offset += AES_BLOCK_SIZE) {
        AES_Decrypt(&ctx, data + offset, data + offset);
    }
    
    printf("Decrypted: ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    printf("decstr=[%s]\n", data);
    
    AES_CTX_Free(&ctx);
    return 0;
}

int main2()
{
    AES_CTX ctx;
    // int EXT_BLOCK_SIZE = 300*AES_BLOCK_SIZE;  OK
    int EXT_BLOCK_SIZE = 2000*AES_BLOCK_SIZE;

    uint8_t key[AES_KEY_SIZE];
    uint8_t iv[AES_BLOCK_SIZE];

    // uint8_t data[EXT_BLOCK_SIZE]; // Example data with three blocks
    uint8_t *data = (uint8_t*)malloc(EXT_BLOCK_SIZE+1);
    printf("data=%p\n", (void*)data );
    fflush(stdout);
    if ( ! data ) {
        printf("d06: malloc failed\n");
        fflush(stdout);
        exit(1);
    }


    data[EXT_BLOCK_SIZE] = 0;

    printf("d03039 AES_KEY_SIZE=%d AES_BLOCK_SIZE=%d EXT_BLOCK_SIZE=%d:\n", AES_KEY_SIZE, AES_BLOCK_SIZE, EXT_BLOCK_SIZE );
    
    // Initialization and usage example
    memcpy(key, "This is aes key!", AES_KEY_SIZE);
    memcpy(iv, "This is an IV123", AES_BLOCK_SIZE);

    memset(data, 0, EXT_BLOCK_SIZE );
    strcpy(  (char*)data, "This is test msg123456 kkkkkkkkkkkkkkkkk  end dddddddddddddddddddddddddd sssssssssssssssssssssssssssssss jdfff  end.");
    
    printf("d12305 done copy. Original:  ");
    fflush(stdout);
    /***
    for (unsigned int index = 0; index < EXT_BLOCK_SIZE; index++) {
        printf("%02X", data[index]);
        fflush(stdout);
    }
    ***/
    printf("OK\n");
    fflush(stdout);
    
    AES_EncryptInit(&ctx, key, iv);
    
    for (unsigned int offset = 0; offset < EXT_BLOCK_SIZE; offset += AES_BLOCK_SIZE) {
        AES_Encrypt(&ctx, data + offset, data + offset);
    }
    
    printf("d2220 Encrypted: ");
    /**
    for (unsigned int index = 0; index < EXT_BLOCK_SIZE; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
    fflush(stdout);
    **/
    
    AES_DecryptInit(&ctx, key, iv);
    
    for (unsigned int offset = 0; offset < EXT_BLOCK_SIZE; offset += AES_BLOCK_SIZE) {
        AES_Decrypt(&ctx, data + offset, data + offset);
    }
    
    printf("Decrypted: ");
    /**
    for (unsigned int index = 0; index < EXT_BLOCK_SIZE; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
    **/

    printf("decstr=[%s]\n", data);
    fflush(stdout);
    
    free( (void*)data);

    AES_CTX_Free(&ctx);
    return 0;
}

