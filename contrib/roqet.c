#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* Rasqal includes */
#include <rasqal.h>
#define RASQAL_GOOD_CAST(t, v) (t)(v)
#define RASQAL_BAD_CAST(t, v) (t)(v)

int main(int argc, char *argv[]);

static char *program=NULL;

static int error_count = 0;
static int warning_count = 0;

static int warning_level = -1;
static int ignore_errors = 0;

#define MAX_QUERY_ERROR_REPORT_LEN 512


static void
roqet_log_handler(void *data, raptor_log_message *message)
{
  switch(message->level) {
    case RAPTOR_LOG_LEVEL_FATAL:
    case RAPTOR_LOG_LEVEL_ERROR:
      if(!ignore_errors) {
        fprintf(stderr, "%s: Error - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
      }

      error_count++;
      break;

    case RAPTOR_LOG_LEVEL_WARN:
      if(warning_level > 0) {
        fprintf(stderr, "%s: Warning - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
      }

      warning_count++;
      break;

    case RAPTOR_LOG_LEVEL_NONE:
    case RAPTOR_LOG_LEVEL_TRACE:
    case RAPTOR_LOG_LEVEL_DEBUG:
    case RAPTOR_LOG_LEVEL_INFO:

      fprintf(stderr, "%s: Unexpected %s message - ", program,
              raptor_log_level_get_label(message->level));
      raptor_locator_print(message->locator, stderr);
      fprintf(stderr, " - %s\n", message->text);
      break;
  }

}

#define SPACES_LENGTH 80
static const char spaces[SPACES_LENGTH + 1] = "                                                                                ";

static void
print_boolean_result_simple(rasqal_query_results *results,
                            FILE* output, int quiet)
{
  fprintf(stderr, "%s: Query has a boolean result: %s\n", program,
          rasqal_query_results_get_boolean(results) ? "true" : "false");
}


static int
print_graph_result(rasqal_query* rq,
                   rasqal_query_results *results,
                   raptor_world* raptor_world_ptr,
                   FILE* output,
                   const char* serializer_syntax_name, raptor_uri* base_uri,
                   int quiet)
{
  int triple_count = 0;
  rasqal_prefix* prefix;
  int i;
  raptor_serializer* serializer = NULL;
  
  if(!quiet)
    fprintf(stderr, "%s: Query has a graph result:\n", program);
  
  serializer = raptor_new_serializer(raptor_world_ptr, serializer_syntax_name);

  if(!serializer) {
    fprintf(stderr, "%s: Failed to create raptor serializer type %s\n",
            program, serializer_syntax_name);
    return(1);
  }
  
  /* Declare any query namespaces in the output serializer */
  for(i = 0; (prefix = rasqal_query_get_prefix(rq, i)); i++)
    raptor_serializer_set_namespace(serializer, prefix->uri, prefix->prefix);
  raptor_serializer_start_to_file_handle(serializer, base_uri, output);
  
  while(1) {
    raptor_statement *rs = rasqal_query_results_get_triple(results);
    if(!rs)
      break;

    raptor_serializer_serialize_statement(serializer, rs);
    triple_count++;
    
    if(rasqal_query_results_next_triple(results))
      break;
  }
  
  raptor_serializer_serialize_end(serializer);
  raptor_free_serializer(serializer);
  
  if(!quiet)
    fprintf(stderr, "%s: Total %d triples\n", program, triple_count);

  return 0;
}

static int
print_formatted_query_results(rasqal_world* world,
                              rasqal_query_results* results,
                              raptor_world* raptor_world_ptr,
                              FILE* output,
                              const char* result_format_name,
                              raptor_uri* base_uri,
                              int quiet)
{
  raptor_iostream *iostr;
  rasqal_query_results_formatter* results_formatter;
  int rc = 0;
  
  results_formatter = rasqal_new_query_results_formatter(world,
                                                         result_format_name,
                                                         NULL, NULL);
  if(!results_formatter) {
    fprintf(stderr, "%s: Invalid bindings result format `%s'\n",
            program, result_format_name);
    rc = 1;
    goto tidy;
  }
  

  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, output);
  if(!iostr) {
    rasqal_free_query_results_formatter(results_formatter);
    rc = 1;
    goto tidy;
  }
  
  rc = rasqal_query_results_formatter_write(iostr, results_formatter,
                                            results, base_uri);
  raptor_free_iostream(iostr);
  rasqal_free_query_results_formatter(results_formatter);

  tidy:
  if(rc)
    fprintf(stderr, "%s: Formatting query results failed\n", program);

  return rc;
}


static rasqal_query*
roqet_init_query(rasqal_world *world, 
                 const char* ql_name,
                 const char* ql_uri, const unsigned char* query_string,
                 raptor_uri* base_uri,
                 rasqal_feature query_feature, int query_feature_value,
                 const unsigned char* query_feature_string_value,
                 raptor_sequence* data_graphs)
{
  rasqal_query* rq;
  rq = rasqal_new_query(world, (const char*)ql_name,
                        (const unsigned char*)ql_uri);
  if(!rq) {
    fprintf(stderr, "%s: Failed to create query name %s\n",
            program, ql_name);
    goto tidy_query;
  }
  

  if(query_feature_value >= 0)
    rasqal_query_set_feature(rq, query_feature, query_feature_value);
  if(query_feature_string_value)
    rasqal_query_set_feature_string(rq, query_feature,
                                    query_feature_string_value);

  if(rasqal_query_prepare(rq, query_string, base_uri)) {
    size_t len = strlen((const char*)query_string);
    
    fprintf(stderr, "%s: Parsing query '", program);
    if(len > MAX_QUERY_ERROR_REPORT_LEN) {
      (void)fwrite(query_string,
                   RASQAL_GOOD_CAST(size_t, MAX_QUERY_ERROR_REPORT_LEN),
                   sizeof(char), stderr);
      fprintf(stderr, "...' (%d bytes) failed\n", RASQAL_BAD_CAST(int, len));
    } else {
      (void)fwrite(query_string, len, sizeof(char), stderr);
      fputs("' failed\n", stderr);
    }

    rasqal_free_query(rq); rq = NULL;
    goto tidy_query;
  }

  if(data_graphs) {
    rasqal_data_graph* dg;
    
    while((dg = (rasqal_data_graph*)raptor_sequence_pop(data_graphs))) {
      if(rasqal_query_add_data_graph(rq, dg)) {
        fprintf(stderr, "%s: Failed to add data graph to query\n",
                program);
        goto tidy_query;
      }
    }
  }

  tidy_query:
  return rq;
}

/* Default parser for input graphs */
#define DEFAULT_DATA_GRAPH_FORMAT "guess"
/* Default serializer for output graphs */
#define DEFAULT_GRAPH_FORMAT "ntriples"
/* Default input result format name */
#define DEFAULT_RESULT_FORMAT_NAME "xml"

int
main(int argc, char *argv[]) 
{ 
  unsigned char *query_string = (unsigned char*) "SELECT * FROM <test.ttl> WHERE { ?s ?p ?o }";
  rasqal_query *rq = NULL;
  rasqal_query_results *results;
  const char *ql_name = "sparql";
  char *ql_uri = NULL;
  int rc = 0;
  raptor_uri *uri = NULL;
  raptor_uri *base_uri = NULL;
  int quiet = 0;
  raptor_sequence* data_graphs = NULL;
  const char *result_format_name = NULL;
  rasqal_feature query_feature = (rasqal_feature)-1;
  int query_feature_value= -1;
  unsigned char* query_feature_string_value = NULL;
  rasqal_world *world;
  raptor_world* raptor_world_ptr = NULL;
  raptor_iostream* iostr = NULL;
  raptor_uri* service_uri = NULL;
  
  world = rasqal_new_world();
  if(!world || rasqal_world_open(world)) {
    fprintf(stderr, "%s: rasqal_world init failed\n", program);
    return(1);
  }
  raptor_world_ptr = rasqal_world_get_raptor(world);
  
/* 'Solo' : Commented out. Seems not needed. */
/*  rasqal_world_set_log_handler(world, world, roqet_log_handler); */
  
  result_format_name = "json";

  rq = roqet_init_query(world,
                          ql_name, ql_uri, query_string,
                          base_uri,
                          query_feature, query_feature_value,
                          query_feature_string_value,
                          data_graphs);
    
  if(!rq) {
    rc = 1;
    goto tidy_query;
  }
  
  results = rasqal_query_execute(rq);
        
  if(!results) {
    fprintf(stderr, "%s: Query execution failed\n", program);
    rc = 1;
    goto tidy_query;
  }

  fprintf(stdout,"(%s:%d)\n",__FILE__,__LINE__);
  if(rasqal_query_results_is_bindings(results)) {
    if(result_format_name)
      rc = print_formatted_query_results(world, results,
                                         raptor_world_ptr, stdout,
                                         result_format_name, base_uri, quiet);
  } else if(rasqal_query_results_is_boolean(results)) {
    if(result_format_name)
      rc = print_formatted_query_results(world, results,
                                         raptor_world_ptr, stdout,
                                         result_format_name, base_uri, quiet);
    else
      print_boolean_result_simple(results, stdout, quiet);
  } else if(rasqal_query_results_is_graph(results)) {
    if(!result_format_name)
      result_format_name = DEFAULT_GRAPH_FORMAT;
    
    rc = print_graph_result(rq, results, raptor_world_ptr,
                            stdout, result_format_name, base_uri, quiet);
  } else {
    fprintf(stderr, "%s: Query returned unknown result format\n", program);
    rc = 1;
  }

  rasqal_free_query_results(results);
  
 tidy_query:

  if(rq)
    rasqal_free_query(rq);

  if(data_graphs)
    raptor_free_sequence(data_graphs);
  if(base_uri)
    raptor_free_uri(base_uri);
  if(uri)
    raptor_free_uri(uri);
  if(iostr)
    raptor_free_iostream(iostr);
  if(service_uri)
    raptor_free_uri(service_uri);

  rasqal_free_world(world);

  if(error_count && !ignore_errors)
    return 1;

  if(warning_count && warning_level != 0)
    return 2;
  
  return (rc);
}
