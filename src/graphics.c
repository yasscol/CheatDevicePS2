#include "graphics.h"
#include "stb_font.h"
#include "libraries/upng.h"
#include "version.h"
#include "cheats.h"
#include "util.h"
#include <time.h>
#include <graph.h>
#include <stdio.h>
#include <kernel.h>

typedef struct menuIcon {
        char *label;
        GSTEXTURE *tex;
} menuIcon_t;

static GSGLOBAL *gsGlobal;
static GSTEXTURE bg;
static GSTEXTURE check;
static GSTEXTURE font;
static GSTEXTURE gamepad;
static GSTEXTURE cube;
static GSTEXTURE cogs;
static GSTEXTURE savemanager;
static GSTEXTURE flashdrive;
static GSTEXTURE memorycard1;
static GSTEXTURE memorycard2;
static GSTEXTURE buttonCross;
static GSTEXTURE buttonCircle;
static GSTEXTURE buttonTriangle;
static GSTEXTURE buttonSquare;
static GSTEXTURE buttonL1;
static GSTEXTURE buttonL2;
static GSTEXTURE buttonR1;
static GSTEXTURE buttonR2;
static stb_fontchar fontdata[STB_SOMEFONT_NUM_CHARS];
static int initialized = 0;
static int callbackId;

extern u8  _background_png_start[];
extern int _background_png_size;
extern u8  _check_png_start[];
extern int _check_png_size;
extern u8  _gamepad_png_start[];
extern int _gamepad_png_size;
extern u8  _cube_png_start[];
extern int _cube_png_size;
extern u8  _cogs_png_start[];
extern int _cogs_png_size;
extern u8  _savemanager_png_start[];
extern int _savemanager_png_size;
extern u8  _flashdrive_png_start[];
extern int _flashdrive_png_size;
extern u8  _memorycard1_png_start[];
extern int _memorycard1_png_size;
extern u8  _memorycard2_png_start[];
extern int _memorycard2_png_size;
extern u8  _buttonCross_png_start[];
extern int _buttonCross_png_size;
extern u8  _buttonCircle_png_start[];
extern int _buttonCircle_png_size;
extern u8  _buttonTriangle_png_start[];
extern int _buttonTriangle_png_size;
extern u8  _buttonSquare_png_start[];
extern int _buttonSquare_png_size;
extern u8  _buttonL1_png_start[];
extern int _buttonL1_png_size;
extern u8  _buttonL2_png_start[];
extern int _buttonL2_png_size;
extern u8  _buttonR1_png_start[];
extern int _buttonR1_png_size;
extern u8  _buttonR2_png_start[];
extern int _buttonR2_png_size;

static void graphicsLoadPNG(GSTEXTURE *tex, u8 *data, int len, int linear_filtering);

static u64 graphicsColorTable[] =
		{ GS_SETREG_RGBAQ(0x00,0x00,0x00,0x80,0x80), // BLACK
         GS_SETREG_RGBAQ(0xF0,0xF0,0xF0,0x80,0x80), // WHITE
         GS_SETREG_RGBAQ(0xF0,0x00,0x00,0x80,0x80), // RED
         GS_SETREG_RGBAQ(0x00,0xF0,0x00,0x80,0x80), // GREEN
         GS_SETREG_RGBAQ(0x20,0x20,0xA0,0x80,0x80), // BLUE
         GS_SETREG_RGBAQ(0xF0,0xB0,0x00,0x80,0x80) }; // YELLOW

static int vsync_callback()
{
    gsKit_display_buffer(gsGlobal);
    gsKit_unlock_buffer(gsGlobal);
    ExitHandler();
    return 0;
}

int initGraphics()
{
    if(!initialized)
    {
        printf("\n ** Initializing Graphics **\n");
        dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

        // Initialize the DMAC
        dmaKit_chan_init(DMA_CHANNEL_GIF);

        // Initialize the GS
        gsGlobal = gsKit_init_global();
        gsGlobal->PrimAAEnable = GS_SETTING_ON;
        gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
        gsGlobal->DoubleBuffering = GS_SETTING_OFF;
        gsGlobal->ZBuffering = GS_SETTING_OFF;
        callbackId = gsKit_add_vsync_handler(&vsync_callback);
        gsKit_init_screen( gsGlobal );
        gsKit_mode_switch( gsGlobal, GS_ONESHOT );

        // Clear the screen right away to prevent the old framebuffer from being dumped to screen
        gsKit_set_test(gsGlobal, GS_ZTEST_OFF);
        gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00));
        gsKit_sync_flip( gsGlobal );

        font.Width = STB_SOMEFONT_BITMAP_WIDTH;
        font.Height = STB_SOMEFONT_BITMAP_HEIGHT;
        font.PSM = GS_PSM_T8;
        font.ClutPSM = GS_PSM_CT32;
        font.Mem = memalign(128, gsKit_texture_size_ee(font.Width, font.Height, font.PSM));
        font.Clut = memalign(128, gsKit_texture_size_ee(16, 16, font.ClutPSM));
        font.VramClut = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(16, 16, font.ClutPSM), GSKIT_ALLOC_USERBUFFER);
        font.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(font.Width, font.Height, font.PSM), GSKIT_ALLOC_USERBUFFER);
        font.Filter = GS_FILTER_NEAREST;

        /* Generate palette */
        unsigned int i;
        for(i = 0; i < 256; ++i)
        {
            u8 alpha = (i * 128) / 255;
            font.Clut[i] = GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, alpha);
        }

        STB_SOMEFONT_CREATE(fontdata, (unsigned char(*)[])font.Mem, STB_SOMEFONT_BITMAP_HEIGHT);
        gsKit_texture_upload(gsGlobal, &font);

        graphicsLoadPNG(&bg, _background_png_start, _background_png_size, 0);
        graphicsDrawBackground();
        graphicsDrawText(450, 400, "Please wait...", WHITE);
        graphicsRenderNow();

        graphicsLoadPNG(&check, _check_png_start, _check_png_size, 0);
        graphicsLoadPNG(&gamepad, _gamepad_png_start, _gamepad_png_size, 1);
        graphicsLoadPNG(&cube, _cube_png_start, _cube_png_size, 1);
        graphicsLoadPNG(&cogs, _cogs_png_start, _cogs_png_size, 1);
        graphicsLoadPNG(&savemanager, _savemanager_png_start, _savemanager_png_size, 1);
        graphicsLoadPNG(&flashdrive, _flashdrive_png_start, _flashdrive_png_size, 1);
        graphicsLoadPNG(&memorycard1, _memorycard1_png_start, _memorycard1_png_size, 1);
        graphicsLoadPNG(&memorycard2, _memorycard2_png_start, _memorycard2_png_size, 1);
        graphicsLoadPNG(&buttonCross, _buttonCross_png_start, _buttonCross_png_size, 0);
        graphicsLoadPNG(&buttonCircle, _buttonCircle_png_start, _buttonCircle_png_size, 0);
        graphicsLoadPNG(&buttonTriangle, _buttonTriangle_png_start, _buttonTriangle_png_size, 0);
        graphicsLoadPNG(&buttonSquare, _buttonSquare_png_start, _buttonSquare_png_size, 0);
        graphicsLoadPNG(&buttonL1, _buttonL1_png_start, _buttonL1_png_size, 0);
        graphicsLoadPNG(&buttonL2, _buttonL2_png_start, _buttonL2_png_size, 0);
        graphicsLoadPNG(&buttonR1, _buttonR1_png_start, _buttonR1_png_size, 0);
        graphicsLoadPNG(&buttonR2, _buttonR2_png_start, _buttonR2_png_size, 0);

        return 1;
    }
    else
        return 0;
}

static void graphicsLoadPNG(GSTEXTURE *tex, u8 *data, int len, int linear_filtering)
{
    upng_t* pngTexture = upng_new_from_bytes(data, len);
    upng_header(pngTexture);
    upng_decode(pngTexture);

    tex->VramClut = 0;
    tex->Clut = NULL;
    tex->Width = upng_get_width(pngTexture);
    tex->Height = upng_get_height(pngTexture);
    u8 *imageBuffer = (u8 *)upng_get_buffer(pngTexture);
    
    if(upng_get_format(pngTexture) == UPNG_RGB8)
    {
        tex->PSM = GS_PSM_CT24;
    }
    else if(upng_get_format(pngTexture) == UPNG_RGBA8)
    {
        tex->PSM = GS_PSM_CT32;

        // Convert alpha value range to [0, 80]
        int i;
        for(i = 0; i < tex->Width * tex->Height; i++)
        {
            u8 alpha = imageBuffer[i * 4 + 3];

            if(alpha == 0xFF)
            {
                alpha = 0x80;
            }
            else
            {
                alpha = alpha >> 1;
            }

            imageBuffer[i * 4 + 3] = alpha;
        }
    }

    tex->Mem = memalign(128, gsKit_texture_size_ee(tex->Width, tex->Height, tex->PSM));
    memcpy(tex->Mem, imageBuffer, gsKit_texture_size_ee(tex->Width, tex->Height, tex->PSM));
    tex->Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(tex->Width, tex->Height, tex->PSM), GSKIT_ALLOC_USERBUFFER);
    tex->Filter = (linear_filtering) ? GS_FILTER_LINEAR : GS_FILTER_NEAREST;
    gsKit_texture_upload(gsGlobal, tex);
    
    upng_free(pngTexture);
}

/*
Get special texture from text sequence.
str should be a char string starting with '{' and containing a matching
'}' somewhere in it. The first curly-brace enclosed substring (ex: {CIRCLE})
will be considered. If a valid sequence is found, the pointer to the position
after the matching '}' is returned and texture points to the associated texture
object. Otherwise NULL is returned.
*/
static const char* getSpecialTexture(const char *str, GSTEXTURE** texture)
{
    char const *cptr = str;
    char special[17];
    int i = 0;
    GSTEXTURE *specialTexture = NULL;
    special[0] = '\0';

    if(!cptr)
        return NULL;

    if(cptr[0] != '{')
        return NULL;
    else
        cptr++;

    while(*cptr && *cptr != '}' && i < 16)
    {
        special[i] = *cptr;
        special[i + 1] = '\0';
        cptr++;
        i++;
    }

    if(*cptr == '}')
    {
        // Skip over ending character
        cptr++;
    }
    else
    {
        // Missing ending character
        return NULL;
    }

    if(strncmp(special, "CROSS", 16) == 0)
    {
        specialTexture = &buttonCross;
    }
    else if(strncmp(special, "CIRCLE", 16) == 0)
    {
        specialTexture = &buttonCircle;
    }
    else if(strncmp(special, "TRIANGLE", 16) == 0)
    {
        specialTexture = &buttonTriangle;
    }
    else if(strncmp(special, "SQUARE", 16) == 0)
    {
        specialTexture = &buttonSquare;
    }
    else if(strncmp(special, "L1", 16) == 0)
    {
        specialTexture = &buttonL1;
    }
    else if(strncmp(special, "L2", 16) == 0)
    {
        specialTexture = &buttonL2;
    }
    else if(strncmp(special, "R1", 16) == 0)
    {
        specialTexture = &buttonR1;
    }
    else if(strncmp(special, "R2", 16) == 0)
    {
        specialTexture = &buttonR2;
    }
    else
    {
        // Unrecognized identifier.
        return NULL;
    }

    if(texture)
        *texture = specialTexture;
    return cptr;

}

int graphicsGetSymbolLength(const char *start, int index)
{
    if(!start)
        return 1;

    // Figure out if we're in a {} symbol pair
    // find closing brace
    int endIndex = strlen(start);
    int closingIndex = index;
    while(start[closingIndex] && start[closingIndex] != '}')
        closingIndex++;
    // now work backwards
    int openingIndex = closingIndex;
    while(openingIndex > 0 && start[openingIndex] != '{')
        openingIndex--;

    if(openingIndex > index ||
       openingIndex == closingIndex ||
       closingIndex == endIndex)
       return 1;

    // Might be in a special symbol. Check if it's valid.
    const char *end = getSpecialTexture(&start[index], NULL);

    if(end)
        return end - &start[index];
    else
        return 1;
}

int graphicsGetSymbolLengthBackwards(const char *start, int index)
{
    if(!start || start[index] != '}')
        return 1;

    while(index > 0 && start[index] != '{')
        index--;

    const char *end = getSpecialTexture(&start[index], NULL);

    if(end)
        return end - &start[index];
    else
        return 1;
}

static void graphicsPrintText(int x, int y, const char *txt, u64 color)
{
    char const *cptr = txt;
    char special[17];
    float cx = x;
    float cy = y;

    memset(special, '\0', 17);

    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);

    while(*cptr)
    {
        if(*cptr == '\n')
        {
            cy += 22;
            cx = x;
            cptr++;
            continue;
        }
        else if(*cptr == '{')
        {
            // Read special sequence. A special sequence is surrounded by curly
            // braces and will be replaced by a texture when drawn.
            // For example, {CROSS} will be drawn as a cross button symbol

            const char *ret;
            GSTEXTURE *specialTexture;
            ret = getSpecialTexture(cptr, &specialTexture);

            if(ret != NULL)
            {
                u64 graphicColor;
                if(color == graphicsColorTable[YELLOW])
                {
                    graphicColor = color;
                }
                else
                {
                    graphicColor = 0x80808080;
                }
                gsKit_prim_sprite_texture(gsGlobal, specialTexture,
                                                    cx,
                                                    cy + 4,
                                                    0,
                                                    0,
                                                    cx + specialTexture->Width,
                                                    cy + 4 + specialTexture->Height,
                                                    specialTexture->Width,
                                                    specialTexture->Height,
                                                    1,
                                                    graphicColor);
                cx += specialTexture->Width;
                cptr = ret;
            }
            else
            {
                goto regularPrint;
            }
        }
        else
        {
regularPrint: ;
            int char_codepoint = *cptr++;
            stb_fontchar *cdata = &fontdata[char_codepoint - STB_SOMEFONT_FIRST_CHAR];
            
            if(cx < 700 && cx > -100)
            {
                gsKit_prim_sprite_texture(gsGlobal, &font, cx + cdata->x0f, cy + cdata->y0f, STB_SOMEFONT_BITMAP_WIDTH*cdata->s0f, cdata->t0f*STB_SOMEFONT_BITMAP_HEIGHT,
                                                       cx + cdata->x1f, cy + cdata->y1f, STB_SOMEFONT_BITMAP_WIDTH*cdata->s1f, cdata->t1f*STB_SOMEFONT_BITMAP_HEIGHT,
                                                       1, color);
            }

            cx += cdata->advance;
        }
    }

    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

void graphicsDrawChar(int x, int y, char c, graphicsColor_t color)
{
    stb_fontchar *cdata = &fontdata[c - STB_SOMEFONT_FIRST_CHAR];
    u64 colorValue = graphicsColorTable[color];

    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
    gsKit_prim_sprite_texture(gsGlobal, &font, x + cdata->x0f, y + cdata->y0f, STB_SOMEFONT_BITMAP_WIDTH*cdata->s0f, cdata->t0f*STB_SOMEFONT_BITMAP_HEIGHT,
                                               x + cdata->x1f, y + cdata->y1f, STB_SOMEFONT_BITMAP_WIDTH*cdata->s1f, cdata->t1f*STB_SOMEFONT_BITMAP_HEIGHT,
                                               1, colorValue);
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

void graphicsDrawText(int x, int y, const char *txt, graphicsColor_t color)
{
    graphicsPrintText(x, y, txt, graphicsColorTable[color]);
}

void graphicsDrawTextCentered(int y, const char *txt, graphicsColor_t color)
{
    char const *cptr = txt;
    char const *start = txt;
    double lineWidth = 0;
    
    while(*cptr)
    {
        if(*cptr == '\n')
        {
            *cptr = '\0';
            graphicsPrintText((gsGlobal->Width - lineWidth)/2.0, y, start, graphicsColorTable[color]);
            *cptr = '\n';
            cptr++;
            lineWidth = 0;
            start = cptr;
            y += 22;
            continue;
        }
        else if(*cptr == '{')
        {
            // Read special sequence
            const char *ret;
            GSTEXTURE *specialTexture;
            ret = getSpecialTexture(cptr, &specialTexture);

            if(ret != NULL)
            {
                lineWidth += specialTexture->Width;
                cptr = ret;
                continue;
            }
        }

        int char_codepoint = *cptr++;
        stb_fontchar *cdata = &fontdata[char_codepoint - STB_SOMEFONT_FIRST_CHAR];
        lineWidth += cdata->advance;
    }
    
    graphicsPrintText((gsGlobal->Width - lineWidth)/2.0, y, start, graphicsColorTable[color]); // last line
}

void graphicsDrawLoadingBar(int x, int y, float progress)
{
    int height = 10;
    int width = gsGlobal->Width - 2*x;
    u64 color = GS_SETREG_RGBAQ(0x22, 0x22, 0xee, 0x00, 0x80);
    u64 outline = GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x80);

    if(progress < 0.0)
        progress = 0.0;
    if(progress > 1.0)
        progress = 1.0;

    // outline
    gsKit_prim_quad(gsGlobal, x-5, y-5,
                              x-5, y+height+5,
                              x+5+width, y-5,
                              x+5+width, y+height+5, 1, outline);

    // progress bar
    gsKit_prim_quad(gsGlobal, x, y,
                              x, y+height,
                              x + (progress * width), y,
                              x + (progress * width), y+height, 1, color);
}

void graphicsDrawQuad(float x, float y, float xsize, float ysize, graphicsColor_t color)
{
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);

    gsKit_prim_quad(gsGlobal, x, y,
                              x + xsize, y,
                              x, y + ysize,
                              x + xsize, y + ysize, 1, graphicsColorTable[color]);

    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

static void drawPromptBox(int width, int height, u64 color)
{
    const int x0 = (gsGlobal->Width/2.0) - (width/2.0);
    const int x1 = (gsGlobal->Width/2.0) + (width/2.0);
    const int y0 = (gsGlobal->Height/2.0) - (height/2.0);
    const int y1 = (gsGlobal->Height/2.0) + (height/2.0);

    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);

    gsKit_prim_quad(gsGlobal, x0, y0,
                              x1, y0,
                              x0, y1,
                              x1, y1, 1, color);

    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

void graphicsDrawPromptBox(int width, int height)
{
    drawPromptBox(width, height, GS_SETREG_RGBAQ(0x22, 0x22, 0xEE, 0x25, 0x80));
}

void graphicsDrawPromptBoxBlack(int width, int height)
{
    drawPromptBox(width, height, graphicsColorTable[BLACK]);
}

static void drawMenu(menuIcon_t icons[], int numIcons, int activeItem)
{
    int i;
    const u64 unselected = GS_SETREG_RGBAQ(0x50, 0x50, 0x50, 0x20, 0x80);
    const u64 selected = GS_SETREG_RGBAQ(0x50, 0x50, 0x50, 0x80, 0x80);
    
    graphicsDrawPromptBox(350, 150);
    
    for(i = 0; i < numIcons; i++)
    {
        int x = (gsGlobal->Width / 2) - ((75 * numIcons) / 2.0) + (75 * i);
        gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
        gsKit_prim_sprite_texture(gsGlobal, icons[i].tex,
                                            x,
                                            192,
                                            0,
                                            0,
                                            x + (icons[i].tex)->Width,
                                            192 + (icons[i].tex)->Height,
                                            (icons[i].tex)->Width,
                                            (icons[i].tex)->Height,
                                            1,
                                            (activeItem == i) ? selected : unselected);
        if (activeItem == i) graphicsDrawTextCentered(265, icons[i].label, WHITE);
        gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
    }
}

void graphicsDrawMainMenu(int activeItem)
{
    menuIcon_t icons[] = {{"Start Game", &gamepad},
                          {"Game List", &cube},
                          {"Save Manager", &savemanager}};
    
    drawMenu(icons, 3, activeItem);
}

void graphicsDrawDeviceMenu(int activeItem)
{
    menuIcon_t icons[] = {{"Memory Card (Slot 1)", &memorycard1},
                          {"Memory Card (Slot 2)", &memorycard2},
                          {"Flash Drive", &flashdrive}};
    
    drawMenu(icons, 3, activeItem);
}

void graphicsClearScreen(int r, int g, int b)
{
    gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(r, g, b, 0x00, 0x00));
}

void graphicsDrawBackground()
{
    graphicsClearScreen(0, 0, 0);
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);
    gsKit_prim_sprite_texture(gsGlobal, &bg,
                                        0,                            // X1
                                        0,                            // Y1
                                        0,                            // U1
                                        0,                            // V1
                                        bg.Width,                     // X2
                                        bg.Height,                    // Y2
                                        bg.Width,                     // U2
                                        bg.Height,                    // V2
                                        1,                            // Z
                                        0x80808080                    // RGBAQ
                                        );
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

void graphicsDrawBackgroundBottom(unsigned int rows)
{
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
    gsKit_prim_sprite_texture(gsGlobal, &bg,
                                        0,                            // X1
                                        bg.Height - rows,             // Y1
                                        0,                            // U1
                                        bg.Height - rows,             // V1
                                        bg.Width,      // X2
                                        bg.Height,   // Y2
                                        bg.Width,                     // U2
                                        bg.Height,                    // V2
                                        1,                            // Z
                                        0x80808080                    // RGBAQ
                                        );
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

void graphicsDrawPointer(int x, int y)
{
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
    gsKit_prim_sprite_texture(gsGlobal, &check,
                                        x,
                                        y,
                                        0,
                                        0,
                                        x + check.Width,
                                        y + check.Height,
                                        check.Width,
                                        check.Height,
                                        1,
                                        0x80808080);
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
}

int graphicsGetWidthSubString(const char *str, int n)
{
    if(!str)
        return 0;

    char const *cptr = str;
    double maxWidth = 0;
    double lineWidth = 0;
    
    while(*cptr && n-- > 0)
    {
        if(*cptr == '\n')
        {
            if(lineWidth > maxWidth)
                maxWidth = lineWidth;
            cptr++;
            lineWidth = 0;
        }
        else if(*cptr == '{')
        {
            // Read special sequence
            const char *ret;
            GSTEXTURE *specialTexture;
            ret = getSpecialTexture(cptr, &specialTexture);

            if(ret != NULL)
            {
                lineWidth += specialTexture->Width;
                n -= (ret - cptr) - 1;
                cptr = ret;
                continue;
            }
        }

        int char_codepoint = *cptr++;
        stb_fontchar *cdata = &fontdata[char_codepoint - STB_SOMEFONT_FIRST_CHAR];
    
        lineWidth += cdata->advance;
    }

    if(lineWidth > maxWidth)
        maxWidth = lineWidth;

    return maxWidth;
}

int graphicsGetWidth(const char *str)
{
    if(!str)
        return 0;

    int len = strlen(str);
    return graphicsGetWidthSubString(str, len);
}

int graphicsGetDisplayWidth()
{
    return gsGlobal->Width;
}

int graphicsGetDisplayHeight()
{
    return gsGlobal->Height;
}

void graphicsDrawAboutPage()
{
    char msg[200];

    snprintf(msg, 200, "Cheat Device\n" GIT_VERSION 
                       "\nCompiled " __DATE__ " " __TIME__
                       "\n\nTotal Games: %d\n"
                       "Database: %s",
                       cheatsGetNumGames(), settingsGetDatabasePath());

    displayError(msg);
}

void graphicsRenderNow()
{
    gsKit_queue_exec( gsGlobal );
    gsKit_lock_buffer( gsGlobal );
}

void graphicsRender()
{
    gsKit_queue_exec( gsGlobal );
    gsKit_lock_buffer(gsGlobal);
    gsKit_vsync_wait();
}
