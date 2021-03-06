/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */
/**
 * @file  loc_json_api.c
 * @brief Basic JSON functions
 */

#include "loc_json_api.h"

#ifndef TRACE_GROUP
#define TRACE_GROUP "JSON"
#endif
#include "liveobjects-sys/loc_trace.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "platform_default.h"

static const char* _LO_json_dataTypeStr[LOD_TYPE_MAX_NOT_USED] = {
		"unknown",
		"i32", "i16", "i8",
		"u32", "u16", "u8",
		"str", "bool",
		"f64", "double"
		/*, "max" */
};

/* --------------------------------------------------------------------------------- */
/*  */
static const char* LO_dataType(LiveObjectsD_Type_t data_type) {
	switch (data_type) {
	case LOD_TYPE_UNKNOWN:
		return "xxx";
	case LOD_TYPE_INT32:
		return "i32";
	case LOD_TYPE_INT16:
		return "i16";
	case LOD_TYPE_INT8:
		return "i8";

	case LOD_TYPE_UINT32:
		return "u32";
	case LOD_TYPE_UINT16:
		return "u16";
	case LOD_TYPE_UINT8:
		return "u8";

	case LOD_TYPE_STRING_C:
		return "str";
	case LOD_TYPE_BOOL:
		return "bool";

	case LOD_TYPE_FLOAT:
		return "f64";
	case LOD_TYPE_DOUBLE:
		return "double";

	case LOD_TYPE_MAX_NOT_USED:
		return "max";
	}
	return "Unknown";
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_objTypeCheck(void) {
	int data_type;
	int err = 0;
	for (data_type = 1; data_type < LOD_TYPE_MAX_NOT_USED; data_type++) {
		const char* p = LO_dataType((LiveObjectsD_Type_t)data_type);
		if (strcmp(_LO_json_dataTypeStr[data_type], p)) {
			LOTRACE_ERR("objType %d - '%s' != '%s'", data_type, _LO_json_dataTypeStr[data_type], p);
			err++;
		}
	}
	return err;
}

/* --------------------------------------------------------------------------------- */
/*  */
const char* LO_getDataTypeToStr(LiveObjectsD_Type_t data_type) {
	if (data_type < LOD_TYPE_MAX_NOT_USED) {
		return _LO_json_dataTypeStr[data_type];
	}
	return "BAD";
}

/* --------------------------------------------------------------------------------- */
/*  */
LiveObjectsD_Type_t LO_getDataTypeFromStrL(const char* p, uint32_t len) {
	if ((p) &&(len > 0) && (len < 7)) {
		int i;
		for (i = 1; i < LOD_TYPE_MAX_NOT_USED; i++) {
			if (!strncmp(_LO_json_dataTypeStr[i], p, len)) {
				return ((LiveObjectsD_Type_t) i);
			}
		}
	}
	return LOD_TYPE_UNKNOWN;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_begin(char *pbuf, uint32_t sz) {
	int rc;
	rc = snprintf(pbuf, sz, "{");
	if (rc != 1) {
		LOTRACE_ERR("failed %d", rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_end(char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);
	if (*(pcur - 1) == ',') {
		pcur--;
		len++;
	}
	rc = snprintf(pcur, sz, "}");
	if (rc != 1) {
		LOTRACE_ERR("failed %d", rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_section_start(const char* section_name, char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);
	rc = snprintf(pcur, len, "\"%s\": {", section_name);
	if (rc < 0) {
		LOTRACE_ERR("(%s): failed, rc=%d", section_name, rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_section_end(char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);
	if (*(pcur - 1) == ',') {
		pcur--;
		len++;
	}
	rc = snprintf(pcur, len, "},");
	if (rc != 2) {
		LOTRACE_ERR("failed, rc=%d", rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_begin_section(char *pbuf, uint32_t sz, const char* section_name) {
	int rc;
	rc = snprintf(pbuf, sz, "{\"%s\":{", section_name);
	if (rc <= 0) {
		LOTRACE_ERR("failed %d", rc);
		return -1;
	}
	if (rc == (int) sz) {
		LOTRACE_ERR("too short %d", rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_end_section(char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);
	if (*(pcur - 1) == ',') {
		pcur--;
		len++;
	}
	rc = snprintf(pcur, len, "}}");
	if (rc != 2) {
		LOTRACE_ERR("failed %d != 2", rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_name_int(const char* name, int32_t value, char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);

	rc = snprintf(pcur, len, "\"%s\":%"PRIi32",", name, value);
	if (rc < 0) {
		LOTRACE_ERR("(%s, %"PRIi32"): failed, rc=%d", name, value, rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_name_str(const char* name, const char* value, char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);

	rc = snprintf(pcur, len, "\"%s\":\"%s\",", name, value);
	if (rc < 0) {
		LOTRACE_ERR("(%s, %s): failed, rc=%d", name, value, rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_name_array(const char* name, const char* array, char *pbuf, uint32_t sz) {
	int rc;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);

	rc = snprintf(pcur, len, "\"%s\":[%s],", name, array);
	if (rc < 0) {
		LOTRACE_ERR("(%s, %s): failed, rc=%d", name, array, rc);
		return -1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_item(const LiveObjectsD_Data_t* data_ptr, char *pbuf, uint32_t sz) {
	int rc;
	short i;
	short dim;
	char* data_value_ptr;
	int len = sz - strlen(pbuf);
	char* pcur = pbuf + strlen(pbuf);

	if (data_ptr == NULL) {
		LOTRACE_ERR("Invalid Arguments - data_ptr = NULL ");
		return -1;
	}
	if ((data_ptr->data_name == NULL) || (data_ptr->data_value == NULL) || (data_ptr->data_dim <= 0)) {
		LOTRACE_ERR("Invalid DataDef - name=%p  value=%p dim=%d", 
			data_ptr->data_name, data_ptr->data_value, data_ptr->data_dim);
		return -1;
	}
	
	rc = snprintf(pcur, len, "\"%s\":", data_ptr->data_name);
	if (rc < 0) {
		LOTRACE_ERR("(%d, %s): failed, rc=%d", data_ptr->data_type, data_ptr->data_name, rc);
		return -1;
	}
	len = sz - strlen(pbuf);
	pcur = pbuf + strlen(pbuf);
	if (len < 4) { /* at least 4 free bytes remmaining in buffer */
		LOTRACE_ERR("(%d, %s): failed, free len = %d < 4", data_ptr->data_type, data_ptr->data_name, len);
		return -1;
	}
	dim = data_ptr->data_dim;
	if (dim > 1) {
		*pcur++ = '[';
		len--;
	}
	else if (dim <= 0) {
		LOTRACE_ERR("(%d, %s): force dim=%d to 1", data_ptr->data_type, data_ptr->data_name, dim);
		dim = 1;
	}

	data_value_ptr = (char*)data_ptr->data_value;
	for (i=0;i<dim;i++) {
		switch (data_ptr->data_type) {
		case LOD_TYPE_INT32:
			rc = snprintf(pcur, len, "%"PRIi32",", *((int32_t*) data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(int32_t);
			break;
		case LOD_TYPE_INT16:
			rc = snprintf(pcur, len, "%"PRIi16",", *((int16_t*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(int16_t);			
			break;
		case LOD_TYPE_INT8:
			rc = snprintf(pcur, len, "%"PRIi8"," , *((int8_t*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(int8_t);
			break;
		case LOD_TYPE_UINT32:
			rc = snprintf(pcur, len, "%"PRIu32",", *((uint32_t*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(uint32_t);
			break;
		case LOD_TYPE_UINT16:
			rc = snprintf(pcur, len, "%"PRIu16",", *((uint16_t*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(uint16_t);
			break;
		case LOD_TYPE_UINT8:
			rc = snprintf(pcur, len, "%"PRIu8"," , *((uint8_t*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(uint8_t);
			break;
		case LOD_TYPE_FLOAT:
			rc = snprintf(pcur, len, "%f,", *((float*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(float);
			break;
		case LOD_TYPE_DOUBLE:
			rc = snprintf(pcur, len, "%lf,", *((double*)data_value_ptr));
			if (dim > 1) data_value_ptr += sizeof(double);
			break;
		case LOD_TYPE_BOOL:
			rc = snprintf(pcur, len, "%s,", *((uint8_t*)data_value_ptr) ? "true" : "false");
			if (dim > 1) data_value_ptr += sizeof(uint8_t);
			break;
		case LOD_TYPE_STRING_C:
			rc = snprintf(pcur, len, "\"%s\",", (const char*)data_value_ptr);
			if (dim > 1) data_value_ptr += sizeof(char*);
			break;
		default:
			LOTRACE_ERR("failed  - unknown type %d", data_ptr->data_type);
			return -1;
		}
		if (dim > 1) {
			len = sz - strlen(pbuf);
			pcur = pbuf + strlen(pbuf);
			if (len < 2) { /* at least 2 free bytes remmaining in buffer */
				LOTRACE_ERR("(%d, %s)[%d]: failed, free len = %d < 2", data_ptr->data_type, data_ptr->data_name, i, len);
				return -1;
			}		
		}
	}
	if (dim > 1) {
		pcur--;
		if ((*pcur != ',')  || (len < 2)){
			LOTRACE_ERR("(%d, %s): failed, unexpected char %c or free len = %d", data_ptr->data_type, data_ptr->data_name, *pcur, len);
			return -1;
		}
		*pcur++ = ']';
		*pcur++ = ',';
		*pcur = 0;
	}
	LOTRACE_DBG1("OK - type=%d=%s name=%s", data_ptr->data_type, LO_getDataTypeToStr(data_ptr->data_type),
			data_ptr->data_name);
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_json_add_param(const LiveObjectsD_Data_t* data_ptr, char *pbuf, uint32_t sz) {
	if ((data_ptr->data_type == LOD_TYPE_INT32) || (data_ptr->data_type == LOD_TYPE_UINT32)
			|| (data_ptr->data_type == LOD_TYPE_STRING_C) || (data_ptr->data_type == LOD_TYPE_FLOAT)) {
		int rc;
		int len = sz - strlen(pbuf);
		char* pcur = pbuf + strlen(pbuf);

		rc = snprintf(pcur, len, "\"%s\":{", data_ptr->data_name);
		if (rc < 0) {
			LOTRACE_ERR("(%d, %s): failed, rc=%d", data_ptr->data_type, data_ptr->data_name, rc);
			return -1;
		}

		len = sz - strlen(pbuf);
		pcur = pbuf + strlen(pbuf);
		switch (data_ptr->data_type) {
		case LOD_TYPE_INT32:
			rc = snprintf(pcur, len, "\"t\":\"i32\",\"v\":%d},", *((int*) data_ptr->data_value));
			break;
		case LOD_TYPE_UINT32:
			rc = snprintf(pcur, len, "\"t\":\"u32\",\"v\":%u},", *((unsigned int*) data_ptr->data_value));
			break;
		case LOD_TYPE_FLOAT:
			rc = snprintf(pcur, len, "\"t\":\"f64\",\"v\":%f},", *((float*) data_ptr->data_value));
			break;
		case LOD_TYPE_STRING_C:
			rc = snprintf(pcur, len, "\"t\":\"str\",\"v\":\"%s\"},", (const char*) data_ptr->data_value);
			break;
		default:
			LOTRACE_ERR("LO_json_add_param: failed -  type %d not implemented", data_ptr->data_type);
			return -1;
		}
		LOTRACE_DBG1("OK - type=%d=%s name=%s", data_ptr->data_type,
				LO_getDataTypeToStr(data_ptr->data_type), data_ptr->data_name);
		return 0;
	}
	LOTRACE_WARN("failed - unsupported obj_type = %d %s", data_ptr->data_type,
			LO_getDataTypeToStr(data_ptr->data_type));
	return -1;
}
