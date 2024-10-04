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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/filter.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include "dm_bss_list.h"
#include "dm_easy_mesh.h"
#include "dm_easy_mesh_ctrl.h"

int dm_bss_list_t::get_config(cJSON *obj_arr, void *parent, bool summary)
{
    dm_bss_t *pbss;
    cJSON *obj, *akms_arr;
    mac_addr_str_t  mac_str;
    mac_address_t	ruid;
    unsigned int i;

    dm_easy_mesh_t::string_to_macbytes((char *)parent, ruid);

    pbss = get_first_bss();
    printf("%s:%d: pbss: %p\n", __func__, __LINE__, pbss);
    
    while (pbss != NULL) {
        dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.ruid.mac, mac_str);
        printf("%s:%d: Parent Radio: %s, Current BSS Radio: %s\n", __func__, __LINE__, (char *)parent, mac_str);
        if (memcmp(pbss->m_bss_info.ruid.mac, ruid, sizeof(mac_address_t)) != 0) {
            pbss = get_next_bss(pbss);
            continue;
        }

        obj = cJSON_CreateObject(); 

	dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.bssid.mac, mac_str);
	cJSON_AddStringToObject(obj, "bssid", mac_str);
	cJSON_AddStringToObject(obj, "ssid", pbss->m_bss_info.ssid);
	cJSON_AddBoolToObject(obj, "Enabled", pbss->m_bss_info.enabled);
	cJSON_AddStringToObject(obj, "EstServiceParametersBE", pbss->m_bss_info.est_svc_params_be);
	cJSON_AddStringToObject(obj, "EstServiceParametersBK", pbss->m_bss_info.est_svc_params_bk);
	cJSON_AddStringToObject(obj, "EstServiceParametersVI", pbss->m_bss_info.est_svc_params_vi);
	cJSON_AddStringToObject(obj, "EstServiceParametersVO", pbss->m_bss_info.est_svc_params_vo);

        akms_arr = cJSON_AddArrayToObject(obj, "FronthaulAKMsAllowed");
        for (i = 0; i < pbss->m_bss_info.num_fronthaul_akms; i++) {
            cJSON_AddItemToArray(akms_arr, cJSON_CreateString(pbss->m_bss_info.fronthaul_akm[i]));
        }

        akms_arr = cJSON_AddArrayToObject(obj, "BackhaulAKMsAllowed");
        for (i = 0; i < pbss->m_bss_info.num_backhaul_akms; i++) {
            cJSON_AddItemToArray(akms_arr, cJSON_CreateString(pbss->m_bss_info.backhaul_akm[i]));
        }

	cJSON_AddBoolToObject(obj, "Profile1bSTAsDisallowed", pbss->m_bss_info.profile_1b_sta_allowed);
	cJSON_AddBoolToObject(obj, "Profile2bSTAsDisallowed", pbss->m_bss_info.profile_2b_sta_allowed);

	cJSON_AddNumberToObject(obj, "AssociationAllowanceStatus", pbss->m_bss_info.assoc_allowed_status);

	cJSON_AddBoolToObject(obj, "FronthaulUse", pbss->m_bss_info.fronthaul_use);
	cJSON_AddBoolToObject(obj, "BackhaulUse", pbss->m_bss_info.backhaul_use);
	cJSON_AddBoolToObject(obj, "R1disallowed", pbss->m_bss_info.r1_disallowed);
	cJSON_AddBoolToObject(obj, "R2disallowed", pbss->m_bss_info.r2_disallowed);
	cJSON_AddBoolToObject(obj, "MultiBSSID", pbss->m_bss_info.multi_bssid);
	cJSON_AddBoolToObject(obj, "TransmittedBSSID", pbss->m_bss_info.transmitted_bssid);

	cJSON_AddItemToArray(obj_arr, obj);
	pbss = get_next_bss(pbss);
    }
    
	
    return 0;
}

int dm_bss_list_t::set_config(db_client_t& db_client, dm_bss_t& bss, void *parent_id)
{
    dm_orch_type_t op;  
    mac_addr_str_t  bss_mac_str, radio_mac_str;
                        
    dm_easy_mesh_t::macbytes_to_string(bss.m_bss_info.bssid.mac, bss_mac_str);
    dm_easy_mesh_t::macbytes_to_string(bss.m_bss_info.ruid.mac, radio_mac_str);
    printf("%s:%d: Enter: BSSID: %s RadioID: %s\n", __func__, __LINE__, bss_mac_str, radio_mac_str);

    update_db(db_client, (op = get_dm_orch_type(db_client, bss)), bss.get_bss_info());
    update_list(bss, op);
                        
    return 0;
}

int dm_bss_list_t::set_config(db_client_t& db_client, const cJSON *obj_arr, void *parent_id)
{
    cJSON *obj;
    unsigned int i, size;
    dm_bss_t bss;
    dm_orch_type_t op;

    size = cJSON_GetArraySize(obj_arr);

    for (i = 0; i < size; i++) {
        obj = cJSON_GetArrayItem(obj_arr, i);
	bss.decode(obj, parent_id);
	update_db(db_client, (op = get_dm_orch_type(db_client, bss)), bss.get_bss_info());
	update_list(bss, op);
    }

    return 0;
}

dm_orch_type_t dm_bss_list_t::get_dm_orch_type(db_client_t& db_client, const dm_bss_t& bss)
{
    dm_bss_t *pbss;
    mac_addr_str_t  bss_mac_str, radio_mac_str;

    dm_easy_mesh_t::macbytes_to_string((unsigned char *)bss.m_bss_info.bssid.mac, bss_mac_str);
    dm_easy_mesh_t::macbytes_to_string((unsigned char *)bss.m_bss_info.ruid.mac, radio_mac_str);

    pbss = get_bss(bss_mac_str);

    if (pbss != NULL) {
        if (entry_exists_in_table(db_client, bss_mac_str) == false) {
            return dm_orch_type_bss_insert;
        }

        if (*pbss == bss) {
            printf("%s:%d: BSS: %s Radio: %s already in list\n", __func__, __LINE__, 
            dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.bssid.mac, bss_mac_str),
            dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.ruid.mac, radio_mac_str));
            return dm_orch_type_none;
        }


        printf("%s:%d: BSS: %s Radio: %s in list but needs update\n", __func__, __LINE__,
        dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.bssid.mac, bss_mac_str),
        dm_easy_mesh_t::macbytes_to_string(pbss->m_bss_info.ruid.mac, radio_mac_str));
        return dm_orch_type_bss_update;
    }

    return dm_orch_type_bss_insert;
}


void dm_bss_list_t::update_list(const dm_bss_t& bss, dm_orch_type_t op)
{
    dm_bss_t *pbss;
    mac_addr_str_t	bss_mac_str, radio_mac_str;

    dm_easy_mesh_t::macbytes_to_string((unsigned char *)bss.m_bss_info.bssid.mac, bss_mac_str);
    dm_easy_mesh_t::macbytes_to_string((unsigned char *)bss.m_bss_info.ruid.mac, radio_mac_str);

    printf("%s:%d: BSSID: %s Radio ID: %s\n", __func__, __LINE__, bss_mac_str, radio_mac_str);

    switch (op) {
        case dm_orch_type_bss_insert:
            put_bss(bss_mac_str, &bss);
            break;

        case dm_orch_type_bss_update:
			pbss = get_bss(bss_mac_str);
            memcpy(&pbss->m_bss_info, &bss.m_bss_info, sizeof(em_bss_info_t));
            break;

        case dm_orch_type_bss_delete:
            remove_bss(bss_mac_str);            
            break;
    }

}

void dm_bss_list_t::delete_list()
{       
    dm_bss_t *pbss, *tmp;
    mac_addr_str_t	bss_mac_str, radio_mac_str;
    em_long_string_t key;
    
    pbss = get_first_bss();
    while (pbss != NULL) {
        tmp = pbss;
        pbss = get_next_bss(pbss);
    
        dm_easy_mesh_t::macbytes_to_string((unsigned char *)tmp->m_bss_info.bssid.mac, bss_mac_str);
        dm_easy_mesh_t::macbytes_to_string((unsigned char *)tmp->m_bss_info.ruid.mac, radio_mac_str);
        snprintf(key, sizeof (em_long_string_t), "%s-%s", bss_mac_str, radio_mac_str);
        remove_bss(key);
    }
}   


bool dm_bss_list_t::operator == (const db_easy_mesh_t& obj)
{
    return true;
}

int dm_bss_list_t::update_db(db_client_t& db_client, dm_orch_type_t op, void *data)
{
    mac_addr_str_t bss_mac_str, radio_mac_str;
    em_bss_info_t *info = (em_bss_info_t *)data;
    int ret = 0;
    unsigned int i;
    em_long_string_t	front_akms, back_akms;

    //printf("%s:%d: Opeartion:%d\n", __func__, __LINE__, op);
    memset(front_akms, 0, sizeof(em_long_string_t));
    for (i = 0; i < info->num_fronthaul_akms; i++) {
        strncat(front_akms, info->fronthaul_akm[i], strlen(info->fronthaul_akm[i]));
        strncat(front_akms, ",", strlen(","));
    }
    front_akms[strlen(front_akms) - 1] = 0;
	
    memset(back_akms, 0, sizeof(em_long_string_t));
    for (i = 0; i < info->num_backhaul_akms; i++) {
        strncat(back_akms, info->backhaul_akm[i], strlen(info->backhaul_akm[i]));
        strncat(back_akms, ",", strlen(","));
    }
    back_akms[strlen(back_akms) - 1] = 0;
	
	switch (op) {
		case dm_orch_type_bss_insert:
			ret = insert_row(db_client, dm_easy_mesh_t::macbytes_to_string(info->bssid.mac, bss_mac_str),
						dm_easy_mesh_t::macbytes_to_string(info->ruid.mac, radio_mac_str), info->ssid, info->enabled,
						info->est_svc_params_be, info->est_svc_params_bk, info->est_svc_params_vi, info->est_svc_params_vo,
						front_akms, back_akms, info->profile_1b_sta_allowed, info->profile_2b_sta_allowed, info->assoc_allowed_status,
						info->backhaul_use, info->fronthaul_use, info->r1_disallowed, info->r2_disallowed, 
						info->multi_bssid, info->transmitted_bssid);
							
			break;

		case dm_orch_type_bss_update:
			ret = update_row(db_client, dm_easy_mesh_t::macbytes_to_string(info->ruid.mac, radio_mac_str), info->ssid, info->enabled,
                        info->est_svc_params_be, info->est_svc_params_bk, info->est_svc_params_vi, info->est_svc_params_vo,
                        front_akms, back_akms, info->profile_1b_sta_allowed, info->profile_2b_sta_allowed, info->assoc_allowed_status,
                        info->backhaul_use, info->fronthaul_use, info->r1_disallowed, info->r2_disallowed, 
                        info->multi_bssid, info->transmitted_bssid,
						dm_easy_mesh_t::macbytes_to_string(info->bssid.mac, bss_mac_str));
			break;

		case dm_orch_type_bss_delete:
			ret = delete_row(db_client, dm_easy_mesh_t::macbytes_to_string(info->bssid.mac, bss_mac_str));
			break;

		default:
			break;
	}

    return ret;
}

bool dm_bss_list_t::search_db(db_client_t& db_client, void *ctx, void *key)
{
    mac_addr_str_t	mac;

    while (db_client.next_result(ctx)) {
        db_client.get_string(ctx, mac, 1);

        if (strncmp(mac, (char *)key, strlen((char *)key)) == 0) {
            return true;
        }
    }

    return false;
}

int dm_bss_list_t::sync_db(db_client_t& db_client, void *ctx)
{
    em_bss_info_t info;
    mac_addr_str_t	mac;
    em_short_string_t   str;
    unsigned int i;
    char   *token_parts[EM_MAX_AKMS];
    int rc = 0;

    while (db_client.next_result(ctx)) {
        memset(&info, 0, sizeof(em_bss_info_t));

	db_client.get_string(ctx, mac, 1);
	dm_easy_mesh_t::string_to_macbytes(mac, info.bssid.mac);

	db_client.get_string(ctx, mac, 2);
	dm_easy_mesh_t::string_to_macbytes(mac, info.ruid.mac);

	db_client.get_string(ctx, info.ssid, 3);
        info.enabled = db_client.get_number(ctx, 4);

	db_client.get_string(ctx, info.est_svc_params_be, 5);
	db_client.get_string(ctx, info.est_svc_params_bk, 6);
	db_client.get_string(ctx, info.est_svc_params_vi, 7);
	db_client.get_string(ctx, info.est_svc_params_vo, 8);

        db_client.get_string(ctx, str, 9);
	for (i = 0; i < EM_MAX_AKMS; i++) {
            token_parts[i] = info.fronthaul_akm[i];
        }
        info.num_fronthaul_akms = get_strings_by_token(str, ',', EM_MAX_AKMS, token_parts);

        db_client.get_string(ctx, str, 10);
	for (i = 0; i < EM_MAX_AKMS; i++) {
            token_parts[i] = info.backhaul_akm[i];
        }
        info.num_backhaul_akms = get_strings_by_token(str, ',', EM_MAX_AKMS, token_parts);


	info.profile_1b_sta_allowed = db_client.get_number(ctx, 11);
	info.profile_2b_sta_allowed = db_client.get_number(ctx, 12);
	info.assoc_allowed_status = db_client.get_number(ctx, 13);
	info.backhaul_use = db_client.get_number(ctx, 14);
	info.fronthaul_use = db_client.get_number(ctx, 15);
	info.r1_disallowed = db_client.get_number(ctx, 16);
	info.r2_disallowed = db_client.get_number(ctx, 17);
	info.multi_bssid = db_client.get_number(ctx, 18);
	info.transmitted_bssid = db_client.get_number(ctx, 19);
        
	update_list(dm_bss_t(&info), dm_orch_type_bss_insert);
    }

    return rc;

}

void dm_bss_list_t::init_table()
{
    snprintf(m_table_name, sizeof(m_table_name), "%s", "BSSList");
}

void dm_bss_list_t::init_columns()
{
    m_num_cols = 0;

    m_columns[m_num_cols++] = db_column_t("ID", db_data_type_char, 17);
    m_columns[m_num_cols++] = db_column_t("RadioID", db_data_type_char, 17);
    m_columns[m_num_cols++] = db_column_t("SSID", db_data_type_char, 32);
    m_columns[m_num_cols++] = db_column_t("Enabled", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("EstServiceParametersBE", db_data_type_char, 16);
    m_columns[m_num_cols++] = db_column_t("EstServiceParametersBK", db_data_type_char, 16);
    m_columns[m_num_cols++] = db_column_t("EstServiceParametersVI", db_data_type_char, 16);
    m_columns[m_num_cols++] = db_column_t("EstServiceParametersVO", db_data_type_char, 16);
    m_columns[m_num_cols++] = db_column_t("FronthaulAKMsAllowed", db_data_type_char, 64);
    m_columns[m_num_cols++] = db_column_t("BackhaulAKMsAllowed", db_data_type_char, 64);
    m_columns[m_num_cols++] = db_column_t("Profile1bSTAsDisallowed", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("Profile2bSTAsDisallowed", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("AssociationAllowanceStatus", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("FronthaulUse", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("BackhaulUse", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("R1disallowed", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("R2disallowed", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("MultiBSSID", db_data_type_tinyint, 0);
    m_columns[m_num_cols++] = db_column_t("TransmittedBSSID", db_data_type_tinyint, 0);
}

int dm_bss_list_t::init()
{
    init_table();
    init_columns();
    return 0;
}