/**
 * Copyright 2023 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef EM_CMD_OP_CHANNEL_REPORT_H
#define EM_CMD_OP_CHANNEL_REPORT_H

#include "em_cmd.h"

class em_cmd_op_channel_report_t : public em_cmd_t {

public:
    
	/**!
	 * @brief 
	 *
	 * This function handles the operation of channel reporting.
	 *
	 * @param[in] param The command parameters required for the operation.
	 * @param[out] dm The easy mesh data structure that will be updated with the channel report.
	 *
	 * @returns em_cmd_op_channel_report_t
	 * @retval Returns a status code indicating the success or failure of the operation.
	 *
	 * @note Ensure that the dm structure is properly initialized before calling this function.
	 */
	em_cmd_op_channel_report_t(em_cmd_params_t param, dm_easy_mesh_t& dm);
};

#endif

