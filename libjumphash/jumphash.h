#include <libjumphash/w_blake.h>
#include <libjumphash/w_bmw.h>
#include <libjumphash/w_groestl.h>
#include <libjumphash/w_jh.h>
#include <libjumphash/w_keccak.h>
#include <libjumphash/w_skein.h>
#include <libjumphash/w_luffa.h>
#include <libjumphash/w_cubehash.h>
#include <libjumphash/w_shavite.h>
#include <libjumphash/w_simd.h>
#include <libjumphash/w_echo.h>
#include <libjumphash/w_hamsi.h>
#include <libjumphash/w_fugue.h>
#include <stdlib.h>
static const int hashcount=13;
static void (*jump[])(unsigned char* input, unsigned char* output)={
    blake_scanHash_post,bmw_scanHash_post,groestl_scanHash_post,skein_scanHash_post,
    jh_scanHash_post,keccak_scanHash_post,luffa_scanHash_post,cubehash_scanHash_post,
    shavite_scanHash_post, simd_scanHash_post, echo_scanHash_post, hamsi_scanHash_post, fugue_scanHash_post
};
static void Hex2Str(unsigned char *sSrc, unsigned char *sDest, int nSrcLen)
{
	int  i;
	 char szTmp[3];

	for (i = 0; i < nSrcLen; i++)
	{
		sprintf(szTmp, "%02X", (unsigned char)sSrc[i]);
		memcpy(&sDest[i * 2], szTmp, 2);
	}
	return;
}
static int getcount(){return hashcount;};