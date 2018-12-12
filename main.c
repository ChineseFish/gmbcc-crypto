#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./common.h"
#include "./hash/sha256.h"

static int source_index = 0;
static unsigned char source[MAX_STR_SIZE];

int main()
{
  // rsa_test();

  ecc_test();

  // sha256_test();

  return 0;
}


void sha256_test()
{
  char *buf[] = {
        "",
        "e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855",

        "abc",
        "ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad",

        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        "248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1",

        "The quick brown fox jumps over the lazy dog",
        "d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592",

        "The quick brown fox jumps over the lazy cog", /* avalanche effect test */
        "e4c4d8f3 bf76b692 de791a17 3e053211 50f7a345 b46484fe 427f6acc 7ecc81be",

        "bhn5bjmoniertqea40wro2upyflkydsibsk8ylkmgbvwi420t44cq034eou1szc1k0mk46oeb7ktzmlxqkbte2sy",
        "9085df2f 02e0cc45 5928d0f5 1b27b4bf 1d9cd260 a66ed1fd a11b0a3f f5756d99"
    };

    uint8_t hash[SHA256_BYTES];
    size_t i, j;

    for (i = 0; i < (sizeof(buf) / sizeof(buf[0])); i += 2) {
        sha256(buf[i], strlen(buf[i]), hash);
        printf("input = '%s'\ndigest: %s\nresult: ", buf[i], buf[i + 1]);
        for (j = 0; j < SHA256_BYTES; j++)
            printf("%02x%s", hash[j], ((j % 4) == 3) ? " " : "");
        printf("\n\n");
    }
}

void rsa_test()
{
  int i;
  
  DoGenerateRsaKey(1, "key_pair");

  unsigned char dest[MAX_STR_SIZE];

  RsaEncrypt("56", dest, "key_pair");
  for(i = 0; i < strlen(dest); i++)
  {
   printf("%c", dest[i]);
  }
  printf("\n");

  RsaDecrypt(dest, source, "key_pair");
  for(i = 0; i < strlen(source); i++)
  {
   printf("%c", source[i]);
  }
  printf("\n");


  RsaSign("56", dest, "key_pair");
  for(i = 0; i < strlen(dest); i++)
  {
    printf("%c", dest[i]);
  }
  printf("\n");

  RsaVerifySign(dest, source, "key_pair");
  for(i = 0; i < strlen(source); i++)
  {
    printf("%c", source[i]);
  }
  printf("\n");
}

void ecc_test()
{
  /*  */
  GenerateEccKey("key_pair");
  return;
  /*  */
  char text[BIG_INT_BIT_LEN] = "bhn5bjmoniertqea40wro2upyflkydsibsk8ylkmgbvwi420t44cq034eou1szc1k0mk46oeb7ktzmlxqkbte2sy";
  char r[BIG_INT_BIT_LEN];
  char s[BIG_INT_BIT_LEN];

  /* sha256 */
  uint8_t hash[SHA256_BYTES];
  int i;
  sha256(text, strnlen(text, BIG_INT_BIT_LEN), hash);
  printf("hash: ");
  for(i = 0; i < SHA256_BYTES; i++)
  {
    printf("%02x ", hash[i]);
  }
  printf("\n\n");

  EccSign(1, hash, "key_pair", r, s);
  printf("r: ");
  for(i = 0; i < strnlen(r, BIG_INT_BIT_LEN); i++)
  {
    printf("%02x ", r[i]);
  }
  printf("\n");
  printf("s: ");
  for(i = 0; i < strnlen(s, BIG_INT_BIT_LEN); i++)
  {
    printf("%02x ", s[i]);
  }
  printf("\n\n");

  printf("VerifySign: %d\n", EccVerifySign(hash, "key_pair", r, s));
}