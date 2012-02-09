/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include <girara/types.h>

#include "document.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct page_offset_s
{
  int x;
  int y;
} page_offset_t;

/**
 * Checks if the given file exists
 *
 * @param path Path to the file
 * @return true if the file exists, otherwise false
 */
bool file_exists(const char* path);

/**
 * Returns the file extension of a path
 *
 * @param path Path to the file
 * @return The file extension or NULL
 */
const char* file_get_extension(const char* path);

/**
 * This function checks if the file has a valid extension. A extension is
 * evaluated as valid if it matches a supported filetype.
 *
 * @param zathura Zathura object
 * @param path The path to the file
 * @return true if the extension is valid, otherwise false
 */
bool file_valid_extension(zathura_t* zathura, const char* path);

/**
 * Executes a system command and saves its output into output
 *
 * @param argv The command
 * @param output Pointer where the output will be saved
 * @return true if no error occured, otherwise false
 */
bool execute_command(char* const argv[], char** output);

/**
 * Generates the document index based upon the list retreived from the document
 * object.
 *
 * @param model The tree model
 * @param parent The tree iterator parent
 * @param tree The Tree iterator
 */
void document_index_build(GtkTreeModel* model, GtkTreeIter* parent, girara_tree_node_t* tree);

/**
 * Calculates the offset of the page to the top of the viewing area as
 * well as to the left side of it. The result has to be freed.
 *
 * @param page The Page
 * @param offset Applied offset
 * @return The calculated offset or NULL if an error occured
 */
void page_calculate_offset(zathura_page_t* page, page_offset_t* offset);

/**
 * Rotate a rectangle by 0, 90, 180 or 270 degree
 * @param rect the rectangle to rotate
 * @param degree rotation degree
 * @param height the height of the enclosing rectangle
 * @param width the width of the enclosing rectangle
 * @return the rotated rectangle
 */
zathura_rectangle_t rotate_rectangle(zathura_rectangle_t rectangle, unsigned int degree, int height, int width);

/**
 * Calculates the new coordinates based on the rotation and scale level of the
 * document for the given rectangle
 *
 * @param page Page where the rectangle should be
 * @param rectangle The rectangle
 * @return New rectangle
 */
zathura_rectangle_t recalc_rectangle(zathura_page_t* page, zathura_rectangle_t rectangle);

/**
 * Set adjustment of a GtkAdjustment respecting its limits.
 * @param adjust the GtkAdkustment instance
 * @param value the new adjustment
 */
void set_adjustment(GtkAdjustment* adjust, gdouble value);

/**
 * Calculate the page size according to the corrent scaling and rotation if
 * desired.
 * @param page the page
 * @param page_height the resulting page height
 * @param page_width the resultung page width
 * @param rotate honor page's rotation
 */
void
page_calc_height_width(zathura_page_t* page, unsigned int* page_height, unsigned int* page_width, bool rotate);


#endif // UTILS_H