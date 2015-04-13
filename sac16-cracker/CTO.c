//**********************************************************************
//* File: CTO.c
//*
//* Created: 15th Febuary 2014
//*
//* Purpose: Finds key (from only the cipher text) & decrypts a message
//*          encoded with sac16.
//*			 Requires ciphertext at least two characters long.
//*
//* Author: Hector Grebbell
//*
//* Copyright: Copyright (c) Hector Grebbell 2014
//*
//* The contents of this file should not be copied or distributed
//* without prior permission
//*
//* All rights reserved.
//**********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "freq_analysis.h"
#include "ll.h"

#define BUF_MAX  1048576	// Use something even

//**********************************************************************
//* Runs XOR on data in inbuf with key and places result into outbuf
void decrypt(uint8_t *inbuf, uint8_t *outbuf, uint16_t key, long *buffsize)
{
	long i = 0;
	uint16_t cur = 0;
	
	while (i < *buffsize)
	{
		if ((i+1) == *buffsize)
			break;
		
		cur = inbuf[i+1] | (uint16_t)(inbuf[i]) << 8;
		cur ^= key;
		
		outbuf[i] = cur >> 8;
		outbuf[i+1] = cur & 0xFF;
		
		i+=2;
	}
	if (outbuf[*buffsize-2] == 0)	// Appended by encryption function
		(*buffsize)-=2;
	else if (outbuf[*buffsize-1] == 0)	// Appended by encryption function
		(*buffsize)--;
	
}

//**********************************************************************
//* Reverse Engineer of dice function - Converts XOR Key to sac16 key
//* See Sac16.java
uint16_t genKey(uint16_t rKey)
{
	return (22039 * rKey) % 65536;
}


//**********************************************************************
//* The handout expected us to brute force the key.
//* If not marking exactly to a mark scheme ignore this function.
//* Its only called if the program is called with the -bf flag
//* It tries every key against the first two characters building up a list
//* of the most likely. Then uses frequency analysis to knock out unlikely keys.
//* until the correct key is found
uint16_t bf_key(uint8_t *inbuf, long buffsize, char bf)
{
	struct key_l *h = NULL, *t = NULL, *iter = NULL, *tmp, *bestKey = NULL, *secondBestKey = NULL;
	uint16_t val, scr, cur, keycount = 0 , cutoff = 10;
	uint32_t key;
	long lim = 0;
	uint8_t c1 = 0, c2 = 0, pos = 2, *check = NULL;
		
	if (buffsize < 4)
	{
		printf("I need at least 4 bytes! I mean come on... Outputting garbage\n");
		return 0;
	}
	
	if (buffsize > 40)
		lim = 40;
	else
		lim = buffsize;
	
	if (bf)
	{
		check = (uint8_t*)malloc(sizeof(uint8_t)*lim);
		if (check == NULL)
			printf("Warning: Check malloc failed - Best guess will be used\n");
	}
	
	cur = inbuf[1] | (uint16_t)(inbuf[0]) << 8;
	// Round 1 - for every possible key decrypt the first two characters.
	// Score them based on the frequency data & keep brack of the best.
	for (key = 0; key < 0xFFFF; key++)
	{
		val = cur^key;
		c1 = val >> 8;
		c2 = val & 0xFF;;

		scr = charVal(c1) + charVal(c2) + biVal(val);
		if (scr > 20)
		{
			t = add_key(&h,t,key,scr);
			if (bestKey == NULL || scr > bestKey->score)
			{
				secondBestKey = bestKey;
				bestKey = t;
			}
			keycount++;
		}
	}
	// Subsequent rounds. While we have more than 20 keys
	// And have looked at less than 20 characters & haven't exhausted the buffer
	while (keycount > 20 && pos < 20 && pos < buffsize)
	{
		// Concatenate the two characters.
		cur = inbuf[pos+1] | (uint16_t)(inbuf[pos]) << 8;
		iter = h;
		// Go through all remaining keys
		while (iter != NULL)
		{
			// Check the score for the output characters
			val = cur^iter->key;
			c1 = val >> 8;
			c2 = val & 0xFF;

			scr = charVal(c1) + charVal(c2);
			if (scr > 0)
			{
				scr += biVal(val);
				iter->score += scr;
				if (scr > 20 && iter->score > cutoff)
				{
					if (iter->score > bestKey->score)
					{
						secondBestKey = bestKey;
						bestKey = iter;
					}
				}
				else
				{	
					tmp = iter;
					iter = iter->next;
					if (tmp->key == bestKey->key)
						bestKey = getBest(bestKey->key, &secondBestKey, h);
					remove_key(&h, &t, tmp);
					keycount--;
					continue;
				}
			}
			else
			{
				tmp = iter;
				iter = iter->next;
				if (tmp->key == bestKey->key)
					bestKey = getBest(bestKey->key, &secondBestKey, h);
				remove_key(&h, &t, tmp);
				keycount--;
				continue;
			}
			iter = iter->next;
		}
		if (pos > 7 && bestKey->score > (2*secondBestKey->score))
		{
			printf("Key took %hhu rounds. max = %lu, keycount = %hu (double second highests score)\n", pos/2, bestKey->score, keycount);
			pos+=20;
			break;
		}
		if (check != NULL)
		{
			decrypt(inbuf, check, bestKey->key, &lim);
			c2 = 2;
			for (c1 = 0; c1 < lim && c2; c1++)
			{
				if (charVal(check[c1]) < 35)
					c2--;
			}
			if (c1 != lim)
			{
				printf("Key failed prelim check. Abandoning 0x%X\n", bestKey->key);
				tmp = bestKey;
				bestKey = getBest(bestKey->key, &secondBestKey, h);
				remove_key(&h, &t, tmp);
				keycount--;
			}
			else
			{
				printf("Key took %hhu rounds. max = %lu, keycount = %hu (best key for round check)\n", pos/2, bestKey->score, keycount);
				pos+=20;
				break;
			}
		}
		cutoff*=4;
		pos+=2;
	}
	free(check);
	if (pos <=20)
		printf("Key took %hhu rounds. max = %lu, keycount = %hu (max blocks)\n", pos/2, bestKey->score, keycount);
	iter = h;
	key = bestKey->key;
	while (iter != NULL)
	{
		tmp = iter->next;
		free(iter);
		iter = tmp;
	}
	return key;
}


//**********************************************************************
//* With knowledge of the encryption algorithm we see its vulnerable to
//* frequency analysis. That being that any character value in an even 
//* space will always give the same value (and with odd too).
//* So by finding the most common character in even spaces & the most
//* common character in odd spaces then concatenating them we will almost
//* certainly get the encrypted double space character. This gives us a
//* value which when XOR'd with the cyphertext will yield the plain text.
uint16_t freq_anal(uint8_t *inbuf, long buffsize)
{
	uint8_t p1[256] = {0};
	uint8_t p2[256] = {0};
	uint8_t max1 = 0, max2 = 0, m11 = 0, m21 = 0, *check = NULL, m12 = 0, m22 = 0;
	uint16_t c = 0, key_g = 0;
	long lim = 0;
	if (buffsize < 24)
	{
		printf("Not enough characters to be worth trying freq analysis. Switching to brute force\n");
		return bf_key(inbuf, buffsize, 0);
	}
	else
	{	
		// If we have more than 128 characters only check 128
		if (buffsize > 128)
			lim = 128;
		else
			lim = buffsize;
		check = (uint8_t*)malloc(sizeof(uint8_t)*lim);
		if (check == NULL)
			printf("Warning: Check malloc failed - Best guess will be used\n");
			
		// Find the count of character values in even & odd spaces
		for (; c < lim; c+=2)
		{
			p1[inbuf[c]]+=1;
			p2[inbuf[c+1]]+=1;
		}
		// Find the max values
		for (c = 0; c < 256; c++)
		{
			if (max1 < p1[c])
			{
				max1 = p1[c];
				m12 = m11;
				m11 = c;
			}
			if (max2 < p2[c])
			{
				max2 = p2[c];
				m22 = m22;
				m21 = c;
			}
		}
		// Concatenate & xor with double space to give us the pseudokey
		key_g = (m21 | (uint16_t)(m11) << 8) ^ 0x2020;
		// If we have a check array then verify the key doesn't produce garbage.
		if (check != NULL)
		{
			max1 = 0;
			while (max1 < 4)
			{
				decrypt(inbuf, check, key_g, &lim);
				for (c = 0; c < lim; c++)
					if (!isprint(check[c]) && check[c] != '\t' && check[c] != '\n' && check[c] != '\r')
						break;
				if (c != lim)
				{
					printf("Key failed. Attempting Combination %us of 4\n", max1);
					max1++;
					switch(max1)
					{
						case 1:
							key_g = (m22 | (uint16_t)(m11) << 8) ^ 0x2020;
						case 2:
							key_g = (m21 | (uint16_t)(m12) << 8) ^ 0x2020;
						case 3:
							key_g = (m22 | (uint16_t)(m12) << 8) ^ 0x2020;
					}
				}
				else
					break;
			}
			free(check);
			if (c != lim)
			{
				printf("Key did not pass check. Reverting to Brute Force\n");
				return bf_key(inbuf, buffsize, 0);
			}
		}
		return key_g;
	}
}

//**********************************************************************
//* Cracks the key then Decrypts the file to filename.txt (removes a
//* .bin extention
int handle_file(FILE* fp, char bruteforce, char* fname)
{
	uint8_t *inbuf, *outbuf;
	long size = 0, i = 0, rv = 0, buffsize = 0;
	uint16_t key;
	FILE *of = NULL;
	char* ofname = NULL;
	int len = strlen(fname), ret = EXIT_SUCCESS;
	
	if (!strcmp((fname+len-4), ".bin"))
		fname[len-4] = '\0';
	
	ofname = malloc(sizeof(char)*(strlen(fname)+5));
	strcpy(ofname, fname);
	strcat(ofname, ".txt");
	
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind (fp);
	
	if (size > 1)
	{
		if (size <= BUF_MAX)
			buffsize = size;
		else
			buffsize = BUF_MAX;
		
		inbuf = (uint8_t*)malloc(sizeof(uint8_t)*buffsize);
		if (inbuf == NULL)
		{
			free(ofname);
			perror("malloc");
			return EXIT_FAILURE;
		}
		outbuf = (uint8_t*)malloc(sizeof(uint8_t)*buffsize);
		if (outbuf == NULL)
		{
			free(inbuf);
			free(ofname);
			perror("malloc");
			return EXIT_FAILURE;
		}
		
		i = fread(inbuf, 1, buffsize, fp);
		if (i == buffsize)
		{
			if (bruteforce)
				key = bf_key(inbuf, buffsize, bruteforce-1);
			else
				key = freq_anal(inbuf, buffsize);
				
			printf("RawKey = 0x%X, Key = 0x%X\n", key, genKey(key));
				
			decrypt(inbuf,outbuf,key,&buffsize);
			
			of = fopen(ofname, "w");
			if (fp != NULL)
			{
				rv = fwrite(outbuf, 1, buffsize, of);
				if (rv != buffsize)
				{
					fprintf(stderr, "Write Error\n");
					ret = EXIT_FAILURE;
				}
				else
				{
					while (i < size)
					{
						buffsize = fread(inbuf, 1, buffsize, fp);
						
						decrypt(inbuf,outbuf,key,&buffsize);
						
						rv = fwrite(outbuf, 1, buffsize, of);
						if (rv != buffsize)
						{
							fprintf(stderr, "Write Error\n");
							ret = EXIT_FAILURE;
						}
						
						if (feof(fp))
							break;
						if (ferror(of) || ferror(fp))
						{
							fprintf(stderr, "File Error\n");
							ret = EXIT_FAILURE;
							break;
						}
							
						i+= buffsize;
					}
				}
				fclose (of);
			}
			else
			{
				perror("fopen");
				ret = EXIT_FAILURE;
			}
		}
		else
		{
			fprintf(stderr, "Read Error\n");
			ret = EXIT_FAILURE;
		}
		
		free(inbuf);
		free(outbuf);
	}
	else if (size >= 0)
		printf("Input file is too small\n");
	else
	{
		perror("ftell");
		ret = EXIT_FAILURE;
	}
	free(ofname);
	return ret;
}

//**********************************************************************
//* Argument Handling / Checks the file isn't imaginary
int main(int argc, char*argv[])
{
	char *fname = NULL;
	FILE *fp;
	char bruteforce = 0;
	
	int ret = EXIT_SUCCESS;
	
	if (argc > 1 && !strncmp(argv[1], "--help", 6))
		printf ("\tfilename\tA binary file containing text encrypted with sac16\n\t"
				  "-bf\t\tBrute Force (Don't cheat)\n\t"
				  "-bfnrc\t\tBrute Force Without checking best key every round (use if -bf fails)");
	else if (argc < 2 || argc > 3)
		printf("usage: %s filename [-bf | -bfnrc]\n\t--help for more\n", argv[0]);
	else
	{
		fname = argv[1];
		if (argc == 3)
		{
			if (strcmp(argv[2], "-bfnrc") == 0)
				bruteforce = 1;
			else if (strcmp(argv[2], "-bf") == 0)
				bruteforce = 2;
			else
			{
				bruteforce = -1;
				printf("usage: %s filename plaintext [-bf]\n\t--help for more\n", argv[0]);
			}
		}
		if (bruteforce >= 0)
		{
			fp = fopen(fname, "r");
			if (fp != NULL)
			{
				ret = handle_file(fp, bruteforce, fname);
				fclose(fp);
			}
			else
			{
				ret = EXIT_FAILURE;
				perror("fopen");
			}
		}
	}
	return ret;
}
