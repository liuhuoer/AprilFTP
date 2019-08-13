#include "Common.h"
#include "Error.h"

void 
Fclose(FILE * fp)
{
    if(fclose(fp) != 0)
        Error::sys("fclose error");
}

void 
Fclose(FILE ** fp)
{
    if(fclose(*fp) != 0)
        Error::sys("fclose error");
    *fp = NULL;
}

void * 
Malloc(size_t size)
{
    void  * ptr;

    if( (ptr = malloc(size)) == NULL)
        Error::sys("malloc error");
    memset(ptr, 0, size);
    return (ptr);
}

void
Pthread_create(pthread_t * tid, const pthread_attr_t * attr,
                void * (*func)(void *), void * arg)
{
    int n;
    if( (n = pthread_create(tid, attr, func, arg)) == 0)
        return;
    errno = n;
    Error::sys("pthread_create error");
}

int 
getFileNslice(const char * pathname, uint32_t * pnslice_o)
{
    unsigned long filesize = 0, n = MAXNSLICE;

    struct stat statbuff;

    if(stat(pathname, &statbuff) < 0)
    {
        return -1;  // error
    }else{
        if(statbuff.st_size == 0)
        {
            return 0;   // file is empty
        }else{
            filesize = statbuff.st_size;
        }
    }
    if(filesize % SLICECAP == 0)
    {
        *pnslice_o = filesize / SLICECAP;
    }else if( (n = filesize / SLICECAP + 1) > MAXNSLICE)
    {
        Error::msg("too large file size: %d\n (MAX: %d)", n, MAXNSLICE);
        return -2;
    }else{
        *pnslice_o = filesize / SLICECAP + 1;
    }
    return 1;
}




string 
md5sum(const char * str, int len)
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

string 
md5sumNslice(const char * pathname, uint32_t nslice)
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

string 
visualmd5sum(const char * pathname)
{
    int n;
    char buf[SLICECAP];
    unsigned char out[MD5_DIGEST_LENGTH];
    string md5str;
    int oldProgress = 0, newProgress = 0;
    MD5_CTX ctx;

    uint32_t nslice = 0, sindex = 0;
    string tipstr;
    tipstr += "\033[32mMD5SUM\033[0m(";
    tipstr += pathname;
    tipstr += ")";
    string hfilesize = getFileSizeString(pathname);
    if( (n = getFileNslice(pathname, &nslice)) < 0)
    {
        Error::msg("getFileNslice error");
        return md5str;
    }
    
    FILE * fp;
    if( (fp = fopen(pathname, "rb")) == NULL)
    {
        Error::ret("md5sum#fopen");
        return md5str;
    }

    MD5_Init(&ctx);
    while( (n = fread(buf, sizeof(char), SLICECAP, fp)) > 0)
    {
        MD5_Update(&ctx, buf, n);
        if(nslice > (1024 * 512))
        {
            newProgress = (++sindex * 1.0) / nslice * 100;
            if(newProgress > oldProgress)
            {
                fprintf(stderr, "\033[2K\r\033[0m%-40s%10s\t%3d%%", tipstr.c_str(), hfilesize.c_str(), newProgress);
            }
            oldProgress = newProgress;
        }
    }
    if(nslice > (1024 * 512))
        printf("\n");
    
    MD5_Final(out, &ctx);

    for(n = 0; n < MD5_DIGEST_LENGTH; ++n)
    {
        snprintf(buf, SLICECAP, "%02x", out[n]);
        md5str += buf;
    }
    return md5str;
}

string 
getFileSizeString(const char * pathname)
{
    unsigned long filesize = 0;
    unsigned long n = 0;
    string hsize_o;
    char buf[MAXLINE];
    unsigned long kbase = 1024;
    unsigned long mbase = 1024 * 1024;
    unsigned long gbase = 1024 * 1024 * 1024;

    struct stat statbuff;
    if(stat(pathname, &statbuff) < 0)
    {
        hsize_o = "error";
        return hsize_o;     // error
    }else{
        if(statbuff.st_size == 0)
        {
            hsize_o = "0B"; // file is empty
        }else{
            filesize = statbuff.st_size;
            if(filesize /kbase == 0)
            {
                snprintf(buf, MAXLINE, "%lu", filesize);
                hsize_o += buf;
                hsize_o += "B";
            }else if(filesize / mbase == 0){
                snprintf(buf, MAXLINE, "%lu", filesize / kbase);
                hsize_o += buf;
                n = (filesize % kbase) * 100 /kbase;
                if(n != 0)
                {
                    hsize_o += ".";
                    snprintf(buf, MAXLINE, "%02lu", n);
                    hsize_o += buf;
                }
                hsize_o += "K";
            }else if(filesize / gbase == 0){
                snprintf(buf, MAXLINE, "%2lu", filesize / mbase);
                hsize_o += buf;
                n = (filesize % mbase) * 100 / mbase;
                if(n != 0)
                {
                    hsize_o += ".";
                    snprintf(buf, MAXLINE, "%02lu", n);
                    hsize_o += buf;
                }
                hsize_o += "M";
            }else{
                snprintf(buf, MAXLINE, "%lu", filesize / gbase);
                hsize_o += buf;
                n = (filesize % gbase) * 100 / gbase;
                if(n != 0)
                {
                    hsize_o += ".";
                    snprintf(buf, MAXLINE, "%02lu", n);
                    hsize_o += buf;
                }
                hsize_o += "G";
            }
        }
    }
    return hsize_o;
}

string 
visualmd5sumNslice(const char * pathname, uint32_t nslice)
{
    int n;
    char buf[SLICECAP];
    unsigned char out[MD5_DIGEST_LENGTH];
    string md5str;
    int oldProgress = 0, newProgress = 0;
    MD5_CTX ctx;

    uint32_t fileslice = 0, sindex = 0;
    if( (n = getFileNslice(pathname, &fileslice)) < 0)
    {
        Error::msg("getFileNslice error");
        return md5str;
    }
    int percent = (nslice * 1.0) / fileslice * 100;
    snprintf(buf, SLICECAP, "%u / %u %3d%%", nslice, fileslice, percent);
    string tipstr;
    tipstr += "\033[32mMD5SUM\033[0m(";
    tipstr += pathname;
    tipstr += " ";
    tipstr += buf;
    tipstr += ")";
    string hfilesize = getFileSizeString(pathname);

    FILE * fp;
    if( (fp = fopen(pathname, "rb")) == NULL)
    {
        Error::ret("md5sum#fopen");
        return md5str;
    }

    MD5_Init(&ctx);
    while( (n = fread(buf, sizeof(char), SLICECAP, fp)) > 0)
    {
        ++sindex;
        MD5_Update(&ctx, buf, n);

        if(nslice > (1024 * 512))
        {
            newProgress = (sindex * 1.0) / nslice * 100;
            if(newProgress > oldProgress)
            {
                fprintf(stderr, "\033[2K\r\033[0m%-40s%10s\t%3d%%", tipstr.c_str(), hfilesize.c_str(), newProgress);
            }
            oldProgress = newProgress;
        }
        if(sindex == nslice)
            break;
    }
    if(nslice > (1024 * 512))
        printf("\n");
    
    MD5_Final(out, &ctx);

    for(n = 0; n < MD5_DIGEST_LENGTH; ++n)
    {
        snprintf(buf, SLICECAP, "%02x", out[n]);
        md5str += buf;
    }
    return md5str;
}

string 
encryptPassword(string password)
{
    string saltedPass = PASSSALT0 + password + PASSSALT1;
    //cout << "*****saltedPass_before: " << saltedPass << endl;
    saltedPass = md5sum(saltedPass.c_str(), saltedPass.size());
    //cout << "*****saltedPass_after: " << saltedPass << endl;
    return saltedPass;
}

unsigned long long 
getFilesize(const char * pathname)
{
    struct stat statbuff;
    if(stat(pathname, &statbuff) < 0)
    {
        Error::ret("getFilesize#stat");
        return 0;
    }else{
        return (unsigned long long)statbuff.st_size;
    }
}

string 
getFilesize(string pathname)
{
    struct stat statbuff;
    char buf[MAXLINE];
    string sizestr;
    if(stat(pathname.c_str(), &statbuff) < 0)
    {
        Error::ret("getFilesize#stat");
        return sizestr;
    }else{
        snprintf(buf, MAXLINE, "%llu", (unsigned long long)statbuff.st_size);
        sizestr = buf;
        return sizestr;
    }
}

unsigned long long 
getDiskAvailable()
{
    struct statfs diskInfo;

    statfs(ROOTDIR, &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;
    unsigned long long availableDisk;

    availableDisk = diskInfo.f_bavail * blocksize;
    return availableDisk;
}

void 
split(std::string src, std::string token, vector<string> & vect)
{
    int nbegin = 0;
    int nend = 0;
    while(nend != -1 && (unsigned int)nbegin < src.length())
    {
        nend = src.find_first_of(token, nbegin);
        if(nend == -1)
        {
            vect.push_back(src.substr(nbegin, src.length() - nbegin));
        }else{
            if(nend != nbegin)
            {
                vect.push_back(src.substr(nbegin, nend - nbegin));
            }
        }
        nbegin = nend + 1;
    }
}
