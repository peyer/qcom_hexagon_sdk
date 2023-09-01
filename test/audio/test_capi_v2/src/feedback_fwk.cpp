/*========================================================================
 $Header: //components/dev/avs.adsp/2.7/roverma.AVS.ADSP.2.7.jun_17_27/elite/module_interfaces/utils/src/feedback_module.cpp#1 $

 Edit History

 when       who     what, where, why
 --------   ---     -------------------------------------------------------
 7/22/2014   rv       Created

 ==========================================================================*/

/*-----------------------------------------------------------------------
 Copyright (c) 2012-2015 Qualcomm  Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------*/

/*============================================================================
 *                       INCLUDE FILES FOR MODULE
 *==========================================================================*/
#include "feedback_fwk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HAP_farf.h"
#include "test_profile.h"


#define malloc __wrap_malloc
#define free __wrap_free



typedef struct node_t
{
   void *element; // This node
   struct node_t *next; // ptr to next node
} node_t;

typedef struct queue_t
{
   node_t *front_ptr;
   int8_t num_nodes;
} queue_t;


/* Enqueing the queue */
capi_v2_err_t en_queue_back(void *queue_handle, void *element)
{
   queue_t *queue_ptr = (queue_t *)queue_handle;
   node_t *temp_ptr = (node_t *)malloc(sizeof(node_t));
   node_t **cur_pptr = &queue_ptr->front_ptr;

   while(*cur_pptr!=NULL)
   {
      cur_pptr = &(*cur_pptr)->next;
   }

   temp_ptr->element = element;
   temp_ptr->next = NULL;
   *cur_pptr = temp_ptr;

   queue_ptr->num_nodes++;

   return CAPI_V2_EOK;
}

/* Dequeing the queue from front*/
capi_v2_err_t de_queue_front(void *queue_handle, void **element_pptr)
{
   queue_t *queue_ptr = (queue_t *)queue_handle;
   node_t *temp_ptr;

   if(queue_ptr->num_nodes == 0)
   {
      FARF(ERROR, "De-queing from empty Q!");
      return CAPI_V2_EFAILED ;
   }
   else
   {
      temp_ptr = queue_ptr->front_ptr;
      *element_pptr = temp_ptr->element;
      queue_ptr->front_ptr = temp_ptr->next;
      free(temp_ptr);
      queue_ptr->num_nodes--;
   }

   return CAPI_V2_EOK;
}



capi_v2_err_t destroy_queue(void **queue_handle_ptr)
{
   queue_t *queue_ptr = (queue_t *)*queue_handle_ptr;

   if(NULL == queue_ptr)
   {
      FARF(ERROR, "Cannot destroy NULL Q!");
      return CAPI_V2_EFAILED ;
   }

   while(0 != queue_ptr->num_nodes)
   {
      void *element_ptr = NULL;
      // Free up the elements and node
      de_queue_front(queue_ptr, &element_ptr);
      free(element_ptr);
   }

   // Now the Q should be empty
   free(queue_ptr);

   *queue_handle_ptr = NULL;

   return CAPI_V2_EOK;

}

capi_v2_err_t create_queue(void **queue_handle_ptr)
{
   *queue_handle_ptr = malloc(sizeof(queue_t));
   if(NULL == *queue_handle_ptr)
   {
      FARF(ERROR, "Cannot alloc memory for Q!");
      return CAPI_V2_ENOMEMORY;
   }

   memset(*queue_handle_ptr, 0, sizeof(queue_t));

   return CAPI_V2_EOK;
}


