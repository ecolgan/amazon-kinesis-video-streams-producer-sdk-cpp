// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
// Portions Copyright
/*
* GStreamer
* Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
* Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
* Copyright (C) 2017 <<user@hostname.org>>
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#ifndef __GST_KVS_SINK_H__
#define __GST_KVS_SINK_H__

#include <gst/gst.h>
#include <KinesisVideoProducer.h>
#include <string.h>
#include <mutex>
#include <atomic>
#include <gst/base/gstcollectpads.h>
#include <map>

using namespace com::amazonaws::kinesis::video;

G_BEGIN_DECLS

#define KVS_GST_VERSION "1.0"

#define GST_TYPE_KVS_SINK \
  (gst_kvs_sink_get_type())
#define GST_KVS_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_KVS_SINK,GstKvsSink))
#define GST_KVS_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_KVS_SINK,GstKvsSinkClass))
#define GST_IS_KVS_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_KVS_SINK))
#define GST_IS_KVS_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_KVS_SINK))
#define GST_KVS_SINK_CAST(obj) ((GstKvsSink *)obj)


typedef struct _GstKvsSink GstKvsSink;
typedef struct _GstKvsSinkClass GstKvsSinkClass;

typedef struct _CustomData CustomData;

/* all information needed for one track */
typedef struct _GstKvsSinkTrackData {
    GstCollectData collect;       /* we extend the CollectData */
    MKV_TRACK_INFO_TYPE track_type;
    GstKvsSink *kvssink;
    guint track_id;
} GstKvsSinkTrackData;

typedef enum _MediaType {
    AUDIO_VIDEO,
    VIDEO_ONLY,
    AUDIO_ONLY
} MediaType;

/**
 * GstKvsSink:
 *
 * The opaque #GstKvsSink data structure.
 */
struct _GstKvsSink {
    GstElement		element;

    GstCollectPads              *collect;

    // Stream definition
    gchar                       *stream_name;
    guint                       retention_period_hours;
    gchar                       *kms_key_id;
    STREAMING_TYPE              streaming_type;
    gchar                       *content_type;
    guint                       max_latency_seconds;
    guint                       fragment_duration_miliseconds;
    guint                       timecode_scale_milliseconds;
    gboolean                    key_frame_fragmentation;
    gboolean                    frame_timecodes;
    gboolean                    absolute_fragment_times;
    gboolean                    fragment_acks;
    gboolean                    restart_on_error;
    gboolean                    recalculate_metrics;
    guint                       framerate;
    guint                       avg_bandwidth_bps;
    guint                       buffer_duration_seconds;
    guint                       replay_duration_seconds;
    guint                       connection_staleness_seconds;
    gchar                       *codec_id;
    gchar                       *track_name;
    gchar                       *access_key;
    gchar                       *secret_key;
    gchar                       *aws_region;
    uint8_t                     *frame_data;
    size_t                      current_frame_data_size;
    guint                       rotation_period;
    gchar                       *log_config_path;
    guint                       storage_size;
    gchar                       *credential_file_path;
    GstStructure                *iot_certificate;
    GstStructure                *stream_tags;
    gulong                      file_start_time;
    MKV_TRACK_INFO_TYPE         track_info_type;


    guint                       num_streams;
    guint                       num_audio_streams;
    guint                       num_video_streams;

    unique_ptr<Credentials> credentials_;
    shared_ptr<CustomData> data;
};

struct _GstKvsSinkClass {
    GstElementClass parent_class;
};

GType gst_kvs_sink_get_type (void);

G_END_DECLS

class StreamLatencyStateMachine;
class ConnectionStaleStateMachine;

typedef struct _CallbackStateMachine {
    shared_ptr<StreamLatencyStateMachine> stream_latency_state_machine;
    shared_ptr<ConnectionStaleStateMachine> connection_stale_state_machine;

    _CallbackStateMachine(shared_ptr<CustomData> data);
} CallbackStateMachine;

typedef struct _CustomData {

    _CustomData():
            stream_created(false),
            stream_ready(false),
            stream_status(STATUS_SUCCESS),
            stream_error_code(STATUS_SUCCESS),
            last_dts(0),
            pts_base(0),
            media_type(VIDEO_ONLY),
            first_video_frame(true) {}
    unique_ptr<KinesisVideoProducer> kinesis_video_producer;
    shared_ptr<KinesisVideoStream> kinesis_video_stream;
    shared_ptr<CallbackStateMachine> callback_state_machine;
    map<uint64_t, string> track_cpd;
    GstKvsSink *kvsSink;
    bool stream_created = false;
    MediaType media_type;
    bool first_video_frame;

    atomic_bool stream_ready;
    atomic_uint stream_status;
    // variable to store stream errors that will be sent to bus
    atomic_uint stream_error_code;

    uint64_t last_dts;
    uint64_t pts_base;
} CustomData;

#define ACCESS_KEY_ENV_VAR "AWS_ACCESS_KEY_ID"
#define SECRET_KEY_ENV_VAR "AWS_SECRET_ACCESS_KEY"
#define SESSION_TOKEN_ENV_VAR "AWS_SESSION_TOKEN"
#define TOKEN_EXPIRATION_ENV_VAR "AWS_TOKEN_EXPIRATION"
#define DEFAULT_REGION_ENV_VAR "AWS_DEFAULT_REGION"

#endif /* __GST_KVS_SINK_H__ */
