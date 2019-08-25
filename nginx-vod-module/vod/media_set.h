#ifndef __MEDIA_SET_H__
#define __MEDIA_SET_H__

// includes
#include "mp4/mp4_aes_ctr.h"
#include "media_format.h"
#include "media_clip.h"
#include "json_parser.h"

// constants
#define SEGMENT_BASE_TIME_RELATIVE (ULLONG_MAX)
#define INVALID_SEQUENCE_INDEX (UINT_MAX)
#define INVALID_SEGMENT_INDEX (UINT_MAX)
#define INVALID_SEGMENT_TIME (LLONG_MAX)
#define INVALID_CLIP_INDEX (UINT_MAX)

#define MAX_LOOK_AHEAD_SEGMENTS (2)
#define MAX_NOTIFICATIONS (1024)
#define MAX_CLIPS (128)
#define MAX_CLIPS_PER_REQUEST (16)
#define MAX_SEQUENCES (32)
#define MAX_SOURCES (32)

// enums
enum {
	MEDIA_SET_VOD,
	MEDIA_SET_LIVE,
};

// typedefs
struct segmenter_conf_s;
struct audio_filter_s;
struct media_sequence_s;
typedef struct media_sequence_s media_sequence_t;

typedef struct {
	uint32_t num;
	uint32_t denom;
} vod_fraction_t;

typedef struct {
	media_track_t* first_track;
	media_track_t* last_track;
	media_track_t* longest_track[MEDIA_TYPE_COUNT];
} media_clip_filtered_t;

struct media_sequence_s {
	// initialized during parsing
	uint32_t index;
	vod_array_part_t* unparsed_clips;
	media_clip_t** clips;						// [clip_count]
	vod_str_t stripped_uri;
	vod_str_t id;
	vod_str_t label;
	language_id_t language;
	int64_t first_key_frame_offset;
	vod_array_part_t* key_frame_durations;
	uint64_t last_key_frame_time;

	// initialized after mapping
	vod_str_t mapped_uri;

	// initialized when the main state machine starts
	u_char encryption_key[MEDIA_CLIP_KEY_SIZE];
	void* drm_info;

	// initialized while applying filters
	uint32_t track_count[MEDIA_TYPE_COUNT];		// track count in each filtered_clips
	uint32_t total_track_count;
	int media_type;
	media_clip_filtered_t* filtered_clips;		// [clip_count]		// XXXXX reduce usage, use filtered_tracks instead
	media_clip_filtered_t* filtered_clips_end;

	uint64_t total_frame_size;
	uint32_t total_frame_count;
	uint32_t video_key_frame_count;
};

typedef struct {
	uint32_t* durations;				// [total_count] clip durations in millis
	uint32_t total_count;				// number of clips in the whole set
	uint64_t* times;					// [total_count] clip timestamps in miilis
	uint64_t* original_times;			// [total_count] clip timestamps in miilis
	uint64_t segment_base_time;			// the time of segment 0
	uint64_t total_duration;			// = sum(durations)
	uint64_t first_time;				// = times[0]
	uint64_t original_first_time;		// start time of the first clip before it was trimmed to the live window
	uint64_t first_clip_start_offset;	// difference between first clip time and the original first time of this clip
	uint32_t first_segment_alignment_offset;	// difference between unaligned first segment time and first_time
} media_clip_timing_t;

typedef struct media_notification_s {
	struct media_notification_s* next;
	vod_str_t id;
} media_notification_t;

typedef struct {
	uint64_t start_time;
	uint32_t duration;
} media_look_ahead_segment_t;

typedef struct {
	// initialized during parsing
	struct segmenter_conf_s* segmenter_conf;
	uint32_t version;

	vod_str_t id;
	uint32_t type;
	media_clip_timing_t timing;
	bool_t use_discontinuity;
	bool_t presentation_end;

	uint32_t clip_count;					// number of clips relevant to serve the current request
	uint32_t sequence_count;
	media_sequence_t* sequences;			// [sequence_count]
	media_sequence_t* sequences_end;
	bool_t has_multi_sequences;

	media_clip_source_t* sources_head;
	media_clip_source_t* mapped_sources_head;
	struct media_clip_dynamic_s* dynamic_clips_head;

	uint64_t segment_start_time;
	uint32_t segment_duration;
	int64_t live_window_duration;
	media_look_ahead_segment_t* look_ahead_segments;
	uint32_t look_ahead_segment_count;

	uint32_t initial_segment_index;
	uint32_t initial_segment_clip_relative_index;
	uint32_t initial_clip_index;
	vod_str_t uri;

	media_notification_t* notifications_head;

	// initialized while applying filters
	uint32_t track_count[MEDIA_TYPE_COUNT];	// sum of track count in all sequences per clip
	uint32_t total_track_count;
	media_track_t* filtered_tracks;			// [total_track_count * clip_count] (all tracks of clip 0, then all tracks of clip1 etc.)
	media_track_t* filtered_tracks_end;
	bool_t audio_filtering_needed;
} media_set_t;

typedef struct {
	int64_t segment_time;		// used in mss
	uint32_t segment_index;
	uint32_t clip_index;
	uint32_t pts_delay;
	uint32_t sequences_mask;
	vod_str_t sequence_id;
	uint32_t tracks_mask[MEDIA_TYPE_COUNT];
	uint32_t* sequence_tracks_mask;	// [MAX_SEQUENCES][MEDIA_TYPE_COUNT]
	uint8_t* langs_mask;			// [LANG_MASK_SIZE]
	uint32_t version;
} request_params_t;

#endif //__MEDIA_SET_H__
