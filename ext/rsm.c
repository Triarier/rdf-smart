#include "rsm.h"

/* -- */
// ***********************************************************************************
// GC
// ***********************************************************************************
void rsm_free(rsm_obj *prsm_obj) {
  if (prsm_obj != NULL) {
    free(prsm_obj);
  }
}
  
void rsm_mark(rsm_obj *prsm_obj) {
  if (prsm_obj == NULL) return;
  if (!NIL_P(prsm_obj->data_sources)) rb_gc_mark(prsm_obj->data_sources);
}

//***********************************************************************************
// Methods
//***********************************************************************************
/* ++ */
static int rsm_print_graph_result(rasqal_query* rq,
                              rasqal_query_results *results,
                              raptor_world* raptor_world_ptr,
                              const char* serializer_syntax_name, 
                              char** output,
                              size_t* len)
{
  int triple_count = 0;
  rasqal_prefix* prefix;
  int i;
  raptor_serializer* serializer = NULL;
  
  serializer = raptor_new_serializer(raptor_world_ptr, serializer_syntax_name);

  /* Declare any query namespaces in the output serializer */
  for(i = 0; (prefix = rasqal_query_get_prefix(rq, i)); i++)
    raptor_serializer_set_namespace(serializer, prefix->uri, prefix->prefix);
  raptor_serializer_start_to_string(serializer, NULL, (void**)output, len);
  
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
  
  return 0;
}
static void rsm_extract_namespaces(void* user_data, raptor_namespace *nspace) {
  VALUE *h = (VALUE *)user_data;
  char * tmp = (char *)raptor_namespace_get_prefix(nspace);
  rb_hash_aset(*h, tmp == NULL ? Qnil : rb_str_new2(tmp), rb_str_new2((char *)raptor_uri_to_string(raptor_namespace_get_uri(nspace))));
}

static void rsm_get_namespaces(VALUE namespaces, unsigned char *uri_string) {
  /* input variables - parser
   * 'uri_string' is set to a URI otherwise 'filename' is file name
   * or if NULL, stdin.  Base URI in 'base_uri_string' is required for stdin.
   */
  raptor_world* world = NULL;
  raptor_parser* rdf_parser = NULL;
  char *filename = NULL;
  raptor_uri *uri;

  /* other variables */
  int rc;

  world = raptor_new_world();
  if(!world)
    return;
  rc = raptor_world_open(world);
  if(rc)
    return;

  filename = (char*)uri_string;
  uri_string = raptor_uri_filename_to_uri_string(filename);
  if(!uri_string) {
    fprintf(stderr, "Failed to create URI for file %s.\n", filename);
    return;
  }

  uri = raptor_new_uri(world, uri_string);
  if(!uri) {
    fprintf(stderr, "Failed to create URI for %s\n", uri_string);
    return;
  }

  rdf_parser = raptor_new_parser(world, "guess");
  if(!rdf_parser) {
    fprintf(stderr, "Failed to create raptor parser type %s\n", "guess");
    return;
  }

  raptor_parser_set_namespace_handler(rdf_parser, (void *)&namespaces, rsm_extract_namespaces);
  if(raptor_parser_parse_uri(rdf_parser, uri, NULL)) {
    fprintf(stderr, "Failed to parse URI %s %s content\n", uri_string, "guess");
  }

  raptor_free_parser(rdf_parser);

  if(uri)
    raptor_free_uri(uri);
  raptor_free_memory(uri_string);

  raptor_free_world(world);
}


static int rsm_print_formatted_query_results(rasqal_world* world,
                                         rasqal_query_results* results,
                                         raptor_world* raptor_world_ptr,
                                         const char* result_format_name,
                                         char** output,
                                         size_t* len)
{
  raptor_iostream *iostr;
  rasqal_query_results_formatter* results_formatter;
  int rc = 0;
  
  results_formatter = rasqal_new_query_results_formatter(world, result_format_name, NULL, NULL);

  iostr = raptor_new_iostream_to_string(raptor_world_ptr, (void**)output, len, NULL);
  
  rc = rasqal_query_results_formatter_write(iostr, results_formatter, results, NULL);
  raptor_free_iostream(iostr);
  rasqal_free_query_results_formatter(results_formatter);

  if(rc)
    rb_raise(rb_eRuntimeError, "Formatting query results failed");

  return rc;
}


static rasqal_query* rsm_roqet_init_query(rasqal_world *world, 
                                      const char* ql_name,
                                      const unsigned char* query_string,
                                      raptor_sequence* data_graphs)
{
  rasqal_query* rq;
  rq = rasqal_new_query(world, (const char*)ql_name, NULL);
  if(!rq) {
    rb_raise(rb_eRuntimeError, "Failed to create query name %s\n", ql_name);
  }
  
  if(rasqal_query_prepare(rq, query_string, NULL)) {
    size_t len = strlen((const char*)query_string);
    
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
  }

  if(data_graphs) {
    rasqal_data_graph* dg;
    
    while((dg = (rasqal_data_graph*)raptor_sequence_pop(data_graphs))) {
      if(rasqal_query_add_data_graph(rq, dg)) {
        fprintf(stderr, "Failed to add data graph to query\n");
      }
    }
  }
  return rq;
}
/*
 * This method gets all the namespaces
 */
VALUE rsm_namespaces(VALUE self) {
  rsm_obj *prsm_obj;
  int i;
  Data_Get_Struct(self, rsm_obj, prsm_obj);
  VALUE namespaces = rb_hash_new();

  for (i = 0; i < RARRAY_LEN(prsm_obj->data_sources); i++) {
    const char* data_source = RSTRING_PTR(RARRAY_PTR(prsm_obj->data_sources)[i]);
    rsm_get_namespaces(namespaces,(unsigned char *)data_source);
  } 

  return namespaces;
}

VALUE rsm_execute(VALUE self, VALUE query) { 
  rsm_obj *prsm_obj;

  unsigned char *query_string = (unsigned char *)RSTRING_PTR(query);
  rasqal_query *rq = NULL;
  rasqal_query_results *results;
  rasqal_world *world;
  raptor_world* raptor_world_ptr = NULL;
  char* output;
  size_t len = 0;
  int i;
  raptor_sequence* data_graphs = NULL;
  char* data_graph_parser_name = NULL;

  VALUE routput;
  int enc;

  Check_Type(query, T_STRING);
  Data_Get_Struct(self, rsm_obj, prsm_obj);

  world = rasqal_new_world();

//  if(!world || rasqal_world_open(world)) {
//    rb_raise(rb_eRuntimeError, "rasqal_world init failed");
//  }
  // raptor_world_ptr = rasqal_world_get_raptor(world);
  
//  /* #{{{ Data_Graphs*/
//  for (i = 0; i < RARRAY_LEN(prsm_obj->data_sources); i++) {
//    const char* data_source = RSTRING_PTR(RARRAY_PTR(prsm_obj->data_sources)[i]);
//    rasqal_data_graph *dg = NULL;
//    unsigned int type;
//
//    type = RASQAL_DATA_GRAPH_BACKGROUND;
//
//    if(!access((const char*)data_source, R_OK)) {
//      unsigned char* source_uri_string;
//      raptor_uri* source_uri;
//      raptor_uri* graph_name = NULL;
//
//      source_uri_string = raptor_uri_filename_to_uri_string((const char*)data_source);
//      source_uri = raptor_new_uri(raptor_world_ptr, source_uri_string);
//      raptor_free_memory(source_uri_string);
//
//      
//      if(source_uri)
//        dg = rasqal_new_data_graph_from_uri(world,
//                                            source_uri,
//                                            graph_name,
//                                            type,
//                                            NULL, data_graph_parser_name,
//                                            NULL);
//
//      if(source_uri)
//        raptor_free_uri(source_uri);
//    } else {
//      raptor_uri* source_uri;
//      raptor_uri* graph_name = NULL;
//
//      source_uri = raptor_new_uri(raptor_world_ptr,
//                                  (const unsigned char*)data_source);
//      
//      if(source_uri)
//        dg = rasqal_new_data_graph_from_uri(world,
//                                            source_uri,
//                                            graph_name,
//                                            type,
//                                            NULL, data_graph_parser_name,
//                                            NULL);
//
//      if(source_uri)
//        raptor_free_uri(source_uri);
//    }
//    
//    if(!dg) {
//      rb_raise(rb_eRuntimeError, "booboo");
//    }
//    
//    if(!data_graphs) {
//      data_graphs = raptor_new_sequence((raptor_data_free_handler)rasqal_free_data_graph,
//                                        NULL);
//
//      if(!data_graphs) {
//        rb_raise(rb_eRuntimeError, "Failed to create data graphs sequence");
//      }
//    }
//
//    raptor_sequence_push(data_graphs, dg);
//  }
///* #}}} */
//
//  if (!(rq = rsm_roqet_init_query(world, "sparql", query_string,data_graphs)))
//    goto tidy_query;
//
//  results = rasqal_query_execute(rq);
//
//  if(!results) {
//    fprintf(stderr, "Query execution failed\n");
//    goto tidy_query;
//  }
//
//  if(rasqal_query_results_is_bindings(results) || rasqal_query_results_is_boolean(results)) {
//    //rsm_print_formatted_query_results(world, results, raptor_world_ptr, "json", &output, &len);
//  } else if(rasqal_query_results_is_graph(results)) {
//    //rsm_print_graph_result(rq, results, raptor_world_ptr, "json", &output, &len);
//  } else {
//    rb_raise(rb_eRuntimeError, "Query returned unknown result format");
//  }
//
//  rasqal_free_query_results(results);
//  
//  tidy_query:
//    if(data_graphs) raptor_free_sequence(data_graphs);
//    if(rq) rasqal_free_query(rq);
    rasqal_free_world(world);
//
//    //routput = rb_str_new(output,len);
//    //enc = rb_enc_find_index("UTF-8");
//    //rb_enc_associate_index(routput, enc);
//
//    free(output);
//
//    //return routput;
    return Qnil;
}

VALUE rsm_data_sources(VALUE self) {
  rsm_obj *prsm_obj;
  Data_Get_Struct(self, rsm_obj, prsm_obj);
  return(prsm_obj->data_sources);
}

VALUE rsm_new(int argc, VALUE *argv, VALUE class) {
  int i;
  rsm_obj *prsm_obj = (rsm_obj *)malloc(sizeof(rsm_obj));
  prsm_obj->data_sources = rb_ary_new();
  for (i=0; i<argc; i++) {
    if (TYPE(argv[i]) == T_STRING)  
      rb_ary_push(prsm_obj->data_sources,argv[i]);
  }
  return(Data_Wrap_Struct(rsm_Smart, rsm_mark, rsm_free, prsm_obj));
}

VALUE rsm_RDF;
VALUE rsm_Smart;

void Init_smart( void ) {
  rsm_RDF  = rb_define_module( "RDF" );
  rsm_Smart = rb_define_class_under( rsm_RDF, "Smart", rb_cObject);
  rb_define_const(rsm_Smart, "VERSION", rb_str_new2(RSM_VERSION)); 

  rb_define_singleton_method(rsm_Smart, "new", (VALUE(*)(ANYARGS))rsm_new, -1);
  rb_define_method(rsm_Smart, "data_sources", (VALUE(*)(ANYARGS))rsm_data_sources, 0);
  rb_define_method(rsm_Smart, "namespaces", (VALUE(*)(ANYARGS))rsm_namespaces, 0);
  rb_define_private_method(rsm_Smart, "__execute", (VALUE(*)(ANYARGS))rsm_execute, 1);
}

