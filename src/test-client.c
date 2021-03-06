#include "shs1.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sodium.h>

// from https://stackoverflow.com/a/23898449
uint8_t hextobin(const char * str, uint8_t * bytes, size_t blen)
{
   uint8_t pos;
   uint8_t idx0;
   uint8_t idx1;

   // mapping of ASCII characters to hex values
   const uint8_t hashmap[] =
   {
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
     0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
   };

   bzero(bytes, blen);
   for (pos = 0; ((pos < (blen*2)) && (pos < strlen(str))); pos += 2)
   {
      idx0 = (uint8_t)str[pos+0];
      idx1 = (uint8_t)str[pos+1];
      bytes[pos/2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
   };

   return(0);
}

int main(int argc, char *argv[])
{
  uint8_t network_identifier[SHS1_NETWORKIDENTIFIERBYTES];
  uint8_t server_longterm_pk[crypto_sign_PUBLICKEYBYTES];

  uint8_t client_longterm_pk[crypto_sign_PUBLICKEYBYTES];
  uint8_t client_longterm_sk[crypto_sign_SECRETKEYBYTES];
  uint8_t client_ephemeral_pk[crypto_box_PUBLICKEYBYTES];
  uint8_t client_ephemeral_sk[crypto_box_SECRETKEYBYTES];

  uint8_t msg1[SHS1_MSG1_BYTES];
  uint8_t msg2[SHS1_MSG2_BYTES];
  uint8_t msg3[SHS1_MSG3_BYTES];
  uint8_t msg4[SHS1_MSG4_BYTES];

  SHS1_Outcome client_outcome;

  assert(argc == 3);
  assert(sodium_init() != -1);

  hextobin(argv[1], network_identifier, SHS1_NETWORKIDENTIFIERBYTES);
  hextobin(argv[2], server_longterm_pk, crypto_sign_PUBLICKEYBYTES);

  crypto_sign_keypair(client_longterm_pk, client_longterm_sk);
  crypto_box_keypair(client_ephemeral_pk, client_ephemeral_sk);

  SHS1_Client c;
  SHS1_Client *client = &c;
  shs1_init_client(
    client,
    network_identifier,
    client_longterm_pk,
    client_longterm_sk,
    client_ephemeral_pk,
    client_ephemeral_sk,
    server_longterm_pk
  );

  shs1_create_msg1(msg1, client);
  fwrite(msg1, sizeof(uint8_t), SHS1_MSG1_BYTES, stdout);
  fflush(stdout);

  fread(msg2, sizeof(uint8_t), SHS1_MSG2_BYTES, stdin);
  if (!shs1_verify_msg2(msg2, client)) {
    exit(2);
  }

  assert(shs1_create_msg3(msg3, client) == 0);
  fwrite(msg3, sizeof(uint8_t), SHS1_MSG3_BYTES, stdout);
  fflush(stdout);

  fread(msg4, sizeof(uint8_t), SHS1_MSG4_BYTES, stdin);
  if (!shs1_verify_msg4(msg4, client)) {
    exit(4);
  }

  shs1_client_outcome(&client_outcome, client);
  uint8_t all_outcome[112];
  memcpy(all_outcome, &(client_outcome.encryption_key), 32);
  memcpy(all_outcome + 32, &(client_outcome.encryption_nonce), 24);
  memcpy(all_outcome + 32 + 24, &(client_outcome.decryption_key), 32);
  memcpy(all_outcome + 32 + 24 + 32, &(client_outcome.decryption_nonce), 24);
  fwrite(all_outcome, sizeof(uint8_t), 112, stdout);
  fflush(stdout);
}
