/* See LICENSE file for license and copyright information */

#include "annotations.h"

#include <stdlib.h>
#include <string.h>

struct zathura_annotation_s {
  zathura_annotation_type_t type; /**< Type of the annotation */
  zathura_annotation_flag_t flags; /**< Flags of the annotation */
  char* name; /**< Name of the annotation */
  char* content; /**< Content of the annotation */
  time_t modification_date; /**< Modification date */
  zathura_page_t* page; /**< Zathura page */
  void* data; /**< Custom data */
};

static char* __strdup(const char* string);

zathura_annotation_t*
zathura_annotation_new(zathura_annotation_type_t type)
{
  zathura_annotation_t* annotation = calloc(1, sizeof(zathura_annotation_t));
  if (annotation == NULL) {
    return NULL;
  }

  switch (type) {
    case ZATHURA_ANNOTATION_TEXT:
    case ZATHURA_ANNOTATION_FREE_TEXT:
    case ZATHURA_ANNOTATION_FILE_ATTACHMENT:
    case ZATHURA_ANNOTATION_MOVIE:
    case ZATHURA_ANNOTATION_SCREEN:
      break;
    default:
      free(annotation);
      return NULL;
  }

  annotation->type = type;

  return annotation;
}

void
zathura_annotation_free(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return;
  }

  if (annotation->name) {
    free(annotation->name);
  }

  if (annotation->content) {
    free(annotation->content);
  }

  free(annotation);
}

void*
zathura_annotation_get_data(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return NULL;
  }

  return annotation->data;
}

void
zathura_annotation_set_data(zathura_annotation_t* annotation, void* data)
{
  if (annotation == NULL) {
    return;
  }

  annotation->data = data;
}

zathura_annotation_type_t
zathura_annotation_get_type(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return ZATHURA_ANNOTATION_UNKNOWN;
  }

  return annotation->type;
}

zathura_annotation_flag_t
zathura_annotation_get_flags(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return ZATHURA_ANNOTATION_FLAG_UNKNOWN;
  }

  return annotation->flags;
}

void
zathura_annotation_set_flags(zathura_annotation_t* annotation, zathura_annotation_flag_t flags)
{
  if (annotation == NULL) {
    return;
  }

  annotation->flags = flags;
}

char*
zathura_annotation_get_content(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return NULL;
  }

  return annotation->content;
}

void
zathura_annotation_set_content(zathura_annotation_t* annotation, char* content)
{
  if (annotation == NULL) {
    return;
  }

  if (content != NULL) {
    annotation->content = __strdup(content);
  } else {
    annotation->content = NULL;
  }
}

char*
zathura_annotation_get_name(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return NULL;
  }

  return annotation->name;
}

void
zathura_annotation_set_name(zathura_annotation_t* annotation, const char* name)
{
  if (annotation == NULL) {
    return;
  }

  if (name != NULL) {
    annotation->name = __strdup(name);
  } else {
    annotation->name = NULL;
  }
}

time_t
zathura_annotation_get_modified(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return (time_t) -1;
  }

  return annotation->modification_date;
}

void
zathura_annotation_set_modified(zathura_annotation_t* annotation, time_t modification_date)
{
  if (annotation == NULL) {
    return;
  }

  annotation->modification_date = modification_date;
}

zathura_page_t*
zathura_annotation_get_page_index(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return NULL;
  }

  return annotation->page;
}

void
zathura_annotation_set_page(zathura_annotation_t* annotation, zathura_page_t* page)
{
  if (annotation == NULL) {
    return;
  }

  annotation->page = page;
}

static char*
__strdup(const char* string)
{
  if (string == NULL) {
    return NULL;
  }

  size_t length = strlen(string) + 1;

  char* new_string = malloc(sizeof(char) * length);
  if (new_string == NULL) {
    return NULL;
  }

  return (char*) memcpy(new_string, string, length);
}