#!/usr/bin/python3
# ******************************************************************************
# Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
# licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#     http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
# PURPOSE.
# See the Mulan PSL v2 for more details.
# ******************************************************************************/
"""
Description: statistics method's entrance for custom commands
Class:StatCommand
"""
import sys
from aops_cli.base_cmd import BaseCommand
from aops_utils.conf.constant import GET_HOST_COUNT
from aops_utils.restful.helper import make_manager_url
from aops_utils.cli_utils import cli_request, add_access_token


class StatCommand(BaseCommand):
    """
    Description: start the statistics part
    Attributes:
        sub_parse: Subcommand parameters
        params: Command line parameters
    """

    def __init__(self):
        """
        Description: Instance initialization
        """
        super().__init__()
        self.add_subcommand(
            sub_command='stat',
            help_desc="stat's operations")

        self.sub_parse.add_argument(
            '--action',
            help='stat actions: count',
            nargs='?',
            type=str,
            required=True,
            choices=['count'])

        add_access_token(self.sub_parse)

        self.sub_parse.add_argument(
            '--field',
            help="field about data",
            nargs='?',
            type=str,
            choices=['host'],
            required=True)

    def do_command(self, params):
        """
        Description: Executing command
        Args:
            params: Command line parameters
        """
        action = params.action
        action_dict = {
            'count': self.manage_requests_count,
        }
        return action_dict.get(action)(params)

    @staticmethod
    def manage_requests_count(params):
        """
        Description: Executing getdata command with cofiguration file
        Args:
            params: Command line parameters
        Returns:
            dict: body of response
        """
        if params.field is None:
            print("please input the field of the statistics, using --field <field>.")
            sys.exit(0)
        pyload = {
                 }
        manager_url, header = make_manager_url(GET_HOST_COUNT)
        return cli_request('POST', manager_url, pyload, header, params.access_token)
