#include "database.h"
#include "cheats.h"
#include "graphics.h"
#include "zlib.h"
#include "util.h"
#include "objectpool.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <kernel.h>
#include <tamtypes.h>

#define DBVERSION 0x02

typedef struct cdbMasterHeader {
    char magic[4];
    u8 version;
    u16 numTitles;
    char padding[9];
} cdbMasterHeader_t;

static cheatsGame_t *gamesHead = NULL;

static int cdbOpenBuffer(unsigned char *cdbBuff)
{
    u16 numCheats;
    cheatsCheat_t *cheatsHead = NULL;
    cheatsCheat_t *cheat = NULL;
    u8 *cdbOffset = NULL;
    u8 *cdbCheatsOffset = NULL;
    
    printf("Loading buffer\n");
    cheatsGame_t *game;
    cdbMasterHeader_t *cdbHeader = (cdbMasterHeader_t *)cdbBuff;

    if(strncmp(cdbHeader->magic, "CDB\0", 4) != 0)
    {
        char error[255];
        sprintf(error, "%s: invalid database format %c%c%c\n", __FUNCTION__, cdbHeader->magic[0], cdbHeader->magic[1], cdbHeader->magic[2]);
        displayError(error);

        return 0;
    }

    if(cdbHeader->version > DBVERSION)
    {
        char error[255];
        sprintf(error, "%s: unsupported database version\n", __FUNCTION__);
        displayError(error);

        return 0;
    }

    cdbOffset = cdbBuff + 16;

    game = objectPoolAllocate(OBJECTPOOLTYPE_GAME);
    gamesHead = game;

    int i, j;
    for(i = 0; i < cdbHeader->numTitles; i++)
    {
        int titlelen = READ_8(cdbOffset);
        if(titlelen>81)
            return 0;

        cdbOffset += 1;
        strncpy(game->title, cdbOffset, titlelen - 1);
        cdbOffset += titlelen;
        
        numCheats = READ_16(cdbOffset);
        cdbOffset += sizeof(u16);

        game->codeLinesCapacity = READ_16(cdbOffset);
        game->codeLines = calloc(game->codeLinesCapacity, sizeof(u64));
        cdbOffset += sizeof(u16);
        
        cdbCheatsOffset = cdbBuff + READ_32(cdbOffset);
        cdbOffset += sizeof(u32);

        cheatsHead = objectPoolAllocate(OBJECTPOOLTYPE_CHEAT);
        cheat = cheatsHead;

        /* Read each cheat */
        for(j = 0; j < numCheats; j++)
        {
            titlelen = *cdbCheatsOffset;
            if(titlelen>81)
            {
                printf("ERROR READING DB: title length is too long!\n");
                return 0;
            }

            cdbCheatsOffset++;
            strncpy(cheat->title, cdbCheatsOffset, titlelen-1);
            cdbCheatsOffset += titlelen;

            cheat->numCodeLines = READ_8(cdbCheatsOffset);
            cdbCheatsOffset++;

            cheat->codeLinesOffset = game->codeLinesUsed;

            if(cheat->numCodeLines > 0)
            {
                u64 *codeLine = game->codeLines + cheat->codeLinesOffset;
                memcpy(codeLine, cdbCheatsOffset, cheat->numCodeLines * sizeof(u64));
                cdbCheatsOffset += cheat->numCodeLines * sizeof(u64);
                game->codeLinesUsed += cheat->numCodeLines;
            }
            else
            {
                cheat->type = CHEAT_HEADER;
                game->numCheats--;
            }

            if(j+1 < numCheats)
                cheat->next = objectPoolAllocate(OBJECTPOOLTYPE_CHEAT);
            else
                cheat->next = NULL;

            cheat = cheat->next;
        }

        game->cheats = cheatsHead;
        game->numCheats = numCheats;

        if(i+1 < cdbHeader->numTitles)
        {
            game->next = objectPoolAllocate(OBJECTPOOLTYPE_GAME);
            game = game->next;
        }
        else
        {
            game->next = NULL;
        }
    }

    return cdbHeader->numTitles;
}

cheatsGame_t* cdbOpen(const char *path, unsigned int *numGames)
{
    u8 *compressed, *decompressed;
    int compressedSize;
    unsigned long decompressedSize;
    FILE *dbFile;

    if(!path)
        return NULL;
    if(!numGames)
        return NULL;
    
    dbFile = fopen(path, "rb");
    if(!dbFile)
    {
        printf("%s: failed to open %s\n", __FUNCTION__, path);
        return NULL;
    }
    
    fseek(dbFile, 0, SEEK_END);
    compressedSize = ftell(dbFile);
    fseek(dbFile, 0, SEEK_SET);
    compressed = malloc(compressedSize);
    fread(compressed, 1, compressedSize, dbFile);
    fclose(dbFile);

    decompressedSize = 5*1024*1024; // 5MB
    decompressed = malloc(decompressedSize);
    uncompress(decompressed, &decompressedSize, compressed, compressedSize);
    realloc(decompressed, decompressedSize);
    free(compressed);

    *numGames = cdbOpenBuffer(decompressed);

    free(decompressed);

    return gamesHead;
}

int cdbSave(const char *path, const cheatsGame_t *games)
{
    return 1;
}
