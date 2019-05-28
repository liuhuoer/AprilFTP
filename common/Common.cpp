#include "Common.h"
#include "Error.h"

void Fclose(FILE * fp)
{
    if(fclose(fp) != 0)
        Error::sys("fclose error");
}

void Fclose(FILE ** fp)
{
    if(fclose(*fp) != 0)
        Error::sys("fclose error");
    *fp = NULL;
}

string md5sum(const char * str, int len)
{
    int n;
    MD5_CTX ctx;
    char buf[SLICECAP];
    unsigned char out[MD5_DIGEST_LENGTH];
    string md5str;

    MD5_Init(&ctx);
    MD5_Update(&ctx, str, len);
    MD5_Final(out, &ctx);

    for(n = 0; n < MD5_DIGEST_LENGTH; ++n)
    {
        snprintf(buf, SLICECAP, "%02x", out[n]);
        md5str += buf;
    }

    return md5str;
}

string md5sumNslice(const char * pathname, uint32_t nslice)
{
    int n;
    char buf[SLICECAP];
    unsigned char out[MD5_DIGEST_LENGTH];
    string md5str;
    MD5_CTX ctx;
    uint32_t sindex = 0;

    FILE * fp;
    if( (fp = fopen(pathname, "rb")) == NULL)
    {
        Error::ret("md5sum#fopen");
        return md5str;
    }

    MD5_Init(&ctx);
    while( (n = fread(buf, sizeof(char), SLICECAP, fp)) >0)
    {
        MD5_Update(&ctx, buf, n);
        if((++sindex) == nslice)
        {
            break;
        }
    }
    MD5_Final(out, &ctx);

    for(n = 0; n < MD5_DIGEST_LENGTH; ++n)
    {
        snprintf(buf, SLICECAP, "%02x", out[0]);
        md5str += buf;
    }
    return md5str;
}

void * Malloc(size_t size)
{
    void  * ptr;

    if( (ptr = malloc(size)) == NULL)
        Error::sys("malloc error");
    memset(ptr, 0, size);
    return (ptr);
}

string encryptPassword(string password)
{
    string saltedPass = PASSSALT0 + password + PASSSALT1;
    saltedPass = md5sum(saltedPass.c_str(), saltedPass.size());
    return saltedPass;
}