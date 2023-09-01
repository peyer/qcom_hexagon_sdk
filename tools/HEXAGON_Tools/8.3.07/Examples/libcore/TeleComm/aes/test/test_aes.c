/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:19:54 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:19:54 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include <hexagon_sim_timer.h>

#include "typedef.h"
#include "aes.h"

#include "testdata.h"

#define  OVERHEAD             8

UWord32 FT[4*256]           __attribute__((aligned(1<<10)));
UWord32 RT[4*256 + (256/4)] __attribute__((aligned(1<<10)));

int main()
{
	UWord8 cipherOut[16] __attribute__((aligned(8))); 
	UWord8 decryptOut[16] __attribute__((aligned(8)));
        AES_CTX ctx __attribute__((aligned(8)));

	unsigned long long start;
	unsigned long long end;
        int i;

	aes_gentabs(FT, RT);
        /* ---------------------------------------- */
        /* test with 128bit key                     */
        /* ---------------------------------------- */
        start = hexagon_sim_read_pcycles();
        aes_setkey(tstkey128,16,FT,&ctx);
        end = hexagon_sim_read_pcycles();

        printf("schedule128  :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_encrypt(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut128[i] ) {
              printf("### Encrypt 128 FAILED!!!\n");
              return 1;
           }

        }
        printf("enc 128-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt 128 FAILED!!!\n");
              return 1;
           }

        }
        printf("dec 128-bits :   %d\n",(int)(end-start-OVERHEAD));
        printf("---------------------------\n");


        /* ---------------------------------------- */
        /* test with 192bit key                     */
        /* ---------------------------------------- */
        start = hexagon_sim_read_pcycles();
        aes_setkey(tstkey192,24,FT,&ctx);
        end = hexagon_sim_read_pcycles();

        printf("schedule192  :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_encrypt(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut192[i] ) {
              printf("### Encrypt 192 FAILED!!!\n");
              return 1;
           }
        }
        printf("enc 192-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt 192 FAILED!!!\n");
              return 1;
           }
        }
        printf("dec 192-bits :   %d\n",(int)(end-start-OVERHEAD));
        printf("---------------------------\n");


        /* ---------------------------------------- */
        /* test with 256bit key                     */
        /* ---------------------------------------- */
        start = hexagon_sim_read_pcycles();
        aes_setkey(tstkey256,32,FT,&ctx);
        end = hexagon_sim_read_pcycles();

        printf("schedule256  :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_encrypt(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut256[i] ) {
              printf("### Encrypt 256 FAILED!!!\n");
              return 1;
           }
        }
        printf("enc 256-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt 256 FAILED!!!\n");
              return 1;
           }
        }
        printf("dec 256-bits :   %d\n",(int)(end-start-OVERHEAD));


	printf("\n\n");
        /* ---------------------------------------- */
        /* The second implementation                */
        /* ---------------------------------------- */
	aes_gentabs2(FT, RT);
        /* ---------------------------------------- */
        /* test with 128bit key                     */
        /* ---------------------------------------- */
        aes_setkey(tstkey128,16,FT,&ctx);

        start = hexagon_sim_read_pcycles();
        aes_encrypt2(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut128[i] ) {
              printf("### Encrypt(2) 128 FAILED!!!\n");
              return 1;
           }
        }
        printf("enc(2) 128-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt2(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt(2) 128 FAILED!!!\n");
              return 1;
           }
        }
        printf("dec(2) 128-bits :   %d\n",(int)(end-start-OVERHEAD));
        printf("---------------------------\n");


        /* ---------------------------------------- */
        /* test with 192bit key                     */
        /* ---------------------------------------- */
        aes_setkey(tstkey192,24,FT,&ctx);

        start = hexagon_sim_read_pcycles();
        aes_encrypt2(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut192[i] ) {
              printf("### Encrypt(2) 192 FAILED!!!\n");
              return 1;
           }
        }
        printf("enc(2) 192-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt2(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt 192 FAILED!!!\n");
              return 1;
           }
        }
        printf("dec(2) 192-bits :   %d\n",(int)(end-start-OVERHEAD));
        printf("---------------------------\n");


        /* ---------------------------------------- */
        /* test with 256bit key                     */
        /* ---------------------------------------- */
        aes_setkey(tstkey256,32,FT,&ctx);

        start = hexagon_sim_read_pcycles();
        aes_encrypt2(&ctx,FT,input,cipherOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (cipherOut[i] != ExpectedEncryptOut256[i] ) {
              printf("### Encrypt(2) 256 FAILED!!!\n");
              return 1;
           }
        }
        printf("enc(2) 256-bits :   %d\n",(int)(end-start-OVERHEAD));

        start = hexagon_sim_read_pcycles();
        aes_decrypt2(&ctx,RT,cipherOut,decryptOut);
        end = hexagon_sim_read_pcycles();

        for (i=0; i<16; i++) {
           if (decryptOut[i] != input[i] ) {
              printf("### Decrypt(2) 256 FAILED!!!\n");
              return 1;
           }
        }
        printf("dec(2) 256-bits :   %d\n",(int)(end-start-OVERHEAD));
        printf("\n*** PASS!!!\n");
	return 0;
}

