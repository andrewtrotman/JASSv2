#include <stdio.h>
#include <stdlib.h>

#include "zstd.h"

static void compressFile(const char* fname, const char* outName, int cLevel)
{
    FILE* const fin  = fopen(fname, "rb");
    FILE* const fout = fopen(outName, "wb");
    size_t const buffInSize = ZSTD_CStreamInSize();    /* can always read one full block */
    void*  const buffIn  = malloc(buffInSize);
    size_t const buffOutSize = ZSTD_CStreamOutSize();  /* can always flush a full block */
    void*  const buffOut = malloc(buffOutSize);

    ZSTD_CStream* const cstream = ZSTD_createCStream();
    if (cstream == NULL)
    	{
    	fprintf(stderr, "ZSTD_createCStream() error \n");
    	exit(10);
		}
    size_t const initResult = ZSTD_initCStream(cstream, cLevel);
    if (ZSTD_isError(initResult))
    	{
    	fprintf(stderr, "ZSTD_initCStream() error : %s \n", ZSTD_getErrorName(initResult));
    	exit(11);
		}

    size_t read, toRead = buffInSize;
    while( (read = fread(buffIn, 1, toRead, fin)) ) {
        ZSTD_inBuffer input = { buffIn, read, 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
            toRead = ZSTD_compressStream(cstream, &output , &input);   /* toRead is guaranteed to be <= ZSTD_CStreamInSize() */
            if (ZSTD_isError(toRead)) { fprintf(stderr, "ZSTD_compressStream() error : %s \n", ZSTD_getErrorName(toRead)); exit(12); }
            if (toRead > buffInSize) toRead = buffInSize;   /* Safely handle case when `buffInSize` is manually changed to a value < ZSTD_CStreamInSize()*/
            fwrite(buffOut, 1, output.pos, fout);
        }
    }

    ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
    size_t const remainingToFlush = ZSTD_endStream(cstream, &output);   /* close frame */
    if (remainingToFlush) { fprintf(stderr, "not fully flushed"); exit(13); }
    fwrite(buffOut, 1, output.pos, fout);

    ZSTD_freeCStream(cstream);
    fclose(fout);
    fclose(fin);
    free(buffIn);
    free(buffOut);
}



static void decompressFile(const char* fname)
{
    FILE* const fin  = fopen(fname, "rb");
    size_t const buffInSize = ZSTD_DStreamInSize();
    void*  const buffIn  = malloc(buffInSize);
    FILE* const fout = stdout;
    size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
    void*  const buffOut = malloc(buffOutSize);

    ZSTD_DStream* const dstream = ZSTD_createDStream();
    if (dstream==NULL) { fprintf(stderr, "ZSTD_createDStream() error \n"); exit(10); }

    /* In more complex scenarios, a file may consist of multiple appended frames (ex : pzstd).
    *  The following example decompresses only the first frame.
    *  It is compatible with other provided streaming examples */
    size_t const initResult = ZSTD_initDStream(dstream);
    if (ZSTD_isError(initResult)) { fprintf(stderr, "ZSTD_initDStream() error : %s \n", ZSTD_getErrorName(initResult)); exit(11); }
    size_t read, toRead = initResult;
    while ( (read = fread(buffIn, 1, toRead, fin)) ) {
        ZSTD_inBuffer input = { buffIn, read, 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
            toRead = ZSTD_decompressStream(dstream, &output , &input);  /* toRead : size of next compressed block */
            if (ZSTD_isError(toRead)) { fprintf(stderr, "ZSTD_decompressStream() error : %s \n", ZSTD_getErrorName(toRead)); exit(12); }
            fwrite(buffOut, 1, output.pos, fout);
        }
    }

    ZSTD_freeDStream(dstream);
    fclose(fin);
    fclose(fout);
    free(buffIn);
    free(buffOut);
}
/*
	MAIN()
	------
*/
int main(void)
	{

	return 0;
	}
