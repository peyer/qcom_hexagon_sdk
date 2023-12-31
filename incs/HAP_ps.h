#ifndef HAP_PS_H
#define HAP_PS_H
/*==============================================================================
  Copyright (c) 2012-2019 Qualcomm Technologies Incorporated.
  All Rights Reserved Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S.
  Government. Diversion contrary to U.S. law prohibited.
==============================================================================*/

#define PROCESS_NAME_LEN 56
typedef struct HAP_process HAP_process;
struct HAP_process {
   char name[PROCESS_NAME_LEN];
   int32 asid;
   int32 hlos_pid;
};

int HAP_get_process_list(uint32* num_processes, HAP_process** processes);
int HAP_add_to_process_list(HAP_process* process);
int HAP_remove_from_process_list(int hlos_pid);
int HAP_set_process_name(char *name);
int HAP_thread_migrate(int tidQ);

/* Send signal to CPU for early wake up with approximate time to complete the job.
 * This signal helps to reduce fastrpc latency.
 * @param tidQ: QuRT thread id of a skel invoke thread.
 * @param earlyWakeTime: approximate time to complete job after sending the signal in us.
 * @return 0 on success
 */
int fastrpc_send_early_signal(uint32_t tidQ, uint32_t earlyWakeTime);

#endif /*HAP_PS_H */

