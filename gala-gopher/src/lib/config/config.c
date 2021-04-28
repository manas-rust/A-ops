#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libconfig.h>

#include "config.h"

ConfigMgr *ConfigMgrCreate()
{
    ConfigMgr *mgr = NULL;
    mgr = (ConfigMgr *)malloc(sizeof(ConfigMgr));
    if (mgr == NULL) {
        return NULL;
    }
    memset(mgr, 0, sizeof(ConfigMgr));

    mgr->globalConfig = (GlobalConfig *)malloc(sizeof(GlobalConfig));
    if (mgr->globalConfig == NULL) {
        goto ERR;
    }
    memset(mgr->globalConfig, 0, sizeof(GlobalConfig));

    mgr->ingressConfig = (IngressConfig *)malloc(sizeof(IngressConfig));
    if (mgr->ingressConfig == NULL) {
        goto ERR;
    }
    memset(mgr->ingressConfig, 0, sizeof(IngressConfig));

    mgr->egressConfig = (EgressConfig *)malloc(sizeof(EgressConfig));
    if (mgr->egressConfig == NULL) {
        goto ERR;
    }
    memset(mgr->egressConfig, 0, sizeof(EgressConfig));
    
    mgr->taosdataConfig = (TaosdataConfig *)malloc(sizeof(TaosdataConfig));
    if (mgr->taosdataConfig == NULL) {
        goto ERR;
    }
    memset(mgr->taosdataConfig, 0, sizeof(TaosdataConfig));

    mgr->kafkaConfig = (KafkaConfig *)malloc(sizeof(KafkaConfig));
    if (mgr->kafkaConfig == NULL) {
        goto ERR;
    }
    memset(mgr->kafkaConfig, 0, sizeof(KafkaConfig));

    mgr->probesConfig = (ProbesConfig *)malloc(sizeof(ProbesConfig));
    if (mgr->probesConfig == NULL) {
        goto ERR;
    }
    memset(mgr->probesConfig, 0, sizeof(ProbesConfig));

    return mgr;
ERR:
    ConfigMgrDestroy(mgr);
    return NULL;
}

void ConfigMgrDestroy(ConfigMgr *mgr)
{
    if (mgr == NULL) {
        return;
    }

    if (mgr->globalConfig != NULL) {
        free(mgr->globalConfig);
    }

    if (mgr->taosdataConfig != NULL) {
        free(mgr->taosdataConfig);
    }

    if (mgr->kafkaConfig != NULL) {
        free(mgr->kafkaConfig);
    }

    if (mgr->probesConfig != NULL) {
        for (int i = 0; i < mgr->probesConfig->probesNum; i++) {
            if (mgr->probesConfig->probesConfig[i] != NULL) {
                free(mgr->probesConfig->probesConfig[i]);
            }
        }
        free(mgr->probesConfig);
    }

    free(mgr);
    return;
}

static int ConfigMgrLoadGlobalConfig(void *config, config_setting_t *settings)
{
    GlobalConfig *globalConfig = (GlobalConfig *)config;
    uint32_t ret = 0;

    const char *strVal;
    ret = config_setting_lookup_string(settings, "log_directory", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for log_directory failed.\n");
        return -1;
    }

    memcpy(globalConfig->logDirectory, strVal, strlen(strVal));
    return 0;
}

static int ConfigMgrLoadIngressConfig(void *config, config_setting_t *settings)
{
    IngressConfig *ingressConfig = (IngressConfig *)config;
    uint32_t ret = 0;

    int intVal = 0;
    ret = config_setting_lookup_int(settings, "interval", &intVal);
    if (ret == 0) {
        printf("[CONFIG] load config for ingress interval failed.\n");
        return -1;
    }
    ingressConfig->interval = intVal;
        
    return 0;
}

static int ConfigMgrLoadEgressConfig(void *config, config_setting_t *settings)
{
    EgressConfig *egressConfig = (EgressConfig *)config;
    uint32_t ret = 0;

    int intVal = 0;
    ret = config_setting_lookup_int(settings, "interval", &intVal);
    if (ret == 0) {
        printf("[CONFIG] load config for egress interval failed.\n");
        return -1;
    }
    egressConfig->interval = intVal;

    ret = config_setting_lookup_int(settings, "time_range", &intVal);
    if (ret == 0) {
        printf("[CONFIG] load config for egress time_range failed.\n");
        return -1;
    }
    egressConfig->timeRange = intVal;

    return 0;
}

static int ConfigMgrLoadTaosdataConfig(void *config, config_setting_t *settings)
{
    TaosdataConfig *taosConfig = (TaosdataConfig *)config;
    uint32_t ret = 0;
    const char *strVal = NULL;
    int intVal = 0;
    
    ret = config_setting_lookup_string(settings, "taosdata_ip", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for taosdata_ip failed.\n");
        return -1;
    }
    memcpy(taosConfig->ip, strVal, strlen(strVal));

    ret = config_setting_lookup_string(settings, "taosdata_user", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for taosdata_user failed.\n");
        return -1;
    }
    memcpy(taosConfig->user, strVal, strlen(strVal));

    ret = config_setting_lookup_string(settings, "taosdata_pass", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for taosdata_pass failed.\n");
        return -1;
    }
    memcpy(taosConfig->pass, strVal, strlen(strVal));

    ret = config_setting_lookup_string(settings, "taosdata_db", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for taosdata_db failed.\n");
        return -1;
    }
    memcpy(taosConfig->dbName, strVal, strlen(strVal));

    ret = config_setting_lookup_int(settings, "taosdata_port", &intVal);
    if (ret == 0) {
        printf("[CONFIG] load config for taosdata_port failed.\n");
        return -1;
    }
    taosConfig->port = intVal;
    
    return 0;

}

static int ConfigMgrLoadKafkaConfig(void *config, config_setting_t *settings)
{
    KafkaConfig *kafkaConfig = (KafkaConfig *)config;
    uint32_t ret = 0;
    const char *strVal = NULL;
    
    ret = config_setting_lookup_string(settings, "kafka_broker", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for kafka_broker failed.\n");
        return -1;
    }
    memcpy(kafkaConfig->broker, strVal, strlen(strVal));

    ret = config_setting_lookup_string(settings, "kafka_topic", &strVal);
    if (ret == 0) {
        printf("[CONFIG] load config for kafka_topic failed.\n");
        return -1;
    }
    memcpy(kafkaConfig->topic, strVal, strlen(strVal));

    return 0;

}

static int ConfigMgrLoadProbesConfig(void *config, config_setting_t *settings)
{
    ProbesConfig *probesConfig = (ProbesConfig *)config;
    uint32_t ret = 0;
    int count = 0;
    const char *strVal = NULL;
    int intVal = 0;

    count = config_setting_length(settings);
    for (int i = 0; i < count; i++) {
        if (probesConfig->probesNum == MAX_PROBES_NUM) {
            printf("[CONFIG] probesConfig list full.\n");
            return -1;
        }
        config_setting_t *_probe = config_setting_get_elem(settings, i);

        ProbeConfig *_probeConfig = (ProbeConfig *)malloc(sizeof(ProbeConfig));
        if (_probeConfig == NULL) {
            printf("[CONFIG] failed to malloc memory for ProbeConfig \n");
            return -1;
        }
        memset(_probeConfig, 0, sizeof(ProbeConfig));
        probesConfig->probesConfig[probesConfig->probesNum] = _probeConfig;
        probesConfig->probesNum++;

        ret = config_setting_lookup_string(_probe, "name", &strVal);
        if (ret == 0) {
            printf("[CONFIG] load config for probe name failed.\n");
            return -1;
        }
        memcpy(_probeConfig->name, strVal, strlen(strVal));

        ret = config_setting_lookup_string(_probe, "switch", &strVal);
        if (ret == 0) {
            printf("[CONFIG] load config for probe switch failed.\n");
            return -1;
        }
        if (strcmp(strVal, "auto") == 0) {
            _probeConfig->probeSwitch = PROBE_SWITCH_AUTO;
        } else if (strcmp(strVal, "on") == 0) {
            _probeConfig->probeSwitch = PROBE_SWITCH_ON;
        } else {
            _probeConfig->probeSwitch = PROBE_SWITCH_OFF;
        }
        
        ret = config_setting_lookup_int(_probe, "interval", &intVal);
        if (ret == 0) {
            printf("[CONFIG] load config for probe interval failed.\n");
            return -1;
        }
        _probeConfig->interval = intVal;

    }

    return 0;
}

typedef int (*ConfigLoadFunc)(void *config, config_setting_t *settings);

typedef struct {
    void *config;
    char *sectionName;
    ConfigLoadFunc func;
} ConfigLoadHandle;

int ConfigMgrLoad(ConfigMgr *mgr, const char *confPath)
{
    ConfigLoadHandle configLoadHandles[] = {
        { (void *)mgr->globalConfig, "global", ConfigMgrLoadGlobalConfig},
        { (void *)mgr->ingressConfig, "ingress", ConfigMgrLoadIngressConfig},
        { (void *)mgr->egressConfig, "egress", ConfigMgrLoadEgressConfig},
        { (void *)mgr->taosdataConfig, "taosdata", ConfigMgrLoadTaosdataConfig},
        { (void *)mgr->kafkaConfig, "kafka", ConfigMgrLoadKafkaConfig},
        { (void *)mgr->probesConfig, "probes", ConfigMgrLoadProbesConfig}
    };

    int ret = 0;
    config_t cfg;
    config_setting_t *settings = NULL;

    config_init(&cfg);
    ret = config_read_file(&cfg, confPath);
    if (ret == 0) {
        printf("[CONFIG] config read %s failed.\n", confPath);
        goto ERR;
    }

    uint32_t configUnitNum = sizeof(configLoadHandles) / sizeof(configLoadHandles[0]);
    for (int i = 0; i < configUnitNum; i++) {
        settings = config_lookup(&cfg, configLoadHandles[i].sectionName);
        if (settings == NULL) {
            printf("[CONFIG] config lookup %s failed.\n", configLoadHandles[i].sectionName);
            goto ERR;
        }

        ret = configLoadHandles[i].func(configLoadHandles[i].config, settings);
        if (ret != 0) {
            printf("[CONFIG] config load handle %s failed.\n", configLoadHandles[i].sectionName);
            goto ERR;
        }
    }

    config_destroy(&cfg);
    return 0;
ERR:
    config_destroy(&cfg);
    return -1;
}
