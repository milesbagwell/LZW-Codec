/*******************************************************************************
** Miles Bagwell
** ECE 4680 Spring 2016
**
** Lab 3 LZW Codec
*******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COUNT 255
#define MAX_SIZE 105536

int main(int argc, char **argv){
	char *fileName = NULL, *fileOut = NULL;
	FILE *fin,*fout;
	float diff;
	int i,j,k;
	int found,end;
	int comp = 0, decomp = 0;
	int fsize,csize, dsize, psize;
	unsigned char *dLength;
	unsigned char **dictionary;
	unsigned char *prev, current,*pc;
	unsigned char *X, Y, Z, *XYZ;
	unsigned short pIndex, P,C;

	if (argc != 4){
			printf("Usage: lab2 -c <file to compress>   <output file name>\n");
			printf("       lab2 -d <file to decompress> <output file name>\n");
			exit(1);
	}

	j = getopt(argc, argv, "c:d");

		switch (j){
			case 'c':
				comp = 1;
				fileName = argv[2];
				fileOut  = argv[3];
				break;
			case 'd':
				decomp = 1;
				fileName = argv[2];
				fileOut  = argv[3];
				break;
			case '?':
				if (isprint(optopt))
					fprintf(stderr, "Unknown option %c.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			default:
				printf("Usage: lab3 -c <file to compress> <output file name>\n");
				printf("       lab3 -d <file to decompress> <output file name>\n");
				exit(1);
		}


	fin = fopen(fileName, "rb");
	fout = fopen(fileOut, "wb");

	if (fin == NULL){
		printf("Error Reading File\n");
		exit(1);
	}

	fseek(fin,0,SEEK_END);
	fsize = ftell(fin);
	fseek(fin,0,SEEK_SET);

	// Initialize Dictionary	
	dictionary = (unsigned char **) calloc(MAX_SIZE,sizeof(unsigned short));
	dLength = (unsigned char *) calloc(MAX_SIZE,sizeof(unsigned short));
	for (i = 0; i < 256; i++){
		dictionary[i] = malloc(1);
		dictionary[i][0] = i;
		dLength[i] = 1;
	}

	dsize = 256;
		
	// Compression
	if (comp == 1){ 
		printf("Compressing %s\n",fileName);

		prev = malloc(1);
		fread(prev,1,1,fin);
		pIndex = (unsigned short) prev[0];
		psize = 1;

		end = 1;
		while(end == 1){
			end = fread(&current,1,1,fin);	

			if (end == 0){
				break;
			}

			pc = malloc(psize+1);

			if(pc == NULL){
				printf("Malloc Failed\n");
				exit(2);
			}

			memcpy(pc, prev, psize);
			memcpy((pc+psize), &current, 1);

			// Look if P + C is in the dictionary
			found = 0;
			for (j = 256; j < dsize; j++){
				if ((psize + 1) == dLength[j]){
					for (k = 0; k < psize + 1; k++){
						if (pc[k] == dictionary[j][k]){
							if (k == psize){
								found = 1;
								pIndex = (unsigned short) j; 
							}
						}
						else 
							break;	
					}

				}
				if (found == 1)
					break;
			}


			// P+C in Dictionary
			if (found == 1){
				psize++;
				free(prev);
				prev = calloc(psize,1);
				memcpy(prev, pc, psize);
			}

			// P+C not in Dictionary
			else {
				fwrite(&pIndex,2,1,fout);
				dictionary[dsize] = malloc(psize+1);
				memcpy(dictionary[dsize], pc, psize+1);
				dLength[dsize] = psize+1;
				psize = 1;
				free(prev);
				prev = calloc(1,1);
				prev[0] = current;
				pIndex = (unsigned short) current;
				dsize++;
			}
			free(pc);
		}

		fwrite(&pIndex,2,1,fout);

		csize = ftell(fout);
		printf("Original Size   = %d\n",fsize);
		printf("Compressed Size = %d\n",csize);
		diff = (((float)(csize-fsize))/(float)(fsize))*100;
		printf("%.2f%% Compression\n",diff);

	}


	// Decompression
	else if (decomp == 1){ 

		printf("Decompressing %s\n",fileName);
		C = 0;
		fread(&C,2,1,fin);
		fwrite(dictionary[C],1,1,fout);

		end = 1;

		while(end == 1){
			P = C;
			psize = dLength[P];

			end = fread(&C,2,1,fin);
			if (end == 0)
				break;

			X = (unsigned char *)calloc(dLength[P],sizeof(unsigned char));
			XYZ = (unsigned char *) calloc(dLength[P]+1,sizeof(unsigned char));

			memcpy(X,dictionary[P],dLength[P]);
			
			// C in Dictionary
			if (C < dsize){
				fwrite(dictionary[C],1,dLength[C],fout);
				Y = dictionary[C][0];
				memcpy(XYZ,X,dLength[P]);
				memcpy(XYZ+dLength[P],&Y,1);
		
			}
			
			// C not in Dictionary
			else{
				Z = dictionary[P][0];
				memcpy(XYZ,X,dLength[P]);
				memcpy(XYZ+dLength[P],&Z,1);	
				fwrite(XYZ,1,psize+1,fout);
			}
			
			// Add XY or XZ to Dictionary
			dictionary[dsize] = calloc(psize+1, sizeof(unsigned char));
			memcpy(dictionary[dsize], XYZ, psize + 1);
			dLength[dsize] = psize + 1;
			dsize++;
			free(X);
			free(XYZ);
		}
	}

	fclose(fin);
	fclose(fout);
	return 0;
}
