from typing import List, Any
from dataclasses import dataclass
from dataclasses import field


@dataclass
class Label:
    name: str
    value: Any


@dataclass
class DataRecord:
    metric_id: str
    timestamp: float
    metric_value: Any
    labels: List[Label] = field(default_factory=list)


class DataCollector:
    def __init__(self):
        pass

    def get_instant_data(self, metric_id: str, timestamp: float = None, **kwargs) -> List[DataRecord]:
        pass

    def get_range_data(self, metric_id: str, start: float, end: float, **kwargs) -> List[DataRecord]:
        pass
