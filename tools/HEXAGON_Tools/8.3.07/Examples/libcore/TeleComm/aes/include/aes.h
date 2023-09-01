/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file aes.h
    \brief Brief description of file
*/


typedef struct {
  int key_length;
  int reserve;
  UWord32 E[64];
  UWord32 D[64];
} AES_CTX;

typedef unsigned char __attribute__((aligned(4)))  uword8_4B_align;

/*! 
Generate lookup table for AES cipher

\param EncryptTAB pointer to lookup table used for encryption
\param DecryptTAB pointer to lookup table used for decryption

\details

\b Assembly \b Assumptions
 - \a Lookup table is aligned by 1024-bytes boundray

\b Cycle-Count
 - None

\b Notes
 - LUT for encryption is 4096 bytes
 - LUT for decryption is 4352 bytes

*/

void aes_gentabs(UWord32 *EncryptTAB, UWord32 *DecryptTAB);


/*! 
Generate a smaller lookup table for AES cipher

\param EncryptTAB pointer to lookup table used for encryption
\param DecryptTAB pointer to lookup table used for decryption

\details

\b Assembly \b Assumptions
 - \a Lookup tables are aligned by 1024-bytes boundray

\b Cycle-Count
 - None

\b Notes
 - LUT for encryption is 1024 bytes
 - LUT for decryption is 1280 bytes

*/

void aes_gentabs2(UWord32 *EncryptTAB, UWord32 *DecryptTAB);



/*! 
Implements AES key schedule algorithm

\param in_key pointer to keys
\param key_len length of key in bytes
\param EncryptTAB lookup table used by the function
\param ctx pointer to AES context to be initialized

\details

\b Assembly \b Assumptions
 - \a in_key array is aligned by 4-bytes
 - \a AES context structure is aligned by 4-bytes

\b Cycle-Count
 - 308 (for 128-bit key)
 - 357 (for 192-bit key)
 - 438 (for 256-bit key)

\b Notes
 - None.

*/

int aes_setkey(UWord8 *in_key, unsigned int key_len, UWord32 *EncryptTAB, AES_CTX *ctx);



/*! 
Implements AES encryption

\param ctx pointer to AES context
\param EncryptTAB pointer to lookup table
\param in pointer to 128-bit plaintext block
\param out pointer to ciphertext block

\details

\b Assembly \b Assumptions
 - \a context structure is aligned by 8-bytes
 - \a in/out block are aligned by 8-bytes
 - \a Lookup table is aligned by 1024-bytes

\b Cycle-Count
 - 12*nr + 12 

\b Notes
 - None.

*/

void aes_encrypt(AES_CTX *ctx, UWord32 *EncryptTAB, UWord8 *in, UWord8 *out);


/*! 
Implements AES decryption

\param ctx pointer to AES context
\param DecryptTAB pointer to lookup table
\param in pointer to 128-bit ciphertext block
\param out pointer to plaintext block output

\details

\b Assembly \b Assumptions
 - \a context structure is aligned by 8-bytes
 - \a in/out block are aligned by 8-bytes
 - \a Lookup table is aligned by 1024-bytes

\b Cycle-Count
 - 12*nr + 14 

\b Notes
 - None.

*/

void aes_decrypt(AES_CTX *ctx, UWord32 *DecryptTAB, UWord8 *in, UWord8 *out);



/*! 
Implements AES encryption

\param ctx pointer to AES context
\param EncryptTAB pointer to lookup table
\param in pointer to 128-bit plaintext block
\param out pointer to ciphertext block 

\details

\b Assembly \b Assumptions
 - \a context structure is aligned by 8-bytes
 - \a in/out block are aligned by 8-bytes
 - \a Lookup table is aligned by 1024-bytes

\b Cycle-Count
 - 17*nr + 7 

\b Notes
 - This implementation requires a smaller LUT

*/

void aes_encrypt2(AES_CTX *ctx, UWord32 *EncryptTAB, UWord8 *in, UWord8 *out);




/*! 
Implements AES decryption

\param ctx pointer to AES context
\param DecryptTAB pointer to lookup table
\param in pointer to 128-bit ciphertext block
\param out pointer to plaintext block output

\details

\b Assembly \b Assumptions
 - \a context structure is aligned by 8-bytes
 - \a in/out block are aligned by 8-bytes
 - \a Lookup table is aligned by 1024-bytes

\b Cycle-Count
 - 17*nr + 8 

\b Notes
 - This implementation requires a smaller LUT

*/

void aes_decrypt2(AES_CTX *ctx, UWord32 *DecryptTAB, UWord8 *in, UWord8 *out);

