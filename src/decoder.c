/*
 * decoder.c - decode input sequence to words
 *
 *  Created on: 2010-1-31
 *      Author: cai
 */

#include <config.h>

#ifdef HAVE_GETTEXT

#include <locale.h>
#include <libintl.h>

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#else

#define _(x) (x)

#endif
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

static char * nextline(char *);

/**
 */

PHRASER *
phraser_new(char * file)
{
  struct stat state;
  void * ptr;
  PHRASER * ret;

  int f = open(file, O_RDONLY);
  if (f < 0)
    return NULL;

  fstat(f, &state);
  ptr = mmap(0, state.st_size, PROT_WRITE|PROT_READ, MAP_PRIVATE, f, 0);
  if (!ptr)
    {
      close(f);
      return NULL;
    }
  ret = g_new(PHRASER,1);
  ret ->filename = strdup(file);
  ret->fsize = state.st_size;
  ret->start_ptr = (char*) ptr;
  close(f);
  return ret;
}

void
phraser_free(PHRASER*phraser)
{
  free(phraser->filename);
  munmap(phraser->start_ptr, phraser->fsize);
}

//优化数据文件，其实就是使得每一行都一样长
int
phraser_optimise(PHRASER * phraser)
{
  char p0[64], p1[64];

  const int max_length = 64; // 绝对够的，不够你找偶
  char * ptr, *preserve,*ptr2;
  char * p;
  int type, preservesize;

  ptr = phraser->start_ptr;

  preservesize = 1024 * 1024;

  //预先申请 1 M 内存，不够了再说
  ptr2 = preserve = mmap(0, preservesize, PROT_WRITE | PROT_READ, MAP_ANONYMOUS
      | MAP_PRIVATE, -1, 0);

  if (!preserve)
    return -1;


  //进入循环，一行一行的扫描 :)
  while ((ptr = nextline(ptr)) && ((ptr - phraser->start_ptr) < phraser->fsize))
    {
      memcpy(ptr2, ptr, 64); //直接拷贝过去就可以了
      nextline(ptr2)[-1] = 0;
      ptr2 += 64;
    }

  munmap(phraser->start_ptr,phraser->fsize);

  phraser->start_ptr = preserve ;
  phraser->fsize = preservesize;
  return 0;
}

static gint
mysort(gconstpointer a, gconstpointer b)
{
  MATCHED * pa ,  *pb;
  pa = (MATCHED*) a;
  pb = (MATCHED*) b;
//  g_printf("match sort %s %s\n",pa->hanzi,pb->hanzi);

  return (strlen(pa->code) ) - (strlen(pb->code) ) ;
}


int
phraser_get_phrases(GArray * result, GString * input, PHRASER * phraser)
{
  char * ptr, *start_ptr,*p;
  int i,size;
  MATCHED mt;

  result = g_array_set_size(result, 0);

  for (start_ptr = ptr = phraser->start_ptr; (ptr - start_ptr) < phraser->fsize; ptr
      += 64)
    {

      if (memcmp(ptr, input->str, input->len) == 0)
        {
          memset(&mt, 0,64);
          p = ptr;
          while (*p != ' ' && *p != '\t')
            ++p;
          memcpy(mt.code, ptr, p - ptr  );
          while (*p == ' ' || *p == '\t')
            ++p;
          strcpy(mt.hanzi, p);
          result = g_array_append_val(result,mt );
          size++;
        }
    }
  //调节顺序
  g_array_sort(result, mysort);
  return size;
}

static char *
nextline(char * ptr)
{
  while (*ptr != '\n')
    ++ptr;
//  *ptr = 0;
  return *ptr?++ptr:NULL;
}
