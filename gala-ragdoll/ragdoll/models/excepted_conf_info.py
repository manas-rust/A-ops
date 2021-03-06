# coding: utf-8

from __future__ import absolute_import
from datetime import date, datetime  # noqa: F401

from typing import List, Dict  # noqa: F401

from ragdoll.models.base_model_ import Model
from ragdoll import util
from ragdoll.models.conf_base_info import ConfBaseInfo


class ExceptedConfInfo(Model):
    """NOTE: This class is auto generated by the swagger code generator program.

    Do not edit the class manually.
    """

    def __init__(self, domain_name: str=None, conf_base_infos: List[ConfBaseInfo]=None):  # noqa: E501
        """ExceptedConfInfo - a model defined in Swagger

        :param domain_name: The domain_name of this ExceptedConfInfo.  # noqa: E501
        :type domain_name: str
        :param conf_base_infos: The conf_base_infos of this ExceptedConfInfo.  # noqa: E501
        :type conf_base_infos: List[ConfBaseInfo]
        """
        self.swagger_types = {
            'domain_name': str,
            'conf_base_infos': List[ConfBaseInfo]
        }

        self.attribute_map = {
            'domain_name': 'domainName',
            'conf_base_infos': 'confBaseInfos'
        }

        self._domain_name = domain_name
        self._conf_base_infos = conf_base_infos

    @classmethod
    def from_dict(cls, dikt) -> 'ExceptedConfInfo':
        """Returns the dict as a model

        :param dikt: A dict.
        :type: dict
        :return: The ExceptedConfInfo of this ExceptedConfInfo.  # noqa: E501
        :rtype: ExceptedConfInfo
        """
        return util.deserialize_model(dikt, cls)

    @property
    def domain_name(self) -> str:
        """Gets the domain_name of this ExceptedConfInfo.


        :return: The domain_name of this ExceptedConfInfo.
        :rtype: str
        """
        return self._domain_name

    @domain_name.setter
    def domain_name(self, domain_name: str):
        """Sets the domain_name of this ExceptedConfInfo.


        :param domain_name: The domain_name of this ExceptedConfInfo.
        :type domain_name: str
        """

        self._domain_name = domain_name

    @property
    def conf_base_infos(self) -> List[ConfBaseInfo]:
        """Gets the conf_base_infos of this ExceptedConfInfo.


        :return: The conf_base_infos of this ExceptedConfInfo.
        :rtype: List[ConfBaseInfo]
        """
        return self._conf_base_infos

    @conf_base_infos.setter
    def conf_base_infos(self, conf_base_infos: List[ConfBaseInfo]):
        """Sets the conf_base_infos of this ExceptedConfInfo.


        :param conf_base_infos: The conf_base_infos of this ExceptedConfInfo.
        :type conf_base_infos: List[ConfBaseInfo]
        """

        self._conf_base_infos = conf_base_infos
