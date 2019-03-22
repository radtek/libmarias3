/* vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * Copyright 2019 MariaDB Corporation Ab. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

#include "config.h"
#include "common.h"

ms3_st *ms3_init(const char *s3key, const char *s3secret, const char *region, const char *base_domain)
{
  if ((s3key == NULL) or (s3secret == NULL))
  {
    return NULL;
  }

  if ((strlen(s3key) < 20) or (strlen(s3secret) < 40))
  {
    return NULL;
  }

  // Note: this means ms3_init is not thread safe
  curl_global_init(CURL_GLOBAL_DEFAULT);

  ms3_st *ms3 = malloc(sizeof(ms3_st));

  memcpy(ms3->s3key, s3key, 20);
  memcpy(ms3->s3secret, s3secret, 40);
  ms3->region = strdup(region);
  if (base_domain)
  {
    ms3->base_domain = strdup(base_domain);
  }
  else
  {
    ms3->base_domain = NULL;
  }

  return ms3;
}

void ms3_deinit(ms3_st *ms3)
{
  ms3debug("deinit: 0x%" PRIXPTR, (uintptr_t)ms3);

  free(ms3);
}

void ms3_debug(bool state)
{
  ms3debug_set(state);
  if (state)
  {
    ms3debug("enabling debug");
  }
}

const char *ms3_error(uint8_t errcode)
{
  if (errcode >= MS3_ERR_MAX)
  {
    return baderror;
  }
  return errmsgs[errcode];
}

uint8_t ms3_list(ms3_st *ms3, const char *bucket, const char *prefix, ms3_list_st **list)
{
  // TODO: make pagination work (IsTruncated)
  (void) prefix;
  uint8_t res= 0;
  if (not ms3 or not bucket or not list)
  {
    return MS3_ERR_PARAMETER;
  }
  res= execute_request(ms3, MS3_CMD_LIST, bucket, NULL, prefix, NULL, 0, list);
  return res;
}

uint8_t ms3_put(ms3_st *ms3, const char *bucket, const char *key, const uint8_t *data, size_t length)
{
  uint8_t res;
  if (not ms3 or not bucket or not key or not data)
  {
    return MS3_ERR_PARAMETER;
  }

  if (length == 0)
  {
    return MS3_ERR_NO_DATA;
  }

  res= execute_request(ms3, MS3_CMD_PUT, bucket, key, NULL, data, length, NULL);

  return res;
}

uint8_t ms3_get(ms3_st *ms3, const char *bucket, const char *key, uint8_t **data, size_t *length)
{
  uint8_t res= 0;
  memory_buffer_st buf;
  if (not ms3 or not bucket or not key or not data or not length)
  {
    return MS3_ERR_PARAMETER;
  }
  res= execute_request(ms3, MS3_CMD_GET, bucket, key, NULL, NULL, 0, &buf);
  *data= buf.data;
  *length= buf.length;
  return res;
}

uint8_t ms3_delete(ms3_st *ms3, const char *bucket, const char *key)
{
  uint8_t res;
  if (not ms3 or not bucket or not key)
  {
    return MS3_ERR_PARAMETER;
  }
  res= execute_request(ms3, MS3_CMD_DELETE, bucket, key, NULL, NULL, 0, NULL);
  return res;
}

uint8_t ms3_status(ms3_st *ms3, const char *bucket, const char *key, ms3_status_st *status)
{
  uint8_t res;
  if (not ms3 or not bucket or not key or not status)
  {
    return MS3_ERR_PARAMETER;
  }
  res= execute_request(ms3, MS3_CMD_HEAD, bucket, key, NULL, NULL, 0, status);
  return res;
}

void ms3_list_free(ms3_list_st *list)
{
  ms3_list_st *tmp;

  while (list)
  {
    free(list->key);
    tmp= list;
    list= list->next;
    free(tmp);
  }
}