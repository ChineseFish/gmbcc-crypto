#include "../common.h"

static unsigned char tmp[AES_GROUP_SIZE];
static unsigned char pre_encrypted_text[AES_GROUP_SIZE];

// at CFB mode, the bit sequecne generated by encryption algorithm is called key stream.
// CFB mode is also known as a way to use grouped cipher to realize key stream.
void aes_cfb_encrypt(unsigned char *source)
{
	int i, j;
	int source_size = strnlen(source, MAX_STR_SIZE);

	/*  */
	for(i = 0; i < source_size; i += AES_GROUP_SIZE)
	{
		if(i == 0)
		{
			/*  */
			for(j = 0; j < AES_GROUP_SIZE; j++)
			{
				tmp[j] = init_vector[j];
			}
		}
		else
		{
			/*  */
			for(j = 0; j < AES_GROUP_SIZE; j++)
			{
				tmp[j] = source[i - AES_GROUP_SIZE + j];
			}
		}

		/*  */
		aes_encrypt(tmp);

		/*  */
		for(j = 0; j < AES_GROUP_SIZE &&  i + j < source_size; j++)
		{
			source[i + j] ^= tmp[j];
		}
	}
}

void aes_cfb_decrypt(unsigned char *source)
{
	int i, j;
	int source_size = strnlen(source, MAX_STR_SIZE);

	for(i = 0; i < source_size; i += AES_GROUP_SIZE)
	{
		/*  */
		if(i == 0)
		{
			for(j = 0; j < AES_GROUP_SIZE; j++)
			{
				pre_encrypted_text[j] = init_vector[j];
			}
		}	
		else
		{
			for(j = 0; j < AES_GROUP_SIZE; j++)
			{
				pre_encrypted_text[j] = tmp[j];
			}
		}

		/*  */
		aes_encrypt(pre_encrypted_text);

		/*  */
		for(j = 0; j < AES_GROUP_SIZE &&  i + j < source_size; j++)
		{
			tmp[j] = source[i + j];
		}

		/*  */
		for(j = 0; j < AES_GROUP_SIZE &&  i + j < source_size; j++)
		{
			source[i + j] ^= pre_encrypted_text[j];
		}
	}
}

// 0 ^ 1 = 1
// 0 ^ 0 = 0
// 1 ^ 0 = 1
// 1 ^ 1 = 0