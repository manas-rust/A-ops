#!/usr/bin/python3
# ******************************************************************************
# Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
# licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#     http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN 'AS IS' BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
# PURPOSE.
# See the Mulan PSL v2 for more details.
# ******************************************************************************/
"""
Time:
Author:
Description: some helper function
"""
from aops_utils.conf import configuration
from aops_utils.conf.constant import URL_FORMAT


def make_datacenter_url(route):
    """
    make database center url

    Args:
        route(str)

    Returns:
        str: url
    """
    # make database center url
    database_ip = configuration.database.get("IP")  # pylint: disable=E1101
    database_port = configuration.database.get("PORT")  # pylint: disable=E1101
    database_url = URL_FORMAT % (database_ip, database_port, route)
    return database_url