/*
secp256k1
The curve I've chosen is secp256k1, from SECG (the "Standards for Efficient Cryptography Group", founded by Certicom). This same curve is also used by Bitcoin for digital signatures. 
y^2 = (x ^ 3 + a * x + b) mod p (p  is a prime)

*/

#include "../common.h"
#include <stdio.h>

static BigInt P;
static BigInt A;
static BigInt B;
static BigInt X_G;
static BigInt Y_G;
static BigInt N; /* the order of discrete ellipse curve, N * p = 0 (p is a random point which at descrete ellipse curve) */
static BigInt H; /* n * h = N, n * ( h * p) = 0, G = h * p (p is a random point which at descrete ellipse curve) */

void InitDomainParameters()
{
	char s_decimal_p[BIG_INT_BIT_LEN];
	char s_tmp_1[BIG_INT_BIT_LEN];
	char s_tmp_2[BIG_INT_BIT_LEN];
	ChangeStringRadix("fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f", 16, 10, s_decimal_p);
	StrToBigInt(s_decimal_p, &P);

	StrToBigInt("0", &A);

	StrToBigInt("7", &B);

	char s_decimal_x_g[BIG_INT_BIT_LEN];
	ChangeStringRadix("79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798", 16, 10, s_decimal_x_g);
	StrToBigInt(s_decimal_x_g, &X_G);

	char s_decimal_y_g[BIG_INT_BIT_LEN];
	ChangeStringRadix("483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8", 16, 10, s_decimal_y_g);
	StrToBigInt(s_decimal_y_g, &Y_G);

	char s_decimal_n[BIG_INT_BIT_LEN];
	ChangeStringRadix("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141", 16, 10, s_decimal_n);
	StrToBigInt(s_decimal_n, &N);

	StrToBigInt("1", &H);
}

BigInt *GeneMulReverse(BigInt *a, BigInt *b, BigInt *x, BigInt *mul_reverse)
{
	BigInt result, zero, one, y, neg_y;

	StrToBigInt("0", &zero);
	StrToBigInt("1", &one);

	StrToBigInt("0", &y);
	
	while(1)
	{
			CopyBigInt(&y, mul_reverse);
			DoExGcd(a, b, x, mul_reverse, &result);
			if(DoCompare(mul_reverse, &zero) > 0)
			{
				break;
			}

			if(DoCompare(&y, &zero) == 0)
			{
				DoAdd(&y, &one, &y);
				continue;
			}
			
			DoSub(&zero, &y, &neg_y);
			CopyBigInt(&neg_y, mul_reverse);
			DoExGcd(a, b, x, mul_reverse, &result);
			if(DoCompare(mul_reverse, &zero) > 0)
			{
				break;
			}


			DoAdd(&y, &one, &y);
	}
	
	return mul_reverse;
}

static void ComputeMG(BigInt *private_key, BigInt *p_x, BigInt *p_y, BigInt *origin_x, BigInt *origin_y)
{
	BigInt tmp, tmp_1, tmp_2, m, r_x, r_y;
	BigInt i, zero, one, two, three;

	char s_tmp_1[BIG_INT_BIT_LEN];
	char s_tmp_2[BIG_INT_BIT_LEN];

	StrToBigInt("0", &i);
	StrToBigInt("0", &zero);
	StrToBigInt("1", &one);
	StrToBigInt("2", &two);
	StrToBigInt("3", &three);

	/* init origin point */
	CopyBigInt(origin_x, p_x);
	CopyBigInt(origin_y, p_y);

	while(DoCompare(&i, private_key) < 0)
	{
		/* compute slope m */
		DoPow(p_x, &two, &tmp_1);
		DoMul(&three, &tmp_1, &tmp_1);
		DoAdd(&tmp_1, &A, &tmp_1);
		/*  */
		DoMul(p_y, &two, &tmp_2);
		if(DoCompare(&P, &tmp_2) > 0)
		{
			GeneMulReverse(&P, &tmp_2, &tmp, &tmp_2);
		}
		else
		{
			GeneMulReverse(&tmp_2, &P, &tmp_2, &tmp);
		}

		/*  */
		DoMul(&tmp_1, &tmp_2, &tmp_1);
		DoMod(&tmp_1, &P, &m);
		if(DoCompare(&m, &zero) < 0)
		{
			DoAdd(&m, &P, &m);
		}

		/* compute x */
		DoPow(&m, &two, &tmp_1);
		DoMul(p_x, &two, &tmp_2);
		DoSub(&tmp_1, &tmp_2, &tmp_1);
		DoMod(&tmp_1, &P, &r_x);
		if(DoCompare(&r_x, &zero) < 0)
		{
			DoAdd(&r_x, &P, &r_x);
		}

		/* compute y */
		DoSub(&r_x, p_x, &tmp_1);
		DoMul(&tmp_1, &m, &tmp_1);
		DoAdd(&tmp_1, p_y, &tmp_1);
		DoMod(&tmp_1, &P, &r_y);
		if(DoCompare(&r_y, &zero) < 0)
		{
			DoAdd(&r_y, &P, &r_y);
		}

		/*  */
		CopyBigInt(&r_x, p_x);
		CopyBigInt(&r_y, p_y);

		if(ECC_DEBUG)
		{
			BigIntToStr(&r_x, s_tmp_1);
			BigIntToStr(&r_y, s_tmp_2);

			printf("r_x: %s\nr_y: %s\n\n", s_tmp_1, s_tmp_2);
		}

		/*  */
		DoAdd(&i, &one, &i);
	}
}

static void ComputeXGAddYG(BigInt *x_p, BigInt *y_p, BigInt *x_q, BigInt *y_q, BigInt *result_x, BigInt *result_y)
{
	BigInt tmp, tmp_1, tmp_2, m;
	BigInt zero, two;

	char s_tmp_1[BIG_INT_BIT_LEN];
	char s_tmp_2[BIG_INT_BIT_LEN];

	StrToBigInt("0", &zero);
	StrToBigInt("2", &two);

	/* compute slope m */
	DoSub(&y_p, &y_q, &tmp_1);
	DoSub(&x_p, &x_q, &tmp_2);
	if(DoCompare(&P, &tmp_2) > 0)
	{
		GeneMulReverse(&P, &tmp_2, &tmp, &tmp_2);
	}
	else
	{
		GeneMulReverse(&tmp_2, &P, &tmp_2, &tmp);
	}
	
	DoMul(&tmp_1, &tmp_2, &tmp_1);
	DoMod(&tmp_1, &P, &m);
	if(DoCompare(&m, &zero) < 0)
	{
		DoAdd(&m, &P, &m);
	}

	/* compute x */
	DoPow(&m, &two, &tmp_1);
	DoSub(&tmp_1, &x_p, &tmp_1);
	DoSub(&tmp_1, &x_q, &tmp_1);
	DoMod(&tmp_1, &P, &result_x);
	if(DoCompare(&result_x, &zero) < 0)
	{
		DoAdd(&result_x, &P, &result_x);
	}

	/* compute y */
	DoSub(&result_x, x_p, &tmp_1);
	DoMul(&tmp_1, &m, &tmp_1);
	DoAdd(&tmp_1, y_p, &tmp_1);
	DoMod(&tmp_1, &P, &result_y);
	if(DoCompare(&result_y, &zero) < 0)
	{
		DoAdd(&result_y, &P, &result_y);
	}

	if(ECC_DEBUG)
	{
		BigIntToStr(&result_x, s_tmp_1);
		BigIntToStr(&result_y, s_tmp_2);

		printf("result_x: %s\nresult_y: %s\n\n", s_tmp_1, s_tmp_2);
	}
}

void GenerateEccKey(int byteLen, char *key_pair_file)
{
	printf("begin to generate ecc key ...\n");

	InitDomainParameters();

	BigInt p_x, p_y, private_key;

	char s_private_key[BIG_INT_BIT_LEN], s_public_key_x[BIG_INT_BIT_LEN], s_public_key_y[BIG_INT_BIT_LEN], s_g_x[BIG_INT_BIT_LEN], s_g_y[BIG_INT_BIT_LEN];
	char s_tmp_1[BIG_INT_BIT_LEN];
	char s_tmp_2[BIG_INT_BIT_LEN];

	/*  */
	BigIntToStr(&X_G, s_g_x);
	BigIntToStr(&Y_G, s_g_y);

	/* init private key */
	DoGetPositiveRandBigInt(byteLen, &private_key);
	DoMod(&private_key, &P, &private_key);
	BigIntToStr(&private_key, &s_private_key);

	if(ECC_DEBUG)
	{
		BigIntToStr(&p_x, s_tmp_1);
		BigIntToStr(&p_y, s_tmp_2);
		printf("origin p_x: %s\norigin p_y: %s\n\n", s_tmp_1, s_tmp_2);

		printf("private_key: %s\n\n", s_private_key);
	}
	
	ComputeMG(&private_key, &p_x, &p_y, &X_G, &Y_G);

	/*  */
	BigIntToStr(&p_x, s_public_key_x);
	BigIntToStr(&p_y, s_public_key_y);

	/* generate key pair file */
	char private_file_name[FILE_NAME_LEN];
	snprintf(private_file_name, FILE_NAME_LEN, "%s_private.ecc",  key_pair_file);
	FILE *p_private_file;
	p_private_file = fopen(private_file_name, "wt");
	if(p_private_file == NULL)
	{
		printf("GenerateEccKey, open file %s err\n", private_file_name);
		exit(1);
	}

	char public_file_name[FILE_NAME_LEN];
	snprintf(public_file_name, FILE_NAME_LEN, "%s_public.ecc",  key_pair_file);
	FILE *p_public_file;
	p_public_file = fopen(public_file_name, "wt");
	if(p_public_file == NULL)
	{
		printf("GenerateEccKey, open file %s err\n", public_file_name);
		exit(1);
	}

	if(EOF == fputs(s_public_key_x, p_public_file) || EOF == fputc('\n', p_public_file) 
		|| EOF == fputs(s_public_key_y, p_public_file) || EOF == fputc('\n', p_public_file) 
		|| EOF == fputs(s_private_key, p_private_file)  || EOF == fputc('\n', p_private_file)
		|| EOF == fputs(s_public_key_x, p_private_file)  || EOF == fputc('\n', p_private_file) 
		|| EOF == fputs(s_public_key_y, p_private_file)  || EOF == fputc('\n', p_private_file)  
		|| EOF == fputs(s_g_x, p_public_file) || EOF == fputc('\n', p_public_file) 
		|| EOF == fputs(s_g_y, p_public_file) || EOF == fputc('\n', p_public_file)
		|| EOF == fputs(s_g_x, p_private_file) || EOF == fputc('\n', p_private_file) 
		|| EOF == fputs(s_g_y, p_private_file) || EOF == fputc('\n', p_private_file))
	{
		printf("GenerateEccKey, write e to file err\n");
		exit(1);
	}

	fclose(p_private_file);
	fclose(p_public_file);
}

void EccSign(int byteLen, char *s_source, int s_source_len, char *key_pair_file, char *s_r, char *s_s)
{
	printf("begin to sign ...\n");

	InitDomainParameters();

	/*  */
	char private_file_name[FILE_NAME_LEN];
	snprintf(private_file_name, FILE_NAME_LEN, "%s_private.ecc",  key_pair_file);
	FILE *p_private_file;
	p_private_file = fopen(private_file_name, "rt");
	if(p_private_file == NULL)
	{
		printf("EccSign, open file %s err\n", private_file_name);
		exit(1);
	}

	/*  */
	char buffer[BIG_INT_BIT_LEN];
	BigInt private_key, public_p_x, public_p_y, origin_p_x, origin_p_y;
	char c;
	int mark = 0;
	
	while((fgets(buffer, BIG_INT_BIT_LEN, p_private_file)) != NULL)
	{
		if(mark == 0)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &private_key);
		}
		else if(mark == 1)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &public_p_x);
		}
		else if(mark == 2)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &public_p_y);
		}
		else if(mark == 3)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &origin_p_x);
		}
		else
		{
			StrToBigInt(buffer, &origin_p_y);
		}

		mark++;
	}
	if(!feof(p_private_file))
	{
		printf("EccSign, fgets err %s\n", private_file_name);
		exit(1);
	}
	fclose(p_private_file);

	/* sign */
	BigInt source, tmp, p_x, p_y, zero, left, right, k, r, s, truncated_hash;

	ChangeStringRadix(s_source, 16, 10, s_source);
	StrToBigInt(s_source, &source);
	StrToBigInt("0", &zero);

	/* compute truncated_hash */
	DoMod(&source, &N, &truncated_hash);

	do
	{
		do
		{
			/* init k */
			DoGetPositiveRandBigInt(byteLen, &k);
			DoMod(&k, &P, &k);
			/* compute kG */
			ComputeMG(&k, &p_x, &p_y, &X_G, &Y_G);
			/* compute r */
			DoMod(&p_x, &N, &r);
		}
		while(DoCompare(&r, &zero) == 0);

		/* compute k ^ -1 */
		if(DoCompare(&k, &N) > 0)
		{
			GeneMulReverse(&k, &N, &left, &tmp);
		}
		else
		{
			GeneMulReverse(&N, &k, &tmp, &left);
		}

		/* compute right */
		DoMul(&r, &private_key, &tmp);
		DoAdd(&tmp, &truncated_hash, &right);

		/*  */
		DoMul(&left, &right, &tmp);
		DoMod(&tmp, &N, &s);
	}
	while(DoCompare(&s, &zero) == 0);

	BigIntToStr(&r, s_r);
	BigIntToStr(&s, s_s);
}

int EccVerifySign(char *s_source, int s_source_len, char *key_pair_file, char *s_r, char *s_s)
{
	InitDomainParameters();

	/*  */
	char public_file_name[FILE_NAME_LEN];
	snprintf(public_file_name, FILE_NAME_LEN, "%s_public.ecc",  key_pair_file);
	FILE *p_public_file;
	p_public_file = fopen(public_file_name, "rt");
	if(p_public_file == NULL)
	{
		printf("EccVerifySign, open file %s err\n", public_file_name);
		exit(1);
	}

	/*  */
	char buffer[BIG_INT_BIT_LEN];
	BigInt public_p_x, public_p_y, origin_p_x, origin_p_y;
	char c;
	int mark = 0;
	
	while((fgets(buffer, BIG_INT_BIT_LEN, p_public_file)) != NULL)
	{
		if(mark == 0)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &public_p_x);
		}
		else if(mark == 1)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &public_p_y);
		}

		else if(mark == 2)
		{
			int real_size = strnlen(buffer, BIG_INT_BIT_LEN) - SLASH_N_SIZE;
			buffer[real_size] = '\0';

			StrToBigInt(buffer, &origin_p_x);
		}

		else if(mark == 3)
		{
			StrToBigInt(buffer, &origin_p_y);
		}

		mark++;
	}
	if(!feof(p_public_file))
	{
		printf("EccVerifySign, fgets err %s\n", public_file_name);
		exit(1);
	}
	fclose(p_public_file);

	/* verify */
	BigInt source, tmp, truncated_hash, s, r, s_reverse, v1, v2, x_p, y_p, x_q, y_q, result_x, result_y;

	ChangeStringRadix(s_source, 16, 10, s_source);
	StrToBigInt(s_source, &source);
	StrToBigInt(s_s, &s);
	StrToBigInt(s_r, &r);

	/* compute truncated_hash */
	DoMod(&source, &N, &truncated_hash);	

	/*  */
	if(DoCompare(&s, &N) > 0)
	{
		GeneMulReverse(&s, &N, &s_reverse, &tmp);
	}
	else
	{
		GeneMulReverse(&N, &s, &tmp, &s_reverse);
	}

	/* compute v1 */
	DoMul(&s_reverse, &truncated_hash, &tmp);
	DoMod(&tmp, &N, &v1);

	/* compute v2 */
	DoMul(&s_reverse, &r, &tmp);
	DoMod(&tmp, &N, &v2);

	/* compute left */
	ComputeMG(&v1, &x_p, &y_p, &X_G, &Y_G);

	/* compute right */
	ComputeMG(&v2, &x_q, &y_q, &public_p_x, &public_p_y);

	/*  */
	ComputeXGAddYG(&x_p, &y_p, &x_q, &y_q, &result_x, &result_y);

	/*  */
	DoMod(&result_x, &N, &tmp);

	if(DoCompare(&tmp, &r) == 0)
	{
		return 1;
	}
	return 0;
}