//
//  GameSave.cpp
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <bzlib.h>
#include "Config.h"
#include "bson/BSON.h"
#include "GameSave.h"
#include "simulation/SimulationData.h"

GameSave::GameSave(GameSave & save) :
waterEEnabled(save.waterEEnabled),
legacyEnable(save.legacyEnable),
gravityEnable(save.gravityEnable),
paused(save.paused),
gravityMode(save.gravityMode),
airMode(save.airMode),
signs(save.signs)
{
	setSize(save.width, save.height);

	particlesCount = save.particlesCount;
	copy(save.particles, save.particles+NPART, particles);
	copy(save.blockMapPtr, save.blockMapPtr+((height/CELL)*(width/CELL)), blockMapPtr);
	copy(save.fanVelXPtr, save.fanVelXPtr+((height/CELL)*(width/CELL)), fanVelXPtr);
	copy(save.fanVelYPtr, save.fanVelYPtr+((height/CELL)*(width/CELL)), fanVelYPtr);
}

GameSave::GameSave(int width, int height)
{
	setSize(width, height);
}

GameSave::GameSave(char * data, int dataSize)
{
	width, height = 0;
	blockMap, blockMapPtr, fanVelX, fanVelXPtr, fanVelY, fanVelYPtr, particles = NULL;
	try {
		readPSv(data, dataSize);
	} catch (ParseException& e) {
		this->~GameSave();	//Free any allocated memory
		throw;
	}
}

void GameSave::setSize(int newWidth, int newHeight)
{
	this->width = (newWidth/CELL)*CELL;
	this->height = (newHeight/CELL)*CELL;
	
	particlesCount = 0;
	particles = new Particle[NPART];
	blockMap = new unsigned char*[height/CELL];
	blockMapPtr = new unsigned char[(height/CELL)*(width/CELL)];
	fill(blockMapPtr, blockMapPtr+((height/CELL)*(width/CELL)), 0);
	for(int y = 0; y < height/CELL; y++)
		blockMap[y] = &blockMapPtr[y*(width/CELL)];
	fanVelXPtr = new float[(height/CELL)*(width/CELL)];
	fill(fanVelXPtr, fanVelXPtr+((height/CELL)*(width/CELL)), 0);
	fanVelX = new float*[height/CELL];
	for(int y = 0; y < height/CELL; y++)
		fanVelX[y] = &fanVelXPtr[y*(width/CELL)];
	fanVelYPtr = new float[(height/CELL)*(width/CELL)];
	fill(fanVelYPtr, fanVelYPtr+((height/CELL)*(width/CELL)), 0);
	fanVelY = new float*[height/CELL];
	for(int y = 0; y < height/CELL; y++)
		fanVelY[y] = &fanVelYPtr[y*(width/CELL)];
}

char * GameSave::Serialise(int & dataSize)
{
	return serialiseOPS(dataSize);
}

void GameSave::Transform(matrix2d transform, vector2d translate)
{
	
}

void GameSave::readOPS(char * data, int dataLength)
{
	unsigned char * inputData = (unsigned char *)data, *bsonData = NULL, *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL;
	unsigned int inputDataLen = dataLength, bsonDataLen = 0, partsDataLen, partsPosDataLen, fanDataLen, wallDataLen;
	int i, freeIndicesCount, x, y, j;
	int *freeIndices = NULL;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	bson b;
	bson_iterator iter;
	
	//Block sizes
	blockX = 0;
	blockY = 0;
	blockW = inputData[6];
	blockH = inputData[7];
	
	//Full size, normalised
	fullX = blockX*CELL;
	fullY = blockY*CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;
	
	//From newer version
	if(inputData[4] > SAVE_VERSION)
		throw ParseException(ParseException::WrongVersion, "Save from newer version");
	
	//Incompatible cell size
	if(inputData[5] > CELL)
		throw ParseException(ParseException::InvalidDimensions, "Incorrect CELL size");
	
	//Too large/off screen
	if(blockX+blockW > XRES/CELL || blockY+blockH > YRES/CELL)
		throw ParseException(ParseException::InvalidDimensions, "Save too large");
	
	setSize(fullW, fullH);
	
	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;
	
	bsonData = (unsigned char*)malloc(bsonDataLen+1);
	if(!bsonData)
		throw ParseException(ParseException::InternalError, "Unable to allocate memory");
		
	//Make sure bsonData is null terminated, since all string functions need null terminated strings
	//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
	bsonData[bsonDataLen] = 0;
	
	if (BZ2_bzBuffToBuffDecompress((char*)bsonData, &bsonDataLen, (char*)(inputData+12), inputDataLen-12, 0, 0))
		throw ParseException(ParseException::Corrupt, "Unable to decompress");
	
	bson_init_data(&b, (char*)bsonData);
	bson_iterator_init(&iter, &b);
	
	std::vector<sign> tempSigns;
	
	while(bson_iterator_next(&iter))
	{
		if(strcmp(bson_iterator_key(&iter), "signs")==0)
		{
			if(bson_iterator_type(&iter)==BSON_ARRAY)
			{
				bson_iterator subiter;
				bson_iterator_subiterator(&iter, &subiter);
				while(bson_iterator_next(&subiter))
				{
					if(strcmp(bson_iterator_key(&subiter), "sign")==0)
					{
						if(bson_iterator_type(&subiter)==BSON_OBJECT)
						{
							bson_iterator signiter;
							bson_iterator_subiterator(&subiter, &signiter);
							
							sign tempSign("", 0, 0, sign::Left);
							while(bson_iterator_next(&signiter))
							{
								if(strcmp(bson_iterator_key(&signiter), "text")==0 && bson_iterator_type(&signiter)==BSON_STRING)
								{
									tempSign.text = bson_iterator_string(&signiter);
									clean_text((char*)tempSign.text.c_str(), 158-14);
								}
								else if(strcmp(bson_iterator_key(&signiter), "justification")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.ju = (sign::Justification)bson_iterator_int(&signiter);
								}
								else if(strcmp(bson_iterator_key(&signiter), "x")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.x = bson_iterator_int(&signiter)+fullX;
								}
								else if(strcmp(bson_iterator_key(&signiter), "y")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									tempSign.y = bson_iterator_int(&signiter)+fullY;
								}
								else
								{
									fprintf(stderr, "Unknown sign property %s\n", bson_iterator_key(&signiter));
								}
							}
							tempSigns.push_back(tempSign);
						}
						else
						{
							fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&subiter));
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "parts")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		if(strcmp(bson_iterator_key(&iter), "partsPos")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsPosDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsPosData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle position data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "wallMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (wallDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				wallData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of wall data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "fanMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (fanDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				fanData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of fan data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "legacyEnable")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				legacyEnable = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityEnable")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				gravityEnable = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "waterEEnabled")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				waterEEnabled = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "paused")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				paused = bson_iterator_bool(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityMode")==0)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				gravityMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "airMode")==0)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				airMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
	}
	
	//Read wall and fan data
	if(wallData)
	{
		j = 0;
		if(blockW * blockH > wallDataLen)
		{
			fprintf(stderr, "Not enough wall data\n");
			goto fail;
		}
		for(x = 0; x < blockW; x++)
		{
			for(y = 0; y < blockH; y++)
			{
				if (wallData[y*blockW+x])
					blockMap[blockY+y][blockX+x] = wallData[y*blockW+x];
				if (wallData[y*blockW+x] == WL_FAN && fanData)
				{
					if(j+1 >= fanDataLen)
					{
						fprintf(stderr, "Not enough fan data\n");
					}
					fanVelX[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
					fanVelY[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
				}
			}
		}
	}
	
	//Read particle data
	if(partsData && partsPosData)
	{
		int newIndex = 0, fieldDescriptor, tempTemp;
		int posCount, posTotal, partsPosDataIndex = 0;
		int saved_x, saved_y;
		if(fullW * fullH * 3 > partsPosDataLen)
		{
			fprintf(stderr, "Not enough particle position data\n");
			goto fail;
		}
		i = 0;
		newIndex = 0;
		for (saved_y=0; saved_y<fullH; saved_y++)
		{
			for (saved_x=0; saved_x<fullW; saved_x++)
			{
				//Read total number of particles at this position
				posTotal = 0;
				posTotal |= partsPosData[partsPosDataIndex++]<<16;
				posTotal |= partsPosData[partsPosDataIndex++]<<8;
				posTotal |= partsPosData[partsPosDataIndex++];
				//Put the next posTotal particles at this position
				for (posCount=0; posCount<posTotal; posCount++)
				{
					particlesCount = newIndex+1;
					if(newIndex>=NPART)
					{
						goto fail;
					}
					
					//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
					if (i+3 >= partsDataLen)
						goto fail;
					x = saved_x + fullX;
					y = saved_y + fullY;
					fieldDescriptor = partsData[i+1];
					fieldDescriptor |= partsData[i+2] << 8;
					if(x >= XRES || x < 0 || y >= YRES || y < 0)
					{
						fprintf(stderr, "Out of range [%d]: %d %d, [%d, %d], [%d, %d]\n", i, x, y, (unsigned)partsData[i+1], (unsigned)partsData[i+2], (unsigned)partsData[i+3], (unsigned)partsData[i+4]);
						goto fail;
					}
					if(partsData[i] >= PT_NUM)
						partsData[i] = PT_DMND;	//Replace all invalid elements with diamond

					if(newIndex < 0 || newIndex >= NPART)
						goto fail;
					
					//Clear the particle, ready for our new properties
					memset(&(particles[newIndex]), 0, sizeof(Particle));
					
					//Required fields
					particles[newIndex].type = partsData[i];
					particles[newIndex].x = x;
					particles[newIndex].y = y;
					i+=3;
					
					//Read temp
					if(fieldDescriptor & 0x01)
					{
						//Full 16bit int
						tempTemp = partsData[i++];
						tempTemp |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].temp = tempTemp;
					}
					else
					{
						//1 Byte room temp offset
						tempTemp = (char)partsData[i++];
						particles[newIndex].temp = tempTemp+294.15f;
					}
					
					//Read life
					if(fieldDescriptor & 0x02)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].life = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x04)
						{
							if(i >= partsDataLen) goto fail;
							particles[newIndex].life |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read tmp
					if(fieldDescriptor & 0x08)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].tmp = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x10)
						{
							if(i >= partsDataLen) goto fail;
							particles[newIndex].tmp |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read ctype
					if(fieldDescriptor & 0x20)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].ctype = partsData[i++];
						//Read additional bytes
						if(fieldDescriptor & 0x200)
						{
							if(i+2 >= partsDataLen) goto fail;
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 24);
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 16);
							particles[newIndex].ctype |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read dcolour
					if(fieldDescriptor & 0x40)
					{
						if(i+3 >= partsDataLen) goto fail;
						particles[newIndex].dcolour = (((unsigned)partsData[i++]) << 24);
						particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 16);
						particles[newIndex].dcolour |= (((unsigned)partsData[i++]) << 8);
						particles[newIndex].dcolour |= ((unsigned)partsData[i++]);
					}
					
					//Read vx
					if(fieldDescriptor & 0x80)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].vx = (partsData[i++]-127.0f)/16.0f;
					}
					
					//Read vy
					if(fieldDescriptor & 0x100)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].vy = (partsData[i++]-127.0f)/16.0f;
					}
					
					//Read tmp2
					if(fieldDescriptor & 0x400)
					{
						if(i >= partsDataLen) goto fail;
						particles[newIndex].tmp2 = partsData[i++];
					}
					
					/*if ((sim->player.spwn == 1 && particles[newIndex].type==PT_STKM) || (sim->player2.spwn == 1 && particles[newIndex].type==PT_STKM2))
					{
						particles[newIndex].type = PT_NONE;
					}
					else if (particles[newIndex].type == PT_STKM)
					{
						//STKM_init_legs(&player, newIndex);
						sim->player.spwn = 1;
						sim->player.elem = PT_DUST;
					}
					else if (particles[newIndex].type == PT_STKM2)
					{
						//STKM_init_legs(&player2, newIndex);
						sim->player2.spwn = 1;
						sim->player2.elem = PT_DUST;
					}
					else if (particles[newIndex].type == PT_FIGH)
					{
						//TODO: 100 should be replaced with a macro
						unsigned char fcount = 0;
						while (fcount < 100 && fcount < (sim->fighcount+1) && sim->fighters[fcount].spwn==1) fcount++;
						if (fcount < 100 && sim->fighters[fcount].spwn==0)
						{
							particles[newIndex].tmp = fcount;
							sim->fighters[fcount].spwn = 1;
							sim->fighters[fcount].elem = PT_DUST;
							sim->fighcount++;
							//STKM_init_legs(&(sim->fighters[sim->fcount]), newIndex);
						}
					}
					if (!sim->elements[particles[newIndex].type].Enabled)
						particles[newIndex].type = PT_NONE;*/
					newIndex++;
				}
			}
		}
	}
	goto fin;
fail:
	//Clean up everything
	bson_destroy(&b);
	if(freeIndices)
		free(freeIndices);
	throw ParseException(ParseException::Corrupt, "Save data currupt");
fin:
	bson_destroy(&b);
	if(freeIndices)
		free(freeIndices);
}

void GameSave::readPSv(char * data, int dataLength)
{
	unsigned char * d = NULL, * c = (unsigned char *)data;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0, tempGrav = 0;
	int bx0=0, by0=0, bw, bh, w, h, y0 = 0, x0 = 0;
	int nf=0, new_format = 0, ttv = 0;
	int *fp = (int *)malloc(NPART*sizeof(int));
	
	std::vector<sign> tempSigns;
	char tempSignText[255];
	sign tempSign("", 0, 0, sign::Left);
	
	//Gol data used to read older saves
	int goltype[NGOL];
	int grule[NGOL+1][10];
	
	int golRulesCount;
	int * golRulesT = LoadGOLRules(golRulesCount);
	memcpy(grule, golRulesT, sizeof(int) * (golRulesCount*10));
	free(golRulesT);
	
	int golTypesCount;
	int * golTypesT = LoadGOLTypes(golTypesCount);
	memcpy(goltype, golTypesT, sizeof(int) * (golTypesCount));
	free(golTypesT);

	vector<Element> elements = GetElements();
	
	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error
	
	if (dataLength<16)
		throw ParseException(ParseException::Corrupt, "No save data");
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		throw ParseException(ParseException::Corrupt, "Unknown format");
	if (c[2]==0x76 && c[1]==0x53 && c[0]==0x50) {
		new_format = 1;
	}
	if (c[4]>SAVE_VERSION)
		throw ParseException(ParseException::WrongVersion, "Save from newer version");
	ver = c[4];
	
	if (ver<34)
	{
		legacyEnable = 1;
	}
	else
	{
		if (ver>=44) {
			legacyEnable = c[3]&0x01;
			paused = (c[3]>>1)&0x01;
			if (ver>=46) {
				gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49) {
				gravityEnable = ((c[3]>>7)&0x01);
			}
		} else {
			if (c[3]==1||c[3]==0) {
				legacyEnable = c[3];
			} else {
				legacy_beta = 1;
			}
		}
	}
	
	bw = c[6];
	bh = c[7];
	if (bx0+bw > XRES/CELL)
		bx0 = XRES/CELL - bw;
	if (by0+bh > YRES/CELL)
		by0 = YRES/CELL - bh;
	if (bx0 < 0)
		bx0 = 0;
	if (by0 < 0)
		by0 = 0;
	
	if (c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
		throw ParseException(ParseException::InvalidDimensions, "Save too large");
	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = (unsigned char *)malloc(i);
	if (!d)
		throw ParseException(ParseException::Corrupt, "Cannot allocate memory");
	
	setSize(bw*CELL, bh*CELL);
	
	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), dataLength-12, 0, 0))
		throw ParseException(ParseException::Corrupt, "Cannot decompress");
	dataLength = i;
	
	if (dataLength < bw*bh)
		throw ParseException(ParseException::Corrupt, "Save data corrupt (missing data)");
	
	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;
	
	if (ver<46) {
		gravityMode = 0;
		airMode = 0;
	}
	m = (int *)calloc(XRES*YRES, sizeof(int));
	
	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				//In old saves, ignore walls created by sign tool bug
				//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
				if (ver<71 && d[p]==O_WL_SIGN)
				{
					p++;
					continue;
				}
				blockMap[y][x] = d[p];
				if (blockMap[y][x]==1)
					blockMap[y][x]=WL_WALL;
				if (blockMap[y][x]==2)
					blockMap[y][x]=WL_DESTROYALL;
				if (blockMap[y][x]==3)
					blockMap[y][x]=WL_ALLOWLIQUID;
				if (blockMap[y][x]==4)
					blockMap[y][x]=WL_FAN;
				if (blockMap[y][x]==5)
					blockMap[y][x]=WL_STREAM;
				if (blockMap[y][x]==6)
					blockMap[y][x]=WL_DETECT;
				if (blockMap[y][x]==7)
					blockMap[y][x]=WL_EWALL;
				if (blockMap[y][x]==8)
					blockMap[y][x]=WL_WALLELEC;
				if (blockMap[y][x]==9)
					blockMap[y][x]=WL_ALLOWAIR;
				if (blockMap[y][x]==10)
					blockMap[y][x]=WL_ALLOWSOLID;
				if (blockMap[y][x]==11)
					blockMap[y][x]=WL_ALLOWALLELEC;
				if (blockMap[y][x]==12)
					blockMap[y][x]=WL_EHOLE;
				if (blockMap[y][x]==13)
					blockMap[y][x]=WL_ALLOWGAS;
				
				if (blockMap[y][x]==O_WL_WALLELEC)
					blockMap[y][x]=WL_WALLELEC;
				if (blockMap[y][x]==O_WL_EWALL)
					blockMap[y][x]=WL_EWALL;
				if (blockMap[y][x]==O_WL_DETECT)
					blockMap[y][x]=WL_DETECT;
				if (blockMap[y][x]==O_WL_STREAM)
					blockMap[y][x]=WL_STREAM;
				if (blockMap[y][x]==O_WL_FAN||blockMap[y][x]==O_WL_FANHELPER)
					blockMap[y][x]=WL_FAN;
				if (blockMap[y][x]==O_WL_ALLOWLIQUID)
					blockMap[y][x]=WL_ALLOWLIQUID;
				if (blockMap[y][x]==O_WL_DESTROYALL)
					blockMap[y][x]=WL_DESTROYALL;
				if (blockMap[y][x]==O_WL_ERASE)
					blockMap[y][x]=WL_ERASE;
				if (blockMap[y][x]==O_WL_WALL)
					blockMap[y][x]=WL_WALL;
				if (blockMap[y][x]==O_WL_ALLOWAIR)
					blockMap[y][x]=WL_ALLOWAIR;
				if (blockMap[y][x]==O_WL_ALLOWSOLID)
					blockMap[y][x]=WL_ALLOWSOLID;
				if (blockMap[y][x]==O_WL_ALLOWALLELEC)
					blockMap[y][x]=WL_ALLOWALLELEC;
				if (blockMap[y][x]==O_WL_EHOLE)
					blockMap[y][x]=WL_EHOLE;
				if (blockMap[y][x]==O_WL_ALLOWGAS)
					blockMap[y][x]=WL_ALLOWGAS;
				if (blockMap[y][x]==O_WL_GRAV)
					blockMap[y][x]=WL_GRAV;
				if (blockMap[y][x]==O_WL_ALLOWENERGY)
					blockMap[y][x]=WL_ALLOWENERGY;
			}
			
			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				fanVelX[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				fanVelY[y][x] = (d[p++]-127.0f)/64.0f;
			}
	
	// load the particle map
	i = 0;
	k = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= dataLength)
				goto corrupt;
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//goto corrupt;
			}
			if (j)
			{
				memset(particles+k, 0, sizeof(Particle));
				particles[k].type = j;
				if (j == PT_COAL)
					particles[k].tmp = 50;
				if (j == PT_FUSE)
					particles[k].tmp = 50;
				if (j == PT_PHOT)
					particles[k].ctype = 0x3fffffff;
				if (j == PT_SOAP)
					particles[k].ctype = 0;
				if (j==PT_BIZR || j==PT_BIZRG || j==PT_BIZRS)
					particles[k].ctype = 0x47FFFF;
				particles[k].x = (float)x;
				particles[k].y = (float)y;
				m[(x-x0)+(y-y0)*w] = k+1;
				particlesCount = k++;
			}
		}
	
	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= dataLength)
				goto corrupt;
			if (i < NPART)
			{
				particles[i].vx = (d[p++]-127.0f)/16.0f;
				particles[i].vy = (d[p++]-127.0f)/16.0f;
			}
			else
				p += 2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=44) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					particles[i-1].life = ttv;
				} else {
					p+=2;
				}
			} else {
				if (p >= dataLength)
					goto corrupt;
				if (i <= NPART)
					particles[i-1].life = d[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i)
			{
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					particles[i-1].tmp = ttv;
					if (ver<53 && !particles[i-1].tmp)
						for (q = 1; q<=NGOLALT; q++) {
							if (particles[i-1].type==goltype[q-1] && grule[q][9]==2)
								particles[i-1].tmp = grule[q][9]-1;
						}
					if (ver>=51 && ver<53 && particles[i-1].type==PT_PBCN)
					{
						particles[i-1].tmp2 = particles[i-1].tmp;
						particles[i-1].tmp = 0;
					}
				} else {
					p+=2;
				}
			}
		}
	}
	if (ver>=53) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			ty = d[pty+j];
			if (i && ty==PT_PBCN)
			{
				if (p >= dataLength)
					goto corrupt;
				if (i <= NPART)
					particles[i-1].tmp2 = d[p++];
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					particles[i-1].dcolour = d[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					particles[i-1].dcolour |= d[p++];
				} else {
					p++;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
		if (i)
		{
			if (ver>=34&&legacy_beta==0)
			{
				if (p >= dataLength)
				{
					goto corrupt;
				}
				if (i <= NPART)
				{
					if (ver>=42) {
						if (new_format) {
							ttv = (d[p++])<<8;
							ttv |= (d[p++]);
							if (particles[i-1].type==PT_PUMP) {
								particles[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								particles[i-1].temp = ttv;
							}
						} else {
							particles[i-1].temp = (d[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP;
						}
					} else {
						particles[i-1].temp = ((d[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273;
					}
				}
				else
				{
					p++;
					if (new_format) {
						p++;
					}
				}
			}
			else
			{
				particles[i-1].temp = elements[particles[i-1].type].Temperature;
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		int gnum = 0;
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= dataLength)
				goto corrupt;
			if (i <= NPART)
				particles[i-1].ctype = d[p++];
			else
				p++;
		}
		//TODO: STKM_init_legs
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if (particles[i-1].type == PT_SPNG)
			{
				if (fabs(particles[i-1].vx)>0.0f || fabs(particles[i-1].vy)>0.0f)
					particles[i-1].flags |= FLAG_MOVABLE;
			}
			
			if (ver<48 && (ty==OLD_PT_WIND || (ty==PT_BRAY&&particles[i-1].life==0)))
			{
				// Replace invisible particles with something sensible and add decoration to hide it
				x = (int)(particles[i-1].x+0.5f);
				y = (int)(particles[i-1].y+0.5f);
				particles[i-1].dcolour = 0xFF000000;
				particles[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				particles[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (ty==goltype[gnum])
						particles[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (particles[i-1].ctype==goltype[gnum])
					{
						particles[i-1].ctype = PT_LIFE;
						particles[i-1].tmp = gnum;
					}
				}
			}
			if(ty==PT_LCRY){
				if(ver<67)
				{
					//New LCRY uses TMP not life
					if(particles[i-1].life>=10)
					{
						particles[i-1].life = 10;
						particles[i-1].tmp2 = 10;
						particles[i-1].tmp = 3;
					}
					else if(particles[i-1].life<=0)
					{
						particles[i-1].life = 0;
						particles[i-1].tmp2 = 0;
						particles[i-1].tmp = 0;
					}
					else if(particles[i-1].life < 10 && particles[i-1].life > 0)
					{
						particles[i-1].tmp = 1;
					}
				}
				else
				{
					particles[i-1].tmp2 = particles[i-1].life;
				}
			}
		}
	}
	
	if (p >= dataLength)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > dataLength)
			goto corrupt;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		tempSign.y = x+y0;
		x = d[p++];
		tempSign.ju = (sign::Justification)x;
		x = d[p++];
		if (p+x > dataLength)
			goto corrupt;
		if(x>254)
			x = 254;
		memcpy(tempSignText, d+p, x);
		tempSignText[x] = 0;
		tempSign.text = tempSignText;
		tempSigns.push_back(tempSign);
		p += x;
	}
	
	for (i = 0; i < tempSigns.size(); i++)
	{
		if(i == MAXSIGNS)
			break;
		signs.push_back(tempSigns[i]);
	}
	
version1:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);
	
	return;
	
corrupt:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);
	
	throw ParseException(ParseException::Corrupt, "Save data corrupt");
}

char * GameSave::serialiseOPS(int & dataLength)
{
	//Particle *particles = sim->parts;
	unsigned char *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *finalData = NULL, *outputData = NULL;
	unsigned *partsPosLink = NULL, *partsPosFirstMap = NULL, *partsPosCount = NULL, *partsPosLastMap = NULL;
	unsigned int partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, finalDataLen, outputDataLen;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int x, y, i, wallDataFound = 0;
	int posCount, signsCount;
	bson b;
	
	//Get coords in blocks
	blockX = 0;//orig_x0/CELL;
	blockY = 0;//orig_y0/CELL;
	
	//Snap full coords to block size
	fullX = blockX*CELL;
	fullY = blockY*CELL;
	
	//Original size + offset of original corner from snapped corner, rounded up by adding CELL-1
	blockW = (width-fullX+CELL-1)/CELL;
	blockH = (height-fullY+CELL-1)/CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;
	
	//Copy fan and wall data
	wallData = (unsigned char*)malloc(blockW*blockH);
	wallDataLen = blockW*blockH;
	fanData = (unsigned char*)malloc((blockW*blockH)*2);
	fanDataLen = 0;
	for(x = blockX; x < blockX+blockW; x++)
	{
		for(y = blockY; y < blockY+blockH; y++)
		{
			wallData[(y-blockY)*blockW+(x-blockX)] = blockMap[y][x];
			if(blockMap[y][x] && !wallDataFound)
				wallDataFound = 1;
			if(blockMap[y][x]==WL_FAN)
			{
				i = (int)(fanVelX[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
				i = (int)(fanVelY[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
		}
	}
	if(!fanDataLen)
	{
		free(fanData);
		fanData = NULL;
	}
	if(!wallDataFound)
	{
		free(wallData);
		wallData = NULL;
	}
	
	//Index positions of all particles, using linked lists
	//partsPosFirstMap is pmap for the first particle in each position
	//partsPosLastMap is pmap for the last particle in each position
	//partsPosCount is the number of particles in each position
	//partsPosLink contains, for each particle, (i<<8)|1 of the next particle in the same position
	partsPosFirstMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLastMap = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosCount = (unsigned int *)calloc(fullW*fullH, sizeof(unsigned));
	partsPosLink = (unsigned int *)calloc(NPART, sizeof(unsigned));
	for(i = 0; i < NPART; i++)
	{
		if(particles[i].type)
		{
			x = (int)(particles[i].x+0.5f);
			y = (int)(particles[i].y+0.5f);
			//Coordinates relative to top left corner of saved area
			x -= fullX;
			y -= fullY;
			if (!partsPosFirstMap[y*fullW + x])
			{
				//First entry in list
				partsPosFirstMap[y*fullW + x] = (i<<8)|1;
				partsPosLastMap[y*fullW + x] = (i<<8)|1;
			}
			else
			{
				//Add to end of list
				partsPosLink[partsPosLastMap[y*fullW + x]>>8] = (i<<8)|1;//link to current end of list
				partsPosLastMap[y*fullW + x] = (i<<8)|1;//set as new end of list
			}
			partsPosCount[y*fullW + x]++;
		}
	}
	
	//Store number of particles in each position
	partsPosData = (unsigned char*)malloc(fullW*fullH*3);
	partsPosDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			posCount = partsPosCount[y*fullW + x];
			partsPosData[partsPosDataLen++] = (posCount&0x00FF0000)>>16;
			partsPosData[partsPosDataLen++] = (posCount&0x0000FF00)>>8;
			partsPosData[partsPosDataLen++] = (posCount&0x000000FF);
		}
	}
	
	//Copy parts data
	/* Field descriptor format:
	 |		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|
	 |		tmp2	|	ctype[2]	|		vy		|		vx		|	dcololour	|	ctype[1]	|		tmp[2]	|		tmp[1]	|		life[2]	|		life[1]	|	temp dbl len|
	 life[2] means a second byte (for a 16 bit field) if life[1] is present
	 */
	partsData = (unsigned char *)malloc(NPART * (sizeof(Particle)+1));
	partsDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			//Find the first particle in this position
			i = partsPosFirstMap[y*fullW + x];
			
			//Loop while there is a pmap entry
			while (i)
			{
				unsigned short fieldDesc = 0;
				int fieldDescLoc = 0, tempTemp, vTemp;
				
				//Turn pmap entry into a particles index
				i = i>>8;
				
				//Type (required)
				partsData[partsDataLen++] = particles[i].type;
				
				//Location of the field descriptor
				fieldDescLoc = partsDataLen++;
				partsDataLen++;
				
				//Extra Temperature (2nd byte optional, 1st required), 1 to 2 bytes
				//Store temperature as an offset of 21C(294.15K) or go into a 16byte int and store the whole thing
				if(fabs(particles[i].temp-294.15f)<127)
				{
					tempTemp = (particles[i].temp-294.15f);
					partsData[partsDataLen++] = tempTemp;
				}
				else
				{
					fieldDesc |= 1;
					tempTemp = particles[i].temp;
					partsData[partsDataLen++] = tempTemp;
					partsData[partsDataLen++] = tempTemp >> 8;
				}
				
				//Life (optional), 1 to 2 bytes
				if(particles[i].life)
				{
					fieldDesc |= 1 << 1;
					partsData[partsDataLen++] = particles[i].life;
					if(particles[i].life > 255)
					{
						fieldDesc |= 1 << 2;
						partsData[partsDataLen++] = particles[i].life >> 8;
					}
				}
				
				//Tmp (optional), 1 to 2 bytes
				if(particles[i].tmp)
				{
					fieldDesc |= 1 << 3;
					partsData[partsDataLen++] = particles[i].tmp;
					if(particles[i].tmp > 255)
					{
						fieldDesc |= 1 << 4;
						partsData[partsDataLen++] = particles[i].tmp >> 8;
					}
				}
				
				//Ctype (optional), 1 or 4 bytes
				if(particles[i].ctype)
				{
					fieldDesc |= 1 << 5;
					partsData[partsDataLen++] = particles[i].ctype;
					if(particles[i].ctype > 255)
					{
						fieldDesc |= 1 << 9;
						partsData[partsDataLen++] = (particles[i].ctype&0xFF000000)>>24;
						partsData[partsDataLen++] = (particles[i].ctype&0x00FF0000)>>16;
						partsData[partsDataLen++] = (particles[i].ctype&0x0000FF00)>>8;
					}
				}
				
				//Dcolour (optional), 4 bytes
				if(particles[i].dcolour && (particles[i].dcolour & 0xFF000000))
				{
					fieldDesc |= 1 << 6;
					partsData[partsDataLen++] = (particles[i].dcolour&0xFF000000)>>24;
					partsData[partsDataLen++] = (particles[i].dcolour&0x00FF0000)>>16;
					partsData[partsDataLen++] = (particles[i].dcolour&0x0000FF00)>>8;
					partsData[partsDataLen++] = (particles[i].dcolour&0x000000FF);
				}
				
				//VX (optional), 1 byte
				if(fabs(particles[i].vx) > 0.001f)
				{
					fieldDesc |= 1 << 7;
					vTemp = (int)(particles[i].vx*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}
				
				//VY (optional), 1 byte
				if(fabs(particles[i].vy) > 0.001f)
				{
					fieldDesc |= 1 << 8;
					vTemp = (int)(particles[i].vy*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}
				
				//Tmp2 (optional), 1 byte
				if(particles[i].tmp2)
				{
					fieldDesc |= 1 << 10;
					partsData[partsDataLen++] = particles[i].tmp2;
				}
				
				//Write the field descriptor;
				partsData[fieldDescLoc] = fieldDesc;
				partsData[fieldDescLoc+1] = fieldDesc>>8;
				
				//Get the pmap entry for the next particle in the same position
				i = partsPosLink[i];
			}
		}
	}
	if(!partsDataLen)
	{
		free(partsData);
		partsData = NULL;
	}
	
	bson_init(&b);
	bson_append_bool(&b, "waterEEnabled", waterEEnabled);
	bson_append_bool(&b, "legacyEnable", legacyEnable);
	bson_append_bool(&b, "gravityEnable", gravityEnable);
	bson_append_bool(&b, "paused", paused);
	bson_append_int(&b, "gravityMode", gravityMode);
	bson_append_int(&b, "airMode", airMode);
	
	//bson_append_int(&b, "leftSelectedElement", sl);
	//bson_append_int(&b, "rightSelectedElement", sr);
	//bson_append_int(&b, "activeMenu", active_menu);
	if(partsData)
		bson_append_binary(&b, "parts", BSON_BIN_USER, (const char *)partsData, partsDataLen);
	if(partsPosData)
		bson_append_binary(&b, "partsPos", BSON_BIN_USER, (const char *)partsPosData, partsPosDataLen);
	if(wallData)
		bson_append_binary(&b, "wallMap", BSON_BIN_USER, (const char *)wallData, wallDataLen);
	if(fanData)
		bson_append_binary(&b, "fanMap", BSON_BIN_USER, (const char *)fanData, fanDataLen);
	signsCount = 0;
	for(i = 0; i < signs.size(); i++)
	{
		if(signs[i].text.length() && signs[i].x>=0 && signs[i].x<=fullW && signs[i].y>=0 && signs[i].y<=fullH)
		{
			signsCount++;
		}
	}
	if(signsCount)
	{
		bson_append_start_array(&b, "signs");
		for(i = 0; i < signs.size(); i++)
		{
			if(signs[i].text.length() && signs[i].x>=0 && signs[i].x<=fullW && signs[i].y>=0 && signs[i].y<=fullH)
			{
				bson_append_start_object(&b, "sign");
				bson_append_string(&b, "text", signs[i].text.c_str());
				bson_append_int(&b, "justification", signs[i].ju);
				bson_append_int(&b, "x", signs[i].x);
				bson_append_int(&b, "y", signs[i].y);
				bson_append_finish_object(&b);
			}
		}
	}
	bson_append_finish_array(&b);
	bson_finish(&b);
	bson_print(&b);
	
	finalData = (unsigned char *)bson_data(&b);
	finalDataLen = bson_size(&b);
	outputDataLen = finalDataLen*2+12;
	outputData = (unsigned char *)malloc(outputDataLen);
	
	outputData[0] = 'O';
	outputData[1] = 'P';
	outputData[2] = 'S';
	outputData[3] = '1';
	outputData[4] = SAVE_VERSION;
	outputData[5] = CELL;
	outputData[6] = blockW;
	outputData[7] = blockH;
	outputData[8] = finalDataLen;
	outputData[9] = finalDataLen >> 8;
	outputData[10] = finalDataLen >> 16;
	outputData[11] = finalDataLen >> 24;
	
	if (BZ2_bzBuffToBuffCompress((char*)(outputData+12), &outputDataLen, (char*)finalData, bson_size(&b), 9, 0, 0) != BZ_OK)
	{
		puts("Save Error\n");
		free(outputData);
		dataLength = 0;
		outputData = NULL;
		goto fin;
	}
	
	printf("compressed data: %d\n", outputDataLen);
	dataLength = outputDataLen + 12;
	
fin:
	bson_destroy(&b);
	if(partsData)
		free(partsData);
	if(wallData)
		free(wallData);
	if(fanData)
		free(fanData);
	
	return (char*)outputData;
}

GameSave::~GameSave()
{
	if(width && height)
	{
		if(particles)
		{
			delete[] particles;
		}
		if(blockMap)
		{
			delete[] blockMapPtr;
			delete[] blockMap;
		}
		if(fanVelX)
		{
			delete[] fanVelXPtr;
			delete[] fanVelX;
		}
		if(fanVelY)
		{
			delete[] fanVelYPtr;
			delete[] fanVelY;
		}
	}
}
