#include <rasqal.h>

int main(int argc, char *argv[]);

int main(int argc, char *argv[]) { 
  rasqal_world *world;

  for (int i=0; i<20; i++) { 
    world = rasqal_new_world();
    if(!world || rasqal_world_open(world)) {
      fprintf(stderr, "rasqal_world init failed\n");
      return(1);
    }
    rasqal_free_world(world);
  }
  return (0);
}

