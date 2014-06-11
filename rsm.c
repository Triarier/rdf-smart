static int rssm_print_graph_result(rasqal_query* rq,
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

static int rssm_print_formatted_query_results(rasqal_world* world,
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
  
  rc = rasqal_query_results_formatter_write(iostr, results_formatter,
                                            results, NULL);
  raptor_free_iostream(iostr);
  rasqal_free_query_results_formatter(results_formatter);

  if(rc)
    fprintf(stderr, "Formatting query results failed\n");

  return rc;
}


static rasqal_query* rssm_roqet_init_query(rasqal_world *world, 
                                      const char* ql_name,
                                      const unsigned char* query_string,
                                      raptor_sequence* data_graphs)
{
  rasqal_query* rq;
  rq = rasqal_new_query(world, (const char*)ql_name, NULL);
  if(!rq) {
    fprintf(stderr, "Failed to create query name %s\n", ql_name);
  }
  
  if(rasqal_query_prepare(rq, query_string, NULL)) {
    size_t len = strlen((const char*)query_string);
    
    fprintf(stderr, "Parsing query '");
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

static void rssm_execute(VALUE query, VALUE sources) { 
  unsigned char *query_string = (unsigned char*) "SELECT * WHERE { ?s ?p ?o }";
  rasqal_query *rq = NULL;
  rasqal_query_results *results;
  rasqal_world *world;
  raptor_world* raptor_world_ptr = NULL;
  char* output;
  size_t len;
  raptor_sequence* data_graphs = NULL;
  char* data_graph_parser_name = NULL;
  size_t data_sources_len = 2;

  char** data_sources = (char**) malloc(sizeof(char*) * data_sources_len);

  data_sources[0] ="http://solo.wst.univie.ac.at/rdf/hello.nt"; 
  data_sources[1]="http://solo.wst.univie.ac.at/rdf/nanda.ttl";

  world = rasqal_new_world();
  if(!world || rasqal_world_open(world)) {
    fprintf(stderr, "rasqal_world init failed\n");
    return(1);
  }
  raptor_world_ptr = rasqal_world_get_raptor(world);
  
  /* #{{{ Data_Graphs*/
  for(int i=0;i<data_sources_len;i++){
    const char* data_source=data_sources[i];
    rasqal_data_graph *dg = NULL;
    unsigned int type;

    type = RASQAL_DATA_GRAPH_BACKGROUND;

    if(!access((const char*)data_source, R_OK)) {
      unsigned char* source_uri_string;
      raptor_uri* source_uri;
      raptor_uri* graph_name = NULL;

      source_uri_string = raptor_uri_filename_to_uri_string((const char*)data_source);
      source_uri = raptor_new_uri(raptor_world_ptr, source_uri_string);
      raptor_free_memory(source_uri_string);

      
      if(source_uri)
        dg = rasqal_new_data_graph_from_uri(world,
                                            source_uri,
                                            graph_name,
                                            type,
                                            NULL, data_graph_parser_name,
                                            NULL);

      if(source_uri)
        raptor_free_uri(source_uri);
    } else {
      raptor_uri* source_uri;
      raptor_uri* graph_name = NULL;

      source_uri = raptor_new_uri(raptor_world_ptr,
                                  (const unsigned char*)data_source);
      
      if(source_uri)
        dg = rasqal_new_data_graph_from_uri(world,
                                            source_uri,
                                            graph_name,
                                            type,
                                            NULL, data_graph_parser_name,
                                            NULL);

      if(source_uri)
        raptor_free_uri(source_uri);
    }
    
    if(!dg) {
      fprintf(stderr, "Failed to create data graph for `%s'\n",
             data_source);
      return(1);
    }
    
    if(!data_graphs) {
      data_graphs = raptor_new_sequence((raptor_data_free_handler)rasqal_free_data_graph,
                                        NULL);

    if(!data_graphs) {
        fprintf(stderr, "Failed to create data graphs sequence\n");
        return(1);
      }
    }

    raptor_sequence_push(data_graphs, dg);
  }
/* #}}} */

  if (!(rq = roqet_init_query(world, "sparql", query_string,data_graphs)))
    goto tidy_query;

  results = rasqal_query_execute(rq);

  if(!results) {
    fprintf(stderr, "Query execution failed\n");
    goto tidy_query;
  }

  if(rasqal_query_results_is_bindings(results) || rasqal_query_results_is_boolean(results)) {
    print_formatted_query_results(world, results, raptor_world_ptr, "json", &output, &len);
  } else if(rasqal_query_results_is_graph(results)) {
    print_graph_result(rq, results, raptor_world_ptr, "json", &output, &len);
  } else {
    fprintf(stderr, "Query returned unknown result format\n");
  }
  rasqal_free_query_results(results);
  printf("Sequencelen: %d\n",raptor_sequence_size(data_graphs));
  printf("Length: %lu\n%s\n", len, output);
  
  free(data_sources);
  
  tidy_query:
    free(output);
    if(data_graphs) raptor_free_sequence(data_graphs);
    if(rq) rasqal_free_query(rq);
    rasqal_free_world(world);

  return (0);
}

VALUE rsm_RDF;
VALUE rsm_Smart;

void Init_rsm( void ) {
  rsm_RDF  = rb_define_module( "RDF" );
  rsm_Smart = rb_define_class_under( rsm_RDF, "Smart", rb_cObject);
  rb_define_const(rsm_Smart, "VERSION", rb_str_new2(RSM_VERSION)); 

  rb_define_singleton_method(rsm_Smart, "new", (VALUE(*)(ANYARGS))rsm_new, -1);
}

