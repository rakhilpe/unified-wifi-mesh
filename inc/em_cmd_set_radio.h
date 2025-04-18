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

#ifndef EM_CMD_SET_RADIO_H
#define EM_CMD_SET_RADIO_H

#include "em_cmd.h"

class em_cmd_set_radio_t : public em_cmd_t {

public:
    
	/**!
	 * @brief Sets the radio command parameters.
	 *
	 * This function configures the radio settings using the provided parameters.
	 *
	 * @param[in] param The command parameters to set the radio.
	 * @param[out] dm The easy mesh data structure to be updated.
	 *
	 * @returns Status of the operation.
	 * @retval 0 on success.
	 * @retval -1 on failure.
	 *
	 * @note Ensure that the parameters are valid before calling this function.
	 */
	em_cmd_set_radio_t(em_cmd_params_t param, dm_easy_mesh_t& dm);
};

#endif
