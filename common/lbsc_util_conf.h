#ifndef LBSC_UTIL_CONF_H_
#define LBSC_UTIL_CONF_H_
// fopen fgets fclose
#include <stdio.h>
//memset memcpy
#include <stdlib.h>
#include "list.h"
#ifndef lbtrace
#define lbtrace printf
#endif
#ifndef lbdebug
#define lbdebug printf
#endif
#ifndef lberror
#define lberror printf
#endif

typedef int (*on_opt_read)(void* powner, const char* ptag, const char* pvalue);

static int read_conf_line(const char* pcfg_line, char* ptag, int taglen, char* pvalue, int valuelen)
{
	if(NULL == pcfg_line || NULL == ptag || taglen <= 0 || NULL == pvalue || valuelen <= 0)
	{
		printf("Invalid parameter, pcfg_line:%s, ptag:%s, taglen:%d, pvalue:%s, valuelen:%d\n", pcfg_line, ptag, taglen, pvalue, valuelen);
		return -1;
	}
	int i = 0;
	int tag_begin = 0, tag_end = 0;
	int value_begin = 0;
	int value_end = 0;
	//printf("read conf:%s\n", pcfg_line);
	// find first not empty char
	while(' ' == pcfg_line[i] || '\t' == pcfg_line[i])i++;
    if('#' == pcfg_line[i])
    {
		printf("# == pcfg_line[i:%d]", i);
        return -1;
    }
	tag_begin = i;
    //printf("&pcfg_line[i]:%s, tag_begin:%d\n", &pcfg_line[i], tag_begin);
	while(' ' != pcfg_line[i] && '\t' != pcfg_line[i] && '\0' !=  pcfg_line[i])i++;
	if('\0' == pcfg_line[i])
	{
		printf("\0 == pcfg_line[i:%d]", i);
		return -1;
	}
	tag_end = i;
    //printf("&pcfg_line[i]:%s, tag_end:%d\n", &pcfg_line[i], tag_end);
	if(taglen > tag_end - tag_begin + 1)
	{
        //lbtrace("before memset taglen:%d\n", taglen);
		memset(ptag, 0, taglen);
        //lbtrace("after memset taglen:%d\n", taglen);
		memcpy(ptag, &pcfg_line[tag_begin], tag_end - tag_begin);
        //lbtrace("ptag:%s, &pcfg_line[tag_begin]:%s, tag_end:%d - tag_begin:%d\n", ptag, &pcfg_line[tag_begin], tag_end, tag_begin);
	}
	else
	{
		printf("not enought buffer for conf tag, have %d,  needn %d, conf line:%s\n", taglen, tag_end - tag_begin + 1, pcfg_line);
		return -1;
	}
    //lbtrace("%d, %d t:%d\n", (int)pcfg_line[i], (int)pcfg_line[i+1], (int)'\t');
	while(' ' == pcfg_line[i] || '\t' == pcfg_line[i])i++;
	value_begin = i;
    
	value_end = strlen(pcfg_line);
    //lbtrace("value_begin:%d, pcfg_line[value_begin]:%d pcfg_line[value_end-1]:%d\n", value_begin, (int)pcfg_line[value_begin], (int)pcfg_line[value_end-1]);
	while(' ' == pcfg_line[value_end-1] || '\t' == pcfg_line[value_end-1] || ';' == pcfg_line[value_end-1] || '\n' == pcfg_line[value_end-1] || '\r' == pcfg_line[value_end-1])value_end--;
	lbtrace("value_end:%d, strlen(pcfg_line):%d\n", value_end, strlen(pcfg_line));
	if(valuelen > value_end - value_begin)
	{
		memset(pvalue, 0, valuelen);
		memcpy(pvalue, &pcfg_line[value_begin], value_end - value_begin);
	}
	else
	{
		printf("not enought buffer for conf tag value, have %d,  needn %d, conf line:%s\n", valuelen, value_end - value_begin, pcfg_line);
		return -1;
	}
	lbtrace("pcfg_line:%s, ptag:%s, pvalue:%s\n", pcfg_line, ptag, pvalue);
	return 0;
}

static int load_config(const char* pconf_file, void* powner, on_opt_read pread_func)
{
	int val_len = 0;
	FILE* pfile = fopen(pconf_file, "rb");
	lbdebug("pfile:%p = fopen(pconf_file:%s, rb)\n", pfile, pconf_file);
	char tag[256] = {0};
	char value[256] ={0};
	if(pfile)
	{
		do
		{
			char line[512] = {0};
			char* pline = fgets(line, 512, pfile);
			//printf("2 read pline:%p, line:%s\n", pline, line);
			if(NULL == pline)
			{
				break;
			}
            
			int ret = read_conf_line(line, tag, 256, value, 256);
			printf("ret:%d = read_conf_line(line:%s, tag:%s, 256, value:%s, 256)\n", ret, line, tag, value);
			if(0 != ret)
			{
				lbtrace("ret:%d = read_conf_line(line:%s, tag, 256, value, 256) failed\n", ret, line);
			}
			else if(0 == ret && pread_func)
			{
				pread_func(powner, tag, value);
			}
		}while(1);
		
		fclose(pfile);
		lbdebug("fclose(pfile:%p)\n", pfile);
		pfile = NULL;
	}

	return val_len;
}

static int parser_args(char** args, int argc, void* powner, on_opt_read popt_read)
{
    int ret = 0;
    //assert(ppi);
    //memset(ppi, 0, sizeof(push_ctx));
    int i;
    for(i = 1; i < argc; i++)
    {
		if('-' == args[i][0] && i + 1 < argc)
		{
			popt_read(powner, args[i], args[i+1]);
			i++;
		}
    }

    return ret;
}

#endif