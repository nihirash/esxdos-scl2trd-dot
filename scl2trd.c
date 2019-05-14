#include <stdio.h>
#include <stdarg.h>
#include <arch/zx/esxdos.h>
#include <string.h>

unsigned freeTrack = 1;
unsigned freeSec = 0;
unsigned char *fileName;
unsigned char buff[256];
unsigned char *header[9] = "SINCLAIR";
int iStream;
int oStream;
int i;
unsigned char count;
unsigned int totalFreeSect = 2544;
int isFull = 0;

static void cleanBuff() 
{
    for(i=0;i<256;buff[i++] = 0);
}

void writeData()
{
    i = esx_f_read(iStream, &buff, 256);
    while (i == 256) // Only sectors info 
    {
        esx_f_write(oStream, &buff, i);
        i = esx_f_read(iStream, &buff, 256);
    }

    if (isFull) {
        cleanBuff();
        for (i=0;i<totalFreeSect;i++)
            esx_f_write(oStream, &buff, 256);
    }
}

void writeInfo()
{
    buff[0xe3] = 0x16; // IMPORTANT! 80 track double sided
    buff[0xe4] = count;
    buff[0xe1] = freeSec;
    buff[0xe2] = freeTrack;
    
    if (isFull) {
        buff[0xe6] = totalFreeSect / 256;
        buff[0xe5] = totalFreeSect & 255;
    }
    
    buff[0xe7] = 0x10;
    buff[0xea] = 32;
    buff[0xf5] = 's';
    buff[0xf6] = 'c';
    buff[0xf7] = 'l';
    buff[0xf8] = '2';
    buff[0xf9] = 't';
    buff[0xfa] = 'r';
    buff[0xfb] = 'd';
    esx_f_write(oStream, &buff, 256);
    esx_f_write(oStream, 0, 1792);
}

void writeCatalog()
{
    int cnt;
    esx_f_read(iStream, &count, 1);

    for (cnt=0;cnt<count;cnt++) {
        esx_f_read(iStream, &buff, 14);
        buff[14] = freeSec;
        buff[15] = freeTrack;
        freeSec += buff[0xd];
        totalFreeSect -= (int) buff[0xd];
        freeTrack += freeSec / 16;
        freeSec = freeSec % 16;
        esx_f_write(oStream, &buff, 16);
    }
    cleanBuff();
    for(cnt=count;cnt<128;cnt++)
        esx_f_write(oStream, &buff, 16);
}

void openFile()
{
    cleanBuff();
    iStream = esx_f_open(fileName, ESX_MODE_READ);
    esx_f_read(iStream, &buff, 8);

    if (strcmp(header, &buff)) {
        printf("\nWrong file type!\nOnly SCL allowed\n");
        esx_f_close(iStream);       

        return;
    }
    sprintf(strstr(fileName, "."), ".TRD");
    oStream = esx_f_open(fileName, ESX_MODE_WRITE | ESX_MODE_OPEN_CREAT_TRUNC);
}  

int main(int argc, char **argv)
{
    printf("\nscl2trd-dot by nihirash v.1.0.0\n");
    if (argc < 2) {
        printf("\nUsage .scl2trd [f] filename.scl\n\nf is optional - make full TRD\n\nexample:\n.scl2trd 1.scl   -- cutted TRD\n.scl2trd f 1.scl -- full TRD\n\n");
        return 0;
    }

    if (argv[1][0] == 'f') {
        printf("\nCreating full TRD!\n");
        isFull = 1;
        fileName = argv[2];
    } else {
        isFull = 0;
        fileName = argv[1];
    }
    
    openFile();
    writeCatalog();
    writeInfo();
    writeData();
    esx_f_close(iStream);
    esx_f_close(oStream);
    printf("\nFile converted!\n\n");
    return 0;
}