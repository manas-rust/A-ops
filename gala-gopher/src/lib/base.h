#ifndef __BASE_H__
#define __BASE_H__

#define GALA_GOPHER_INFO(description)   1

// ======== COMMON ========
// probe
#define MAX_PROBES_NUM        32
// extend probe
#define MAX_EXTEND_PROBES_NUM   32

// fifo
#define MAX_FIFO_NUM          MAX_PROBES_NUM
#define MAX_FIFO_SIZE         1024

// meta
#define MAX_META_PATH_LEN           2048
#define MAX_FIELD_DESCRIPTION_LEN   256
#define MAX_FIELD_TYPE_LEN          64
#define MAX_FIELD_NAME_LEN          64
#define MAX_MEASUREMENTS_NUM        64
#define MAX_MEASUREMENT_NAME_LEN    64
#define MAX_FIELDS_NUM              64

// ingress
#define MAX_EPOLL_SIZE        1024
#define MAX_EPOLL_EVENTS_NUM  512

// egress
#define MAX_DATA_STR_LEN      1024

// imdb
#define MAX_IMDB_TABLE_CAPACITY         64
#define MAX_IMDB_RECORD_CAPACITY        64

// kafka
#define MAX_KAFKA_ERRSTR_SIZE 512

// common
#define MAX_THREAD_NAME_LEN   128

// ======== CONFIG ========
// global config
#define MAX_LOG_DIRECTORY_LEN 128

// kafka config
#define MAX_KAFKA_BROKER_LEN  32
#define MAX_KAFKA_TOPIC_LEN   32

// probe config
#define MAX_PROBE_NAME_LEN    32

// extend probe config
#define MAX_EXTEND_PROBE_COMMAND_LEN 128
#define MAX_EXTEND_PROBE_PARAM_LEN 128
#define MAX_COMMAND_LEN 1024

// kafka switch
typedef enum {
    KAFKA_SWITCH_ON = 0,
    KAFKA_SWITCH_OFF,
    KAFKA_SWITCH_MAX
} KafkaSwitch;

// probe status
typedef enum {
    PROBE_SWITCH_AUTO = 0,
    PROBE_SWITCH_ON,
    PROBE_SWITCH_OFF,
    PROBE_SWITCH_MAX
} ProbeSwitch;

typedef enum {
    PROBE_CHK_CNT = 0,
    PROBE_CHK_MAX
} ProbeStartCheckType;

#define GALA_META_DIR_PATH "/opt/gala-gopher/meta"
#define GALA_CONF_PATH_DEFAULT "/opt/gala-gopher/gala-gopher.conf"

char *g_galaConfPath;

#endif

