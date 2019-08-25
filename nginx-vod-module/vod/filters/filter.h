#ifndef __FILTER_H__
#define __FILTER_H__

// includes
#include "../input/read_cache.h"
#include "../media_set.h"

// functions
vod_status_t filter_init_filtered_clips(
	request_context_t* request_context,
	media_set_t* media_set);

vod_status_t filter_init_state(
	request_context_t* request_context,
	read_cache_state_t* read_cache_state,
	media_set_t* media_set, 
	void** context);

vod_status_t filter_run_state_machine(void* context);

#endif // __FILTER_H__
