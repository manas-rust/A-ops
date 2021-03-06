#ifndef __IMDB_H__
#define __IMDB_H__

#include <stdint.h>
#include <pthread.h>
#include <uthash.h>
#include "base.h"

#define MAX_IMDB_DATABASEMGR_CAPACITY   256
// metric specification
#define MAX_IMDB_METRIC_DESC_LEN        1024
#define MAX_IMDB_METRIC_TYPE_LEN        32
#define MAX_IMDB_METRIC_NAME_LEN        32
#define MAX_IMDB_METRIC_VAL_LEN         512

// table specification
#define MAX_IMDB_TABLE_NAME_LEN         32

// database specification
#define MAX_IMDB_DATABASE_NAME_LEN      32

// MAX LENGTH FOR PROMETHEUS LABELS
#define MAX_LABELS_BUFFER_SIZE 512

#define MAX_IMDB_MACHINEID_LEN          40
#define MAX_IMDB_HOSTNAME_LEN           64

#define METRIC_TYPE_LABEL "label"
#define METRIC_TYPE_KEY "key"

typedef struct {
    char machineId[MAX_IMDB_MACHINEID_LEN];
    char hostName[MAX_IMDB_HOSTNAME_LEN];
} IMDB_NodeInfo;

// typedef enum {
//     METRIC_TYPE_COUNTER = 0,
//     METRIC_TYPE_GAUGE,
//     METRIC_TYPE_HISTOGRAM,
//     METRIC_TYPE_SUMMARY,
//     METRIC_TYPE_MAX,
// } MetricType;

typedef struct {
    char description[MAX_IMDB_METRIC_DESC_LEN];
    // MetricType type;
    char type[MAX_IMDB_METRIC_TYPE_LEN];
    char name[MAX_IMDB_METRIC_NAME_LEN];
    char val[MAX_IMDB_METRIC_VAL_LEN];
} IMDB_Metric;

typedef struct {
    uint32_t keySize;
    char *key;
    time_t updateTime;     // Unit: second
    uint32_t capacity;
    uint32_t metricsNum;
    IMDB_Metric **metrics;
    UT_hash_handle hh;
} IMDB_Record;

typedef struct {
    char name[MAX_IMDB_TABLE_NAME_LEN];
    IMDB_Record *meta;

    uint32_t recordsCapacity;      // capacity for records in one table
    uint32_t recordKeySize;
    IMDB_Record **records;
} IMDB_Table;

typedef struct {
    uint32_t capacity;      // capacity for tables in one database
    uint32_t tablesNum;

    IMDB_Table **tables;
    IMDB_NodeInfo nodeInfo;
    pthread_rwlock_t rwlock;
} IMDB_DataBaseMgr;

IMDB_Metric *IMDB_MetricCreate(char *name, char *description, char *type);
int IMDB_MetricSetValue(IMDB_Metric *metric, char *val);
void IMDB_MetricDestroy(IMDB_Metric *metric);

IMDB_Record *IMDB_RecordCreate(uint32_t capacity);
IMDB_Record *IMDB_RecordCreateWithKey(uint32_t capacity, uint32_t keySize);
int IMDB_RecordAddMetric(IMDB_Record *record, IMDB_Metric *metric);
int IMDB_RecordAppendKey(IMDB_Record *record, uint32_t keyIdx, char *val);
void IMDB_RecordUpdateTime(IMDB_Record *record, time_t seconds);
void IMDB_RecordDestroy(IMDB_Record *record);

IMDB_Record *HASH_findRecord(IMDB_Record **records, IMDB_Record *record);
void HASH_deleteRecord(IMDB_Record **records, IMDB_Record *record);
void HASH_deleteAndFreeRecords(IMDB_Record **records);
void HASH_addRecord(IMDB_Record **records, IMDB_Record *record);
uint32_t HASH_recordCount(IMDB_Record **records);

IMDB_Table *IMDB_TableCreate(char *name, uint32_t capacity);
int IMDB_TableSetMeta(IMDB_Table *table, IMDB_Record *metaRecord);
int IMDB_TableSetRecordKeySize(IMDB_Table *table, uint32_t keyNum);
int IMDB_TableAddRecord(IMDB_Table *table, IMDB_Record *record);
void IMDB_TableDestroy(IMDB_Table *table);

IMDB_DataBaseMgr *IMDB_DataBaseMgrCreate(uint32_t capacity);
void IMDB_DataBaseMgrSetRecordTimeout(uint32_t timeout);
void IMDB_DataBaseMgrDestroy(IMDB_DataBaseMgr *mgr);

int IMDB_DataBaseMgrAddTable(IMDB_DataBaseMgr *mgr, IMDB_Table* table);
IMDB_Table *IMDB_DataBaseMgrFindTable(IMDB_DataBaseMgr *mgr, char *tableName);

int IMDB_DataBaseMgrAddRecord(IMDB_DataBaseMgr *mgr, char *recordStr, int len);
int IMDB_DataBaseMgrData2String(IMDB_DataBaseMgr *mgr, char *buffer, int maxLen);

int IMDB_DataStr2Json(IMDB_DataBaseMgr *mgr, char *recordStr, int recordLen, char *jsonStr, int jsonStrLen);

#endif

