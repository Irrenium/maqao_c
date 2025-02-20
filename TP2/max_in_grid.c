/* Author: Emmanuel OSERET, University of Versailles Saint-Quentin-en-Yvelines, France
 * This program pseudo-randomly generates pairs of values (v1,v2) in a 2D grid
 * and then computes and prints maximum (v1) and maximum (v2) and x-y position
 * Execution:
 * - Usage: ./exe <nb repetitions> <nb points X> <nb points Y>
 * - Nb repetitions: number of times the experiment is repeated (from file loading to memory release), allows to increase runtime for more precise sampling-based profiling
 * - Nb points X: input size along X
 * - Nb points Y: input size along Y
 * - Recommended: with the baseline implementation, good starting point is 2000 x 3000
 */
 #include <stdio.h>  // printf, fopen, etc.
#include <stdlib.h> // atoi, malloc, free, etc.

// Abstract values
typedef struct {
   float v1, v2;
} value_t;

// Dynamic array of values
typedef struct {
   unsigned nx, ny;  // number of values along X, Y
   value_t *entries; // array of values (contiguous block)
} value_grid_t;

// Structure to relate values and position in the grid
typedef struct {
   unsigned x, y; // position in the 2D grid
   float v1, v2;
} pos_val_t;

// Dynamic array of pos_val_t entries
typedef struct {
   unsigned nx, ny;  // number of values along X, Y
   pos_val_t *entries; // array of pos_val_t (contiguous block)
} pos_val_grid_t;

size_t sum_bytes = 0; // Cumulated sum of allocated bytes (malloc, realloc)

// Pseudo-randomly generates 'n' values and writes them to a binary file
int generate_random_values(const char *file_name, unsigned nx, unsigned ny) {
   printf("Generate %u x %u values and dump them to %s (binary)...\n", nx, ny, file_name);

   FILE *fp = fopen(file_name, "wb");
   if (!fp) {
      fprintf(stderr, "Cannot write to %s\n", file_name);
      return -1;
   }

   // Write dimensions (nx, ny)
   fwrite(&nx, sizeof(unsigned), 1, fp);
   fwrite(&ny, sizeof(unsigned), 1, fp);

   // Generate and write values
   for (unsigned i = 0; i < nx; i++) {
      for (unsigned j = 0; j < ny; j++) {
         value_t val = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
         fwrite(&val, sizeof(value_t), 1, fp);
      }
   }

   fclose(fp);
   return 0;
}

// Loads values from a binary file written by generate_random_values()
int load_values(const char *file_name, value_grid_t *val_grid) {
   printf("Load values from %s (binary)...\n", file_name);

   FILE *fp = fopen(file_name, "rb");
   if (!fp) {
      fprintf(stderr, "Cannot read %s\n", file_name);
      return -1;
   }

   // Read grid size
   if (fread(&val_grid->nx, sizeof(unsigned), 1, fp) != 1 ||
       fread(&val_grid->ny, sizeof(unsigned), 1, fp) != 1) {
      fprintf(stderr, "Failed to read grid size\n");
      fclose(fp);
      return 1;
   }

   // Allocate memory for the grid
   unsigned total_elements = val_grid->nx * val_grid->ny;
   val_grid->entries = malloc(total_elements * sizeof(value_t));
   if (!val_grid->entries) {
      fprintf(stderr, "Memory allocation failed\n");
      fclose(fp);
      return -1;
   }
   sum_bytes += total_elements * sizeof(value_t);

   // Read values
   if (fread(val_grid->entries, sizeof(value_t), total_elements, fp) != total_elements) {
      fprintf(stderr, "Failed to read values\n");
      free(val_grid->entries);
      fclose(fp);
      return 1;
   }

   fclose(fp);
   return 0;
}

// Relate pairs to coordinates
void load_positions(value_grid_t src, pos_val_grid_t *dst) {
   dst->nx = src.nx;
   dst->ny = src.ny;

   unsigned total_elements = src.nx * src.ny;

   // Allocate memory for positions
   dst->entries = malloc(total_elements * sizeof(pos_val_t));
   if (!dst->entries) {
      fprintf(stderr, "Memory allocation failed for positions\n");
      exit(EXIT_FAILURE);
   }
   sum_bytes += total_elements * sizeof(pos_val_t);

   // Populate the positions
   for (unsigned i = 0; i < total_elements; i++) {
      dst->entries[i].x = i / src.ny;
      dst->entries[i].y = i % src.ny;
      dst->entries[i].v1 = src.entries[i].v1;
      dst->entries[i].v2 = src.entries[i].v2;
   }
}

// Finds the maximum value of v1
pos_val_t *find_max_v1(const pos_val_grid_t *pv_grid) {
   printf("Compute maximum v1...\n");

   pos_val_t *max_val = &pv_grid->entries[0];
   unsigned total_elements = pv_grid->nx * pv_grid->ny;

   for (unsigned i = 1; i < total_elements; i++) {
      if (pv_grid->entries[i].v1 > max_val->v1) {
         max_val = &pv_grid->entries[i];
      }
   }
   return max_val;
}

// Finds the maximum value of v2
pos_val_t *find_max_v2(const pos_val_grid_t *pv_grid) {
   printf("Compute maximum v2...\n");

   pos_val_t *max_val = &pv_grid->entries[0];
   unsigned total_elements = pv_grid->nx * pv_grid->ny;

   for (unsigned i = 1; i < total_elements; i++) {
      if (pv_grid->entries[i].v2 > max_val->v2) {
         max_val = &pv_grid->entries[i];
      }
   }
   return max_val;
}

// Frees memory allocated for positions+values
void free_pos_val_grid(pos_val_grid_t *pv_grid) {
   free(pv_grid->entries);
   sum_bytes -= pv_grid->nx * pv_grid->ny * sizeof(pos_val_t);
}

// Frees memory allocated for values
void free_value_grid(value_grid_t *val_grid) {
   free(val_grid->entries);
   sum_bytes -= val_grid->nx * val_grid->ny * sizeof(value_t);
}

// Program entry point
int main(int argc, char *argv[]) {
   if (argc < 4) {
      fprintf(stderr, "Usage: %s <nb repetitions> <nb points X> <nb points Y>\n", argv[0]);
      return EXIT_FAILURE;
   }

   unsigned nrep = (unsigned)atoi(argv[1]);
   unsigned nx = (unsigned)atoi(argv[2]);
   unsigned ny = (unsigned)atoi(argv[3]);

   const char *input_file_name = "values.bin";
   if (generate_random_values(input_file_name, nx, ny) != 0) {
      fprintf(stderr, "Failed to write %u x %u coordinates to %s\n", nx, ny, input_file_name);
      return EXIT_FAILURE;
   }

   for (unsigned r = 0; r < nrep; r++) {
      value_grid_t value_grid;
      pos_val_grid_t pos_val_grid;

      if (load_values(input_file_name, &value_grid) != 0) {
         fprintf(stderr, "Failed to load coordinates\n");
         return EXIT_FAILURE;
      }

      load_positions(value_grid, &pos_val_grid);

      const pos_val_t *pos_v1_max = find_max_v1(&pos_val_grid);
      const pos_val_t *pos_v2_max = find_max_v2(&pos_val_grid);

      printf("Max v1: x=%u, y=%u, v1=%f\n", pos_v1_max->x, pos_v1_max->y, pos_v1_max->v1);
      printf("Max v2: x=%u, y=%u, v2=%f\n", pos_v2_max->x, pos_v2_max->y, pos_v2_max->v2);

      free_pos_val_grid(&pos_val_grid);
      free_value_grid(&value_grid);
   }

   remove(input_file_name);
   return EXIT_SUCCESS;
}