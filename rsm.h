#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* Rasqal includes */
#include <rasqal.h>
#define RASQAL_GOOD_CAST(t, v) (t)(v)
#define RASQAL_BAD_CAST(t, v) (t)(v)

#define MAX_QUERY_ERROR_REPORT_LEN 512

#include <ruby.h>                                                                                                                                                                                                  
#ifdef RUBY_VM
#include <ruby/io.h>
#include <ruby/st.h>
#else
#include <version.h>
#include <rubyio.h>
#include <st.h>
#if (RUBY_VERSION_MAJOR == 1 &&  RUBY_VERSION_MINOR == 8 && RUBY_VERSION_TEENY < 7)
#define RHASH_SIZE(h) (RHASH(h)->tbl ? RHASH(h)->tbl->num_entries : 0)
#endif
#define RHASH_EMPTY_P(h) (RHASH_SIZE(h) == 0)
#endif

#ifdef HAVE_RB_IO_T
#define RB_IO_T rb_io_t
#if (RUBY_VERSION_MAJOR == 1 &&  RUBY_VERSION_MINOR == 8 && RUBY_VERSION_TEENY < 7)
#define RB_IO_T_FD(o) o->fd
#else
#define RB_IO_T_FD(o) fileno(o->stdio_file)
#endif
#else
#define RB_IO_T OpenFile
#define RB_IO_T_FD(o) fileno(o->f)
#endif

#define RSM_VERSION  "0.0.99"

RUBY_EXTERN VALUE rsm_RDF;
RUBY_EXTERN VALUE rsm_Smart;

typedef struct rsm_obj {
  VALUE data_sources;
}
