#include "../common.h"
#include <stdio.h>

#define MILLER_RABIN_TEST_TIMES 5

BigInt *GeneD(BigInt *e, BigInt *d, BigInt *l)
{
	BigInt k, result, zero, one, y, neg_y;
	char s_d[BIG_INT_BIT_LEN];

	StrToBigInt("0", &zero);
	StrToBigInt("1", &one);

	StrToBigInt("0", &y);


	
	while(1)
	{
			CopyBigInt(&y, d);
			BigIntToStr(d, s_d);
			printf("GeneD, d %s\n", s_d);
			DoExGcd(l, e, &k, d, &result);
			if(DoCompare(d, &zero) > 0)
			{
				break;
			}

			

			if(DoCompare(&y, &zero) == 0)
			{
				DoAdd(&y, &one, &y);
				continue;
			}
			
			DoSub(&zero, &y, &neg_y);
			CopyBigInt(&neg_y, d);
			BigIntToStr(d, s_d);
			printf("GeneD, d %s\n", s_d);
			DoExGcd(l, e, &k, d, &result);
			if(DoCompare(d, &zero) > 0)
			{
				break;
			}


			DoAdd(&y, &one, &y);

			

	}
	
	return d;
}

void DoGenerateRsaKey(int byteLen, char *key_pair_file)
{
	printf("generating rsa key ...\n");

	int i;
	BigInt p, q, e, d, n, pMinusOne, qMinusOne, l, gcd;
	BigInt one;
	char s_result[MAX_STR_SIZE], s_e[MAX_STR_SIZE], s_d[MAX_STR_SIZE], s_n[MAX_STR_SIZE];

	/*  */
	StrToBigInt("1", &one);

	DoGenRandomPrime(byteLen, MILLER_RABIN_TEST_TIMES, &p);	

	do
	{
		DoGenRandomPrime(byteLen, MILLER_RABIN_TEST_TIMES, &q);
	}
	while(DoCompare(&p, &q) == 0);

	/* generate n */
	DoMul(&p, &q, &n);
	BigIntToStr(&n, s_n);

	/* generate l */
	DoSub(&p, &one, &pMinusOne);
	DoSub(&q, &one, &qMinusOne);
	DoLcm(&pMinusOne, &qMinusOne, &l);
 	
	if(RSA_DEBUG)
	{
		printf("p: ");
		BigIntToStr(&p, s_result);
		for(i = 0; i < strlen(s_result); i++)
		{
			printf("%c", s_result[i]);
		}
		printf("\n");

		printf("q: ");
		BigIntToStr(&q, s_result);
		for(i = 0; i < strlen(s_result); i++)
		{
			printf("%c", s_result[i]);
		}
		printf("\n");

		printf("n: ");
		for(i = 0; i < strlen(s_n); i++)
		{
			printf("%c", s_n[i]);
		}
		printf("\n");

		printf("l: ");
		BigIntToStr(&l, s_result);
		for(i = 0; i < strlen(s_result); i++)
		{
			printf("%c", s_result[i]);
		}
		printf("\n");
	}

 	/* generate e */
 	do{
 		/*  */
 		do
 		{
 			DoGetPositiveRand(&l, &e);
 		} while(DoCompare(&e, &one) <= 0);

 		/*  */
 		DoGcd(&e, &l, &gcd);
 	}
 	while(DoCompare(&gcd, &one) != 0);
	BigIntToStr(&e, s_e);

 	/* generate d */
 	GeneD(&e, &d, &l);
 	BigIntToStr(&d, s_d);

 	if(RSA_DEBUG)
	{
		printf("e: ");
		for(i = 0; i < strlen(s_e); i++)
		{
			printf("%c", s_e[i]);
		}
		printf("\n");

		printf("d: ");
		for(i = 0; i < strlen(s_d); i++)
		{
			printf("%c", s_d[i]);
		}
		printf("\n");
	}

	/* generate key pair file */
	char private_file_name[FILE_NAME_LEN];
	snprintf(private_file_name, FILE_NAME_LEN, "%s_private.rsa",  key_pair_file);
	FILE *p_private_file;
	p_private_file = fopen(private_file_name, "wt");
	if(p_private_file == NULL)
	{
		printf("DoGenerateRsaKey, open file %s err\n", private_file_name);
		exit(1);
	}

	char public_file_name[FILE_NAME_LEN];
	snprintf(public_file_name, FILE_NAME_LEN, "%s_public.rsa",  key_pair_file);
	FILE *p_public_file;
	p_public_file = fopen(public_file_name, "wt");
	if(p_public_file == NULL)
	{
		printf("DoGenerateRsaKey, open file %s err\n", public_file_name);
		exit(1);
	}

	if(EOF == fputs(s_e, p_public_file) || EOF == fputc('\n', p_public_file) 
		|| EOF == fputs(s_d, p_private_file) || EOF == fputc('\n', p_private_file) 
		|| EOF == fputs(s_n, p_public_file) || EOF == fputs(s_n, p_private_file))
	{
		printf("DoGenerateRsaKey, write e to file err\n");
		exit(1);
	}

	fclose(p_private_file);
	fclose(p_public_file);
}

static char *Crypt(unsigned char *source, unsigned char *dest, BigInt *key, BigInt *n)
{
	BigInt s, d;

	StrToBigInt(source, &s);

	DoPowMod(&s, key, n, &d);

	return BigIntToStr(&d, dest);
}

char *RsaEncrypt(unsigned char *source, unsigned char *dest, char *key_pair_file)
{
	/*  */
	char public_file_name[FILE_NAME_LEN];
	snprintf(public_file_name, FILE_NAME_LEN, "%s_public.rsa",  key_pair_file);
	FILE *p_public_file;
	p_public_file = fopen(public_file_name, "rt");
	if(p_public_file == NULL)
	{
		printf("RsaEncrypt, open file %s err\n", public_file_name);
		exit(1);
	}

	/*  */
	char buffer[BIG_INT_BIT_LEN];
	BigInt e, n;
	char c;
	int mark = 0;
	
	while((fgets(buffer, BIG_INT_BIT_LEN, p_public_file)) != NULL)
	{
		if(mark == 0)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &e);
		}
		else
		{
			StrToBigInt(buffer, &n);
		}

		mark++;
	}
	if(!feof(p_public_file))
	{
		printf("RsaEncrypt, fgets err %s\n", public_file_name);
		exit(1);
	}
	fclose(p_public_file);

	return Crypt(source, dest, &e, &n);
}

char *RsaDecrypt(unsigned char *source, unsigned char *dest, char *key_pair_file)
{
	/*  */
	char private_file_name[FILE_NAME_LEN];
	snprintf(private_file_name, FILE_NAME_LEN, "%s_private.rsa",  key_pair_file);
	FILE *p_private_file;
	p_private_file = fopen(private_file_name, "rt");
	if(p_private_file == NULL)
	{
		printf("RsaEncrypt, open file %s err\n", private_file_name);
		exit(1);
	}

	/*  */
	char buffer[BIG_INT_BIT_LEN];
	BigInt d, n;
	char c;
	int mark = 0;
	
	while((fgets(buffer, BIG_INT_BIT_LEN, p_private_file)) != NULL)
	{
		if(mark == 0)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &d);
		}
		else
		{
			StrToBigInt(buffer, &n);
		}

		mark++;
	}
	if(!feof(p_private_file))
	{
		printf("RsaEncrypt, fgets err %s\n", private_file_name);
		exit(1);
	}
	fclose(p_private_file);

	return Crypt(source, dest, &d, &n);
}